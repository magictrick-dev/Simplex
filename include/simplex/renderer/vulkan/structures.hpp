#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

#include <simplex/array_view.hpp>
#include <simplex/string_view.hpp>

#include <type_traits>
#include <cstddef>

namespace spx::vk
{

    /// @brief A mixin extension strategy for specializations of certain vulkan structures.
    ///
    /// Each specialization redeclares the members of its native Vulkan struct, in the exact
    /// same order, and layers typed getters/setters on top. vk_struct_base inherits from the
    /// matching specialization, which keeps the wrapper layout-compatible with the native
    /// struct so it can be reinterpret-cast straight into the Vulkan API. The compile-time
    /// guards at the bottom of this file enforce that invariant per specialization.
    template <typename derived_t, typename native_t> struct vk_struct_ext { };

    /// @brief Provides a base structure for vulkan structures with .sType
    /// @tparam structure_type_t The type of Vulkan structure.
    ///
    /// In most cases, you won't directly create a structure with this template, as
    /// they're aliased below with using statements. Overloads for the native type
    /// conversion to the actual vulkan calls are provided for you.
    template <typename structure_type_t>
    struct vk_struct_base : vk_struct_ext<vk_struct_base<structure_type_t>, structure_type_t>
    {

        using native_type_t = structure_type_t;

        inline vk_struct_base() = default;
        inline vk_struct_base(const native_type_t& other) { this->set(other); }

        inline vk_struct_base& set(const native_type_t &base)
        {
            const vk_struct_base& r_cast = reinterpret_cast<const vk_struct_base&>(base);
            *this = r_cast;
            return *this;
        }

        inline operator native_type_t&()             { return *reinterpret_cast<structure_type_t*>(this);        }
        inline operator native_type_t const&() const { return *reinterpret_cast<const structure_type_t*>(this);  }

    };

    // ---------------------------------------------------------------------------------------------
    // Vulkan struct mixins.
    //
    // Reimplements the base vulkan structures with their standard types. Adds helpers and
    // utilities where needed. The structs will be compile-time checked for consistency
    // with the native Vulkan SDK layouts.
    // ---------------------------------------------------------------------------------------------

