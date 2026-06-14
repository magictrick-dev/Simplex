#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

#include <simplex/array_view.hpp>
#include <simplex/string_view.hpp>

namespace spx::vk
{

    /// @brief A mixin extension strategy for specializations of certain vulkan structures.
    template <typename derived_t, typename native_t> struct vk_struct_ext { };

    /// @brief Provides a base structure for vulkan structures with .sType
    /// @tparam structure_type_t The type of Vulkan structure.
    /// @tparam structure_type The structure type.
    ///
    /// In most cases, you won't directly create a structure with this template, as
    /// they're aliased below with using statements. Overloads for the native type
    /// conversion to the actual vulkan calls are provided for you.
    template <typename structure_type_t, VkStructureType structure_type>
    struct vk_struct_wt : vk_struct_ext<vk_struct_wt<structure_type_t, structure_type>, structure_type_t>
    {

        using native_type_t     = structure_type_t;
        native_type_t native    = { .sType = structure_type };

        inline vk_struct_wt& set(const native_type_t &values) { native = values; native.sType = structure_type; };

        inline operator native_type_t&()                { return this->native;  }
        inline operator native_type_t*()                { return &this->native; }
        inline operator native_type_t const&() const    { return this->native;  }
        inline operator native_type_t const*() const    { return &this->native; }

    };

    /// @brief Provides a base structure for vulkan structures without an .sType.
    /// @tparam structure_type_t The type of Vulkan structure.
    ///
    /// In most cases, you won't directly create a structure with this template, as
    /// they're aliased below with using statements. Overloads for the native type
    /// conversion to the actual vulkan calls are provided for you.
    template <typename structure_type_t>
    struct vk_struct_wot : vk_struct_ext<vk_struct_wot<structure_type_t>, structure_type_t>
    {

        using native_type_t     = structure_type_t;
        native_type_t native    = { };

        inline vk_struct_wot& set(const native_type_t &values) { native = values; };

        inline operator native_type_t&()                { return this->native;  }
        inline operator native_type_t*()                { return &this->native; }
        inline operator native_type_t const&() const    { return this->native;  }
        inline operator native_type_t const*() const    { return &this->native; }

    };