    /// @brief VkApplicationInfo mixin.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkApplicationInfo>
    {

        VkStructureType     sType                   { VK_STRUCTURE_TYPE_APPLICATION_INFO    };
        const void*         pNext                   { nullptr                               };
        const char*         pApplicationName        { nullptr                               };
        uint32_t            applicationVersion      {                                       };
        const char*         pEngineName             { nullptr                               };
        uint32_t            engineVersion           {                                       };
        uint32_t            apiVersion              {                                       };

        inline const void*  get_next() const                { return this->pNext;               }
        inline const char*  get_application_name() const    { return this->pApplicationName;    }
        inline uint32_t     get_application_version() const { return this->applicationVersion;  }
        inline const char*  get_engine_name() const         { return this->pEngineName;         }
        inline uint32_t     get_engine_version() const      { return this->engineVersion;       }
        inline uint32_t     get_api_version() const         { return this->apiVersion;          }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();                  }
        inline derived_t& set_application_name(const char* name)    { this->pApplicationName = name; return *s();       }
        inline derived_t& set_application_version(uint32_t version) { this->applicationVersion = version; return *s();  }
        inline derived_t& set_engine_name(const char* name)         { this->pEngineName = name; return *s();            }
        inline derived_t& set_engine_version(uint32_t version)      { this->engineVersion = version; return *s();       }
        inline derived_t& set_api_version(uint32_t version)         { this->apiVersion = version; return *s();          }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkInstanceCreate mixin.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkInstanceCreateInfo>
    {

        VkStructureType             sType                   { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO    };
        const void*                 pNext                   { nullptr                                   };
        VkInstanceCreateFlags       flags                   {                                           };
        const VkApplicationInfo*    pApplicationInfo        { nullptr                                   };
        uint32_t                    enabledLayerCount       {                                           };
        const char* const*          ppEnabledLayerNames     { nullptr                                   };
        uint32_t                    enabledExtensionCount   {                                           };
        const char* const*          ppEnabledExtensionNames { nullptr                                   };

        inline const void*                  get_next() const                { return this->pNext;                                                       }
        inline VkInstanceCreateFlags        get_flags() const               { return this->flags;                                                       }
        inline const VkApplicationInfo*     get_application_info() const    { return this->pApplicationInfo;                                            }
        inline spx::array_view<const char*> get_layers() const              { return { this->ppEnabledLayerNames, this->enabledLayerCount };            }
        inline spx::array_view<const char*> get_extensions() const          { return { this->ppEnabledExtensionNames, this->enabledExtensionCount };    }

        inline derived_t& set_next(const void* next)                            { this->pNext = next; return *s();              }
        inline derived_t& set_flags(VkInstanceCreateFlags flags)                { this->flags = flags; return *s();             }
        inline derived_t& set_application_info(const VkApplicationInfo* info)   { this->pApplicationInfo = info; return *s();   }

        inline derived_t& set_layers(spx::array_view<const char*> layers)
        {
            this->ppEnabledLayerNames = layers.data();
            this->enabledLayerCount   = static_cast<uint32_t>(layers.size());
            return *s();
        }

        inline derived_t& set_extensions(spx::array_view<const char*> extensions)
        {
            this->ppEnabledExtensionNames = extensions.data();
            this->enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    // ---------------------------------------------------------------------------------------------
    // Using statements.
    //
    // Cleans up the template syntax and normalizes it to a friendlier to type
    // variation.
    // ---------------------------------------------------------------------------------------------

    using application_info      = vk_struct_base<VkApplicationInfo>;
    using instance_create_info  = vk_struct_base<VkInstanceCreateInfo>;

    // ---------------------------------------------------------------------------------------------
    // Layout guards.
    //
    // The reinterpret_cast interop in vk_struct_base is only valid if each wrapper is
    // standard-layout and bit-identical to its native struct. These checks turn any drift
    // (a member reordered, retyped, or an SDK bump that changes the struct) into a build
    // error instead of silent memory corruption. Add a matching block for every new
    // specialization.
    // ---------------------------------------------------------------------------------------------

    // VkApplicationInfo checks.
    static_assert(std::is_standard_layout_v<application_info>, "application_info must be standard-layout for native interop.");
    static_assert(sizeof(application_info) == sizeof(VkApplicationInfo), "application_info layout diverged from VkApplicationInfo.");
    static_assert(offsetof(application_info, sType) == offsetof(VkApplicationInfo, sType));
    static_assert(offsetof(application_info, pNext) == offsetof(VkApplicationInfo, pNext));
    static_assert(offsetof(application_info, pApplicationName) == offsetof(VkApplicationInfo, pApplicationName));
    static_assert(offsetof(application_info, applicationVersion) == offsetof(VkApplicationInfo, applicationVersion));
    static_assert(offsetof(application_info, pEngineName) == offsetof(VkApplicationInfo, pEngineName));
    static_assert(offsetof(application_info, engineVersion) == offsetof(VkApplicationInfo, engineVersion));
    static_assert(offsetof(application_info, apiVersion) == offsetof(VkApplicationInfo, apiVersion));

    // VkInstanceCreateInfo checks.
    static_assert(std::is_standard_layout_v<instance_create_info>, "instance_create_info must be standard-layout for native interop.");
    static_assert(sizeof(instance_create_info) == sizeof(VkInstanceCreateInfo), "instance_create_info layout diverged from VkInstanceCreateInfo.");
    static_assert(offsetof(instance_create_info, sType) == offsetof(VkInstanceCreateInfo, sType));
    static_assert(offsetof(instance_create_info, pNext) == offsetof(VkInstanceCreateInfo, pNext));
    static_assert(offsetof(instance_create_info, flags) == offsetof(VkInstanceCreateInfo, flags));
    static_assert(offsetof(instance_create_info, pApplicationInfo) == offsetof(VkInstanceCreateInfo, pApplicationInfo));
    static_assert(offsetof(instance_create_info, enabledLayerCount) == offsetof(VkInstanceCreateInfo, enabledLayerCount));
    static_assert(offsetof(instance_create_info, ppEnabledLayerNames) == offsetof(VkInstanceCreateInfo, ppEnabledLayerNames));
    static_assert(offsetof(instance_create_info, enabledExtensionCount) == offsetof(VkInstanceCreateInfo, enabledExtensionCount));
    static_assert(offsetof(instance_create_info, ppEnabledExtensionNames) == offsetof(VkInstanceCreateInfo, ppEnabledExtensionNames));


}