    using instance_create_info                  = vk_struct_wt<VkInstanceCreateInfo,                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO>;
    using physical_device_properties            = vk_struct_wt<VkPhysicalDeviceProperties2,         VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2>;
    using physical_device_driver_properties     = vk_struct_wt<VkPhysicalDeviceDriverProperties,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES>;
    using physical_device_memory_properties     = vk_struct_wt<VkPhysicalDeviceMemoryProperties2,   VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2>;
    using physical_device_10_features           = vk_struct_wt<VkPhysicalDeviceFeatures2,           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2>;
    using physical_device_11_features           = vk_struct_wt<VkPhysicalDeviceVulkan11Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES>;
    using physical_device_12_features           = vk_struct_wt<VkPhysicalDeviceVulkan12Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES>;
    using physical_device_13_features           = vk_struct_wt<VkPhysicalDeviceVulkan13Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES>;
    using physical_device_14_features           = vk_struct_wt<VkPhysicalDeviceVulkan14Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES>;
    using application_info                      = vk_struct_wot<VkApplicationInfo>;
    using layer_properties                      = vk_struct_wot<VkLayerProperties>;

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkInstanceCreateInfo>
    {

        private:
            inline derived_t& self()
            {
                return *static_cast<derived_t*>(this);
            }

            inline const derived_t& self() const
            {
                return *static_cast<const derived_t*>(this);
            }

        public:
            inline derived_t& set_next(const void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_flags(VkInstanceCreateFlags flags)
            {
                self().native.flags = flags;
                return self();
            }

            inline derived_t& set_application_info(const VkApplicationInfo *info)
            {
                self().native.pApplicationInfo = info;
                return self();
            }

            inline derived_t& set_enabled_layers(uint32_t count, const char* const* names)
            {
                self().native.enabledLayerCount = count;
                self().native.ppEnabledLayerNames = names;
                return self();
            }

            inline derived_t& set_enabled_layers(spx::array_view<const char*> layers)
            {
                return set_enabled_layers((uint32_t)layers.size(), layers.data());
            }

            inline derived_t& set_enabled_extensions(uint32_t count, const char* const* names)
            {
                self().native.enabledExtensionCount = count;
                self().native.ppEnabledExtensionNames = names;
                return self();
            }

            inline derived_t& set_enabled_extensions(spx::array_view<const char*> extensions)
            {
                return set_enabled_extensions((uint32_t)extensions.size(), extensions.data());
            }

            inline const void*                  get_next()                  const { return self().native.pNext; }
            inline VkInstanceCreateFlags        get_flags()                 const { return self().native.flags; }
            inline const VkApplicationInfo*     get_application_info()      const { return self().native.pApplicationInfo; }
            inline spx::array_view<const char*> get_enabled_layers()        const { return { self().native.ppEnabledLayerNames,     self().native.enabledLayerCount     }; }
            inline spx::array_view<const char*> get_enabled_extensions()    const { return { self().native.ppEnabledExtensionNames, self().native.enabledExtensionCount }; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkApplicationInfo>
    {

        private:
            inline derived_t& self()
            {
                return *static_cast<derived_t*>(this);
            }

            inline const derived_t& self() const
            {
                return *static_cast<const derived_t*>(this);
            }

        public:
            inline derived_t& set_next(const void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_application_name(const char* name)
            {
                self().native.pApplicationName = name;
                return self();
            }

            inline derived_t& set_application_version(uint32_t version)
            {
                self().native.applicationVersion = version;
                return self();
            }

            inline derived_t& set_engine_name(const char* name)
            {
                self().native.pEngineName = name;
                return self();
            }

            inline derived_t& set_engine_version(uint32_t version)
            {
                self().native.engineVersion = version;
                return self();
            }

            inline derived_t& set_api_version(uint32_t version)
            {
                self().native.apiVersion = version;
                return self();
            }

            inline const void* get_next()                const { return self().native.pNext; }
            inline const char* get_application_name()    const { return self().native.pApplicationName; }
            inline uint32_t    get_application_version() const { return self().native.applicationVersion; }
            inline const char* get_engine_name()         const { return self().native.pEngineName; }
            inline uint32_t    get_engine_version()      const { return self().native.engineVersion; }
            inline uint32_t    get_api_version()         const { return self().native.apiVersion; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceProperties2>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline void*                             get_next()       const { return self().native.pNext; }
            inline const VkPhysicalDeviceProperties& get_properties() const { return self().native.properties; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceDriverProperties>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline void*                get_next()                const { return self().native.pNext; }
            inline VkDriverId           get_driver_id()           const { return self().native.driverID; }
            inline const char*          get_driver_name()         const { return self().native.driverName; }
            inline const char*          get_driver_info()         const { return self().native.driverInfo; }
            inline VkConformanceVersion get_conformance_version() const { return self().native.conformanceVersion; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceMemoryProperties2>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline void*                                   get_next()              const { return self().native.pNext; }
            inline const VkPhysicalDeviceMemoryProperties& get_memory_properties() const { return self().native.memoryProperties; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceFeatures2>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_features(const VkPhysicalDeviceFeatures& features)
            {
                self().native.features = features;
                return self();
            }

            inline void*                           get_next()     const { return self().native.pNext; }
            inline const VkPhysicalDeviceFeatures& get_features() const { return self().native.features; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan11Features>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_storage_buffer_16bit_access(VkBool32 v)              { self().native.storageBuffer16BitAccess = v;            return self(); }
            inline derived_t& set_uniform_and_storage_buffer_16bit_access(VkBool32 v)  { self().native.uniformAndStorageBuffer16BitAccess = v;  return self(); }
            inline derived_t& set_storage_push_constant_16(VkBool32 v)                 { self().native.storagePushConstant16 = v;               return self(); }
            inline derived_t& set_storage_input_output_16(VkBool32 v)                  { self().native.storageInputOutput16 = v;                return self(); }
            inline derived_t& set_multiview(VkBool32 v)                                { self().native.multiview = v;                           return self(); }
            inline derived_t& set_multiview_geometry_shader(VkBool32 v)                { self().native.multiviewGeometryShader = v;             return self(); }
            inline derived_t& set_multiview_tessellation_shader(VkBool32 v)            { self().native.multiviewTessellationShader = v;         return self(); }
            inline derived_t& set_variable_pointers_storage_buffer(VkBool32 v)         { self().native.variablePointersStorageBuffer = v;       return self(); }
            inline derived_t& set_variable_pointers(VkBool32 v)                        { self().native.variablePointers = v;                    return self(); }
            inline derived_t& set_protected_memory(VkBool32 v)                         { self().native.protectedMemory = v;                     return self(); }
            inline derived_t& set_sampler_ycbcr_conversion(VkBool32 v)                 { self().native.samplerYcbcrConversion = v;              return self(); }
            inline derived_t& set_shader_draw_parameters(VkBool32 v)                   { self().native.shaderDrawParameters = v;                return self(); }

            inline void*    get_next()                                    const { return self().native.pNext; }
            inline VkBool32 get_storage_buffer_16bit_access()             const { return self().native.storageBuffer16BitAccess; }
            inline VkBool32 get_uniform_and_storage_buffer_16bit_access() const { return self().native.uniformAndStorageBuffer16BitAccess; }
            inline VkBool32 get_storage_push_constant_16()                const { return self().native.storagePushConstant16; }
            inline VkBool32 get_storage_input_output_16()                 const { return self().native.storageInputOutput16; }
            inline VkBool32 get_multiview()                               const { return self().native.multiview; }
            inline VkBool32 get_multiview_geometry_shader()               const { return self().native.multiviewGeometryShader; }
            inline VkBool32 get_multiview_tessellation_shader()           const { return self().native.multiviewTessellationShader; }
            inline VkBool32 get_variable_pointers_storage_buffer()        const { return self().native.variablePointersStorageBuffer; }
            inline VkBool32 get_variable_pointers()                       const { return self().native.variablePointers; }
            inline VkBool32 get_protected_memory()                        const { return self().native.protectedMemory; }
            inline VkBool32 get_sampler_ycbcr_conversion()                const { return self().native.samplerYcbcrConversion; }
            inline VkBool32 get_shader_draw_parameters()                  const { return self().native.shaderDrawParameters; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan12Features>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_sampler_mirror_clamp_to_edge(VkBool32 v)                             { self().native.samplerMirrorClampToEdge = v;                            return self(); }
            inline derived_t& set_draw_indirect_count(VkBool32 v)                                      { self().native.drawIndirectCount = v;                                   return self(); }
            inline derived_t& set_storage_buffer_8bit_access(VkBool32 v)                               { self().native.storageBuffer8BitAccess = v;                             return self(); }
            inline derived_t& set_uniform_and_storage_buffer_8bit_access(VkBool32 v)                   { self().native.uniformAndStorageBuffer8BitAccess = v;                   return self(); }
            inline derived_t& set_storage_push_constant_8(VkBool32 v)                                  { self().native.storagePushConstant8 = v;                                return self(); }
            inline derived_t& set_shader_buffer_int64_atomics(VkBool32 v)                              { self().native.shaderBufferInt64Atomics = v;                            return self(); }
            inline derived_t& set_shader_shared_int64_atomics(VkBool32 v)                              { self().native.shaderSharedInt64Atomics = v;                            return self(); }
            inline derived_t& set_shader_float16(VkBool32 v)                                           { self().native.shaderFloat16 = v;                                       return self(); }
            inline derived_t& set_shader_int8(VkBool32 v)                                              { self().native.shaderInt8 = v;                                          return self(); }
            inline derived_t& set_descriptor_indexing(VkBool32 v)                                      { self().native.descriptorIndexing = v;                                  return self(); }
            inline derived_t& set_shader_input_attachment_array_dynamic_indexing(VkBool32 v)           { self().native.shaderInputAttachmentArrayDynamicIndexing = v;           return self(); }
            inline derived_t& set_shader_uniform_texel_buffer_array_dynamic_indexing(VkBool32 v)       { self().native.shaderUniformTexelBufferArrayDynamicIndexing = v;        return self(); }
            inline derived_t& set_shader_storage_texel_buffer_array_dynamic_indexing(VkBool32 v)       { self().native.shaderStorageTexelBufferArrayDynamicIndexing = v;        return self(); }
            inline derived_t& set_shader_uniform_buffer_array_non_uniform_indexing(VkBool32 v)         { self().native.shaderUniformBufferArrayNonUniformIndexing = v;          return self(); }
            inline derived_t& set_shader_sampled_image_array_non_uniform_indexing(VkBool32 v)          { self().native.shaderSampledImageArrayNonUniformIndexing = v;           return self(); }
            inline derived_t& set_shader_storage_buffer_array_non_uniform_indexing(VkBool32 v)         { self().native.shaderStorageBufferArrayNonUniformIndexing = v;          return self(); }
            inline derived_t& set_shader_storage_image_array_non_uniform_indexing(VkBool32 v)          { self().native.shaderStorageImageArrayNonUniformIndexing = v;           return self(); }
            inline derived_t& set_shader_input_attachment_array_non_uniform_indexing(VkBool32 v)       { self().native.shaderInputAttachmentArrayNonUniformIndexing = v;        return self(); }
            inline derived_t& set_shader_uniform_texel_buffer_array_non_uniform_indexing(VkBool32 v)   { self().native.shaderUniformTexelBufferArrayNonUniformIndexing = v;     return self(); }
            inline derived_t& set_shader_storage_texel_buffer_array_non_uniform_indexing(VkBool32 v)   { self().native.shaderStorageTexelBufferArrayNonUniformIndexing = v;     return self(); }
            inline derived_t& set_descriptor_binding_uniform_buffer_update_after_bind(VkBool32 v)      { self().native.descriptorBindingUniformBufferUpdateAfterBind = v;       return self(); }
            inline derived_t& set_descriptor_binding_sampled_image_update_after_bind(VkBool32 v)       { self().native.descriptorBindingSampledImageUpdateAfterBind = v;        return self(); }
            inline derived_t& set_descriptor_binding_storage_image_update_after_bind(VkBool32 v)       { self().native.descriptorBindingStorageImageUpdateAfterBind = v;        return self(); }
            inline derived_t& set_descriptor_binding_storage_buffer_update_after_bind(VkBool32 v)      { self().native.descriptorBindingStorageBufferUpdateAfterBind = v;       return self(); }
            inline derived_t& set_descriptor_binding_uniform_texel_buffer_update_after_bind(VkBool32 v){ self().native.descriptorBindingUniformTexelBufferUpdateAfterBind = v;  return self(); }
            inline derived_t& set_descriptor_binding_storage_texel_buffer_update_after_bind(VkBool32 v){ self().native.descriptorBindingStorageTexelBufferUpdateAfterBind = v;  return self(); }
            inline derived_t& set_descriptor_binding_update_unused_while_pending(VkBool32 v)           { self().native.descriptorBindingUpdateUnusedWhilePending = v;           return self(); }
            inline derived_t& set_descriptor_binding_partially_bound(VkBool32 v)                       { self().native.descriptorBindingPartiallyBound = v;                    return self(); }
            inline derived_t& set_descriptor_binding_variable_descriptor_count(VkBool32 v)             { self().native.descriptorBindingVariableDescriptorCount = v;           return self(); }
            inline derived_t& set_runtime_descriptor_array(VkBool32 v)                                 { self().native.runtimeDescriptorArray = v;                              return self(); }
            inline derived_t& set_sampler_filter_minmax(VkBool32 v)                                    { self().native.samplerFilterMinmax = v;                                 return self(); }
            inline derived_t& set_scalar_block_layout(VkBool32 v)                                      { self().native.scalarBlockLayout = v;                                   return self(); }
            inline derived_t& set_imageless_framebuffer(VkBool32 v)                                    { self().native.imagelessFramebuffer = v;                                return self(); }
            inline derived_t& set_uniform_buffer_standard_layout(VkBool32 v)                           { self().native.uniformBufferStandardLayout = v;                         return self(); }
            inline derived_t& set_shader_subgroup_extended_types(VkBool32 v)                           { self().native.shaderSubgroupExtendedTypes = v;                         return self(); }
            inline derived_t& set_separate_depth_stencil_layouts(VkBool32 v)                           { self().native.separateDepthStencilLayouts = v;                         return self(); }
            inline derived_t& set_host_query_reset(VkBool32 v)                                         { self().native.hostQueryReset = v;                                      return self(); }
            inline derived_t& set_timeline_semaphore(VkBool32 v)                                       { self().native.timelineSemaphore = v;                                   return self(); }
            inline derived_t& set_buffer_device_address(VkBool32 v)                                    { self().native.bufferDeviceAddress = v;                                 return self(); }
            inline derived_t& set_buffer_device_address_capture_replay(VkBool32 v)                     { self().native.bufferDeviceAddressCaptureReplay = v;                   return self(); }
            inline derived_t& set_buffer_device_address_multi_device(VkBool32 v)                       { self().native.bufferDeviceAddressMultiDevice = v;                      return self(); }
            inline derived_t& set_vulkan_memory_model(VkBool32 v)                                      { self().native.vulkanMemoryModel = v;                                   return self(); }
            inline derived_t& set_vulkan_memory_model_device_scope(VkBool32 v)                         { self().native.vulkanMemoryModelDeviceScope = v;                        return self(); }
            inline derived_t& set_vulkan_memory_model_availability_visibility_chains(VkBool32 v)       { self().native.vulkanMemoryModelAvailabilityVisibilityChains = v;       return self(); }
            inline derived_t& set_shader_output_viewport_index(VkBool32 v)                             { self().native.shaderOutputViewportIndex = v;                           return self(); }
            inline derived_t& set_shader_output_layer(VkBool32 v)                                      { self().native.shaderOutputLayer = v;                                   return self(); }
            inline derived_t& set_subgroup_broadcast_dynamic_id(VkBool32 v)                            { self().native.subgroupBroadcastDynamicId = v;                          return self(); }

            inline void*    get_next()                                                    const { return self().native.pNext; }
            inline VkBool32 get_sampler_mirror_clamp_to_edge()                            const { return self().native.samplerMirrorClampToEdge; }
            inline VkBool32 get_draw_indirect_count()                                     const { return self().native.drawIndirectCount; }
            inline VkBool32 get_storage_buffer_8bit_access()                              const { return self().native.storageBuffer8BitAccess; }
            inline VkBool32 get_uniform_and_storage_buffer_8bit_access()                  const { return self().native.uniformAndStorageBuffer8BitAccess; }
            inline VkBool32 get_storage_push_constant_8()                                 const { return self().native.storagePushConstant8; }
            inline VkBool32 get_shader_buffer_int64_atomics()                             const { return self().native.shaderBufferInt64Atomics; }
            inline VkBool32 get_shader_shared_int64_atomics()                             const { return self().native.shaderSharedInt64Atomics; }
            inline VkBool32 get_shader_float16()                                          const { return self().native.shaderFloat16; }
            inline VkBool32 get_shader_int8()                                             const { return self().native.shaderInt8; }
            inline VkBool32 get_descriptor_indexing()                                     const { return self().native.descriptorIndexing; }
            inline VkBool32 get_shader_input_attachment_array_dynamic_indexing()          const { return self().native.shaderInputAttachmentArrayDynamicIndexing; }
            inline VkBool32 get_shader_uniform_texel_buffer_array_dynamic_indexing()      const { return self().native.shaderUniformTexelBufferArrayDynamicIndexing; }
            inline VkBool32 get_shader_storage_texel_buffer_array_dynamic_indexing()      const { return self().native.shaderStorageTexelBufferArrayDynamicIndexing; }
            inline VkBool32 get_shader_uniform_buffer_array_non_uniform_indexing()        const { return self().native.shaderUniformBufferArrayNonUniformIndexing; }
            inline VkBool32 get_shader_sampled_image_array_non_uniform_indexing()         const { return self().native.shaderSampledImageArrayNonUniformIndexing; }
            inline VkBool32 get_shader_storage_buffer_array_non_uniform_indexing()        const { return self().native.shaderStorageBufferArrayNonUniformIndexing; }
            inline VkBool32 get_shader_storage_image_array_non_uniform_indexing()         const { return self().native.shaderStorageImageArrayNonUniformIndexing; }
            inline VkBool32 get_shader_input_attachment_array_non_uniform_indexing()      const { return self().native.shaderInputAttachmentArrayNonUniformIndexing; }
            inline VkBool32 get_shader_uniform_texel_buffer_array_non_uniform_indexing()  const { return self().native.shaderUniformTexelBufferArrayNonUniformIndexing; }
            inline VkBool32 get_shader_storage_texel_buffer_array_non_uniform_indexing()  const { return self().native.shaderStorageTexelBufferArrayNonUniformIndexing; }
            inline VkBool32 get_descriptor_binding_uniform_buffer_update_after_bind()     const { return self().native.descriptorBindingUniformBufferUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_sampled_image_update_after_bind()      const { return self().native.descriptorBindingSampledImageUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_storage_image_update_after_bind()      const { return self().native.descriptorBindingStorageImageUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_storage_buffer_update_after_bind()     const { return self().native.descriptorBindingStorageBufferUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_uniform_texel_buffer_update_after_bind() const { return self().native.descriptorBindingUniformTexelBufferUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_storage_texel_buffer_update_after_bind() const { return self().native.descriptorBindingStorageTexelBufferUpdateAfterBind; }
            inline VkBool32 get_descriptor_binding_update_unused_while_pending()          const { return self().native.descriptorBindingUpdateUnusedWhilePending; }
            inline VkBool32 get_descriptor_binding_partially_bound()                      const { return self().native.descriptorBindingPartiallyBound; }
            inline VkBool32 get_descriptor_binding_variable_descriptor_count()            const { return self().native.descriptorBindingVariableDescriptorCount; }
            inline VkBool32 get_runtime_descriptor_array()                                const { return self().native.runtimeDescriptorArray; }
            inline VkBool32 get_sampler_filter_minmax()                                   const { return self().native.samplerFilterMinmax; }
            inline VkBool32 get_scalar_block_layout()                                     const { return self().native.scalarBlockLayout; }
            inline VkBool32 get_imageless_framebuffer()                                   const { return self().native.imagelessFramebuffer; }
            inline VkBool32 get_uniform_buffer_standard_layout()                          const { return self().native.uniformBufferStandardLayout; }
            inline VkBool32 get_shader_subgroup_extended_types()                          const { return self().native.shaderSubgroupExtendedTypes; }
            inline VkBool32 get_separate_depth_stencil_layouts()                          const { return self().native.separateDepthStencilLayouts; }
            inline VkBool32 get_host_query_reset()                                        const { return self().native.hostQueryReset; }
            inline VkBool32 get_timeline_semaphore()                                      const { return self().native.timelineSemaphore; }
            inline VkBool32 get_buffer_device_address()                                   const { return self().native.bufferDeviceAddress; }
            inline VkBool32 get_buffer_device_address_capture_replay()                    const { return self().native.bufferDeviceAddressCaptureReplay; }
            inline VkBool32 get_buffer_device_address_multi_device()                      const { return self().native.bufferDeviceAddressMultiDevice; }
            inline VkBool32 get_vulkan_memory_model()                                     const { return self().native.vulkanMemoryModel; }
            inline VkBool32 get_vulkan_memory_model_device_scope()                        const { return self().native.vulkanMemoryModelDeviceScope; }
            inline VkBool32 get_vulkan_memory_model_availability_visibility_chains()      const { return self().native.vulkanMemoryModelAvailabilityVisibilityChains; }
            inline VkBool32 get_shader_output_viewport_index()                            const { return self().native.shaderOutputViewportIndex; }
            inline VkBool32 get_shader_output_layer()                                     const { return self().native.shaderOutputLayer; }
            inline VkBool32 get_subgroup_broadcast_dynamic_id()                           const { return self().native.subgroupBroadcastDynamicId; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan13Features>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_robust_image_access(VkBool32 v)                                      { self().native.robustImageAccess = v;                                   return self(); }
            inline derived_t& set_inline_uniform_block(VkBool32 v)                                     { self().native.inlineUniformBlock = v;                                  return self(); }
            inline derived_t& set_descriptor_binding_inline_uniform_block_update_after_bind(VkBool32 v){ self().native.descriptorBindingInlineUniformBlockUpdateAfterBind = v;  return self(); }
            inline derived_t& set_pipeline_creation_cache_control(VkBool32 v)                          { self().native.pipelineCreationCacheControl = v;                        return self(); }
            inline derived_t& set_private_data(VkBool32 v)                                             { self().native.privateData = v;                                         return self(); }
            inline derived_t& set_shader_demote_to_helper_invocation(VkBool32 v)                       { self().native.shaderDemoteToHelperInvocation = v;                      return self(); }
            inline derived_t& set_shader_terminate_invocation(VkBool32 v)                              { self().native.shaderTerminateInvocation = v;                           return self(); }
            inline derived_t& set_subgroup_size_control(VkBool32 v)                                    { self().native.subgroupSizeControl = v;                                 return self(); }
            inline derived_t& set_compute_full_subgroups(VkBool32 v)                                   { self().native.computeFullSubgroups = v;                                return self(); }
            inline derived_t& set_synchronization2(VkBool32 v)                                         { self().native.synchronization2 = v;                                    return self(); }
            inline derived_t& set_texture_compression_astc_hdr(VkBool32 v)                             { self().native.textureCompressionASTC_HDR = v;                          return self(); }
            inline derived_t& set_shader_zero_initialize_workgroup_memory(VkBool32 v)                  { self().native.shaderZeroInitializeWorkgroupMemory = v;                 return self(); }
            inline derived_t& set_dynamic_rendering(VkBool32 v)                                        { self().native.dynamicRendering = v;                                    return self(); }
            inline derived_t& set_shader_integer_dot_product(VkBool32 v)                               { self().native.shaderIntegerDotProduct = v;                             return self(); }
            inline derived_t& set_maintenance4(VkBool32 v)                                             { self().native.maintenance4 = v;                                        return self(); }

            inline void*    get_next()                                                    const { return self().native.pNext; }
            inline VkBool32 get_robust_image_access()                                     const { return self().native.robustImageAccess; }
            inline VkBool32 get_inline_uniform_block()                                    const { return self().native.inlineUniformBlock; }
            inline VkBool32 get_descriptor_binding_inline_uniform_block_update_after_bind() const { return self().native.descriptorBindingInlineUniformBlockUpdateAfterBind; }
            inline VkBool32 get_pipeline_creation_cache_control()                         const { return self().native.pipelineCreationCacheControl; }
            inline VkBool32 get_private_data()                                            const { return self().native.privateData; }
            inline VkBool32 get_shader_demote_to_helper_invocation()                      const { return self().native.shaderDemoteToHelperInvocation; }
            inline VkBool32 get_shader_terminate_invocation()                             const { return self().native.shaderTerminateInvocation; }
            inline VkBool32 get_subgroup_size_control()                                   const { return self().native.subgroupSizeControl; }
            inline VkBool32 get_compute_full_subgroups()                                  const { return self().native.computeFullSubgroups; }
            inline VkBool32 get_synchronization2()                                        const { return self().native.synchronization2; }
            inline VkBool32 get_texture_compression_astc_hdr()                            const { return self().native.textureCompressionASTC_HDR; }
            inline VkBool32 get_shader_zero_initialize_workgroup_memory()                 const { return self().native.shaderZeroInitializeWorkgroupMemory; }
            inline VkBool32 get_dynamic_rendering()                                       const { return self().native.dynamicRendering; }
            inline VkBool32 get_shader_integer_dot_product()                              const { return self().native.shaderIntegerDotProduct; }
            inline VkBool32 get_maintenance4()                                            const { return self().native.maintenance4; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan14Features>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline derived_t& set_next(void* next)
            {
                self().native.pNext = next;
                return self();
            }

            inline derived_t& set_global_priority_query(VkBool32 v)                    { self().native.globalPriorityQuery = v;                  return self(); }
            inline derived_t& set_shader_subgroup_rotate(VkBool32 v)                   { self().native.shaderSubgroupRotate = v;                 return self(); }
            inline derived_t& set_shader_subgroup_rotate_clustered(VkBool32 v)         { self().native.shaderSubgroupRotateClustered = v;        return self(); }
            inline derived_t& set_shader_float_controls2(VkBool32 v)                   { self().native.shaderFloatControls2 = v;                 return self(); }
            inline derived_t& set_shader_expect_assume(VkBool32 v)                     { self().native.shaderExpectAssume = v;                   return self(); }
            inline derived_t& set_rectangular_lines(VkBool32 v)                        { self().native.rectangularLines = v;                     return self(); }
            inline derived_t& set_bresenham_lines(VkBool32 v)                          { self().native.bresenhamLines = v;                       return self(); }
            inline derived_t& set_smooth_lines(VkBool32 v)                             { self().native.smoothLines = v;                          return self(); }
            inline derived_t& set_stippled_rectangular_lines(VkBool32 v)               { self().native.stippledRectangularLines = v;             return self(); }
            inline derived_t& set_stippled_bresenham_lines(VkBool32 v)                 { self().native.stippledBresenhamLines = v;               return self(); }
            inline derived_t& set_stippled_smooth_lines(VkBool32 v)                    { self().native.stippledSmoothLines = v;                  return self(); }
            inline derived_t& set_vertex_attribute_instance_rate_divisor(VkBool32 v)   { self().native.vertexAttributeInstanceRateDivisor = v;   return self(); }
            inline derived_t& set_vertex_attribute_instance_rate_zero_divisor(VkBool32 v) { self().native.vertexAttributeInstanceRateZeroDivisor = v; return self(); }
            inline derived_t& set_index_type_uint8(VkBool32 v)                         { self().native.indexTypeUint8 = v;                       return self(); }
            inline derived_t& set_dynamic_rendering_local_read(VkBool32 v)             { self().native.dynamicRenderingLocalRead = v;            return self(); }
            inline derived_t& set_maintenance5(VkBool32 v)                             { self().native.maintenance5 = v;                         return self(); }
            inline derived_t& set_maintenance6(VkBool32 v)                             { self().native.maintenance6 = v;                         return self(); }
            inline derived_t& set_pipeline_protected_access(VkBool32 v)                { self().native.pipelineProtectedAccess = v;              return self(); }
            inline derived_t& set_pipeline_robustness(VkBool32 v)                      { self().native.pipelineRobustness = v;                   return self(); }
            inline derived_t& set_host_image_copy(VkBool32 v)                          { self().native.hostImageCopy = v;                        return self(); }
            inline derived_t& set_push_descriptor(VkBool32 v)                          { self().native.pushDescriptor = v;                       return self(); }

            inline void*    get_next()                                      const { return self().native.pNext; }
            inline VkBool32 get_global_priority_query()                     const { return self().native.globalPriorityQuery; }
            inline VkBool32 get_shader_subgroup_rotate()                    const { return self().native.shaderSubgroupRotate; }
            inline VkBool32 get_shader_subgroup_rotate_clustered()          const { return self().native.shaderSubgroupRotateClustered; }
            inline VkBool32 get_shader_float_controls2()                    const { return self().native.shaderFloatControls2; }
            inline VkBool32 get_shader_expect_assume()                      const { return self().native.shaderExpectAssume; }
            inline VkBool32 get_rectangular_lines()                         const { return self().native.rectangularLines; }
            inline VkBool32 get_bresenham_lines()                           const { return self().native.bresenhamLines; }
            inline VkBool32 get_smooth_lines()                              const { return self().native.smoothLines; }
            inline VkBool32 get_stippled_rectangular_lines()                const { return self().native.stippledRectangularLines; }
            inline VkBool32 get_stippled_bresenham_lines()                  const { return self().native.stippledBresenhamLines; }
            inline VkBool32 get_stippled_smooth_lines()                     const { return self().native.stippledSmoothLines; }
            inline VkBool32 get_vertex_attribute_instance_rate_divisor()    const { return self().native.vertexAttributeInstanceRateDivisor; }
            inline VkBool32 get_vertex_attribute_instance_rate_zero_divisor() const { return self().native.vertexAttributeInstanceRateZeroDivisor; }
            inline VkBool32 get_index_type_uint8()                          const { return self().native.indexTypeUint8; }
            inline VkBool32 get_dynamic_rendering_local_read()              const { return self().native.dynamicRenderingLocalRead; }
            inline VkBool32 get_maintenance5()                              const { return self().native.maintenance5; }
            inline VkBool32 get_maintenance6()                              const { return self().native.maintenance6; }
            inline VkBool32 get_pipeline_protected_access()                 const { return self().native.pipelineProtectedAccess; }
            inline VkBool32 get_pipeline_robustness()                       const { return self().native.pipelineRobustness; }
            inline VkBool32 get_host_image_copy()                           const { return self().native.hostImageCopy; }
            inline VkBool32 get_push_descriptor()                           const { return self().native.pushDescriptor; }

    };

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkLayerProperties>
    {

        private:
            inline derived_t&       self()       { return *static_cast<derived_t*>(this); }
            inline const derived_t& self() const { return *static_cast<const derived_t*>(this); }

        public:
            inline spx::string_view<char> get_layer_name()           const { return { self().native.layerName }; }
            inline uint32_t               get_spec_version()         const { return self().native.specVersion; }
            inline uint32_t               get_implementation_version() const { return self().native.implementationVersion; }
            inline spx::string_view<char> get_description()          const { return { self().native.description }; }

    };

}