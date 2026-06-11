#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#if defined(_WIN32)
#   if !defined(WIN32_LEAN_AND_MEAN)
#       define WIN32_LEAN_AND_MEAN
#   endif
#   if !defined(NOMINMAX)
#       define NOMINMAX
#   endif
#   define GLFW_EXPOSE_NATIVE_WIN32
#   include <GLFW/glfw3native.h>
#endif

#include <utils/defs.hpp>
#include <simplex/string_view.hpp>

/// @brief  Status results for window helper operations that are not guaranteed
///         to succeed, such as Vulkan surface creation or native handle retrieval.
///         Any value that isn't WindowStatus_OK indicates the out-parameters were
///         not populated with usable values.
enum WindowStatus
{
    WindowStatus_OK,
    WindowStatus_InvalidArgument,
    WindowStatus_WindowInvalid,
    WindowStatus_VulkanUnsupported,
    WindowStatus_VulkanSurfaceFailed,
    WindowStatus_NativeHandleUnavailable,
};

namespace spx
{

    /// @brief  The window interface serves as the primary way to pass around a window
    ///         without knowing the underlying implementation. In the event that platform
    ///         internals are required, casting to platform window type is possible through
    ///         platform detection macros.
    ///
    ///         When a window is created, it is by default not shown to the user.
    ///         The user must manually invoke .show() to display the window.
    ///
    ///         The window interface is trivially copyable; meaning the creation and destruction
    ///         must be invoked manually. In most cases, the intent is clear about who creates
    ///         and destroys a window.
    class window_interface
    {
        public:
            inline          window_interface() = default;
            inline virtual ~window_interface() = default;

            /// @brief Creates a window.
            /// @param window_title The title to give to the window.
            /// @param width The initial starting width of the window.
            /// @param height The initial starting height of the window.
            /// @return True if creation succeeded, false otherwise.
            inline virtual bool create(spx::string_view<char> window_title, uint32_t width, uint32_t height) = 0;

            /// @brief Destroys the window and invalidates the internal handle.
            inline virtual void destroy() = 0;

            /// @brief Polls all the platform events for the window.
            inline virtual void poll_events() = 0;

            /// @brief Hides the window from the user.
            inline virtual void hide() = 0;

            /// @brief Shows the window (if it is hidden).
            inline virtual void show() = 0;

            /// @brief Maximizes the window, behavior is platform dependent.
            inline virtual void maximize() = 0;

            /// @brief Enables fullscreen-borderless mode.
            inline virtual void borderless() = 0;

            /// @brief Normalizes the window (if it is minimized or maximized).
            inline virtual void normalize() = 0;

            /// @brief Prevents resizing and maximization.
            inline virtual void lock_resizing() = 0;

            /// @brief Allows resizing and maximization.
            inline virtual void unlock_resizing() = 0;

            /// @brief Determines if the window is valid.
            /// @return True if it is.
            inline virtual bool is_valid() const = 0;

            /// @brief Determines if the window should be closed and destroyed.
            /// @return True if it should.
            inline virtual bool should_close() const = 0;

            /// @brief Returns the window's width.
            /// @return The window's width.
            inline virtual int32_t get_width() const = 0;

            /// @brief Returns the window's height.
            /// @return The window's height.
            inline virtual int32_t get_height() const = 0;
            
            /// @brief Returns the window's aspect ration.
            /// @return The window's aspect ratio.
            inline virtual real32_t get_aspect_ratio() const = 0;

            /// @brief Returns whether or not the window is visible (platform dependent).
            /// @return True if it is visible.
            inline virtual bool is_visible() const = 0;

            /// @brief Returns whether or not the window is hidden (platform dependent).
            /// @return True if it is hidden.
            inline virtual bool is_hidden() const = 0;

            /// @brief Returns whether or not the window is minimized.
            /// @return True if it is.
            inline virtual bool is_minimized() const = 0;

            /// @brief Returns whether or not the window is maximized.
            /// @return True if it is.
            inline virtual bool is_maximized() const = 0;

            /// @brief Returns whether or not the window is a normal state (not maximized/fullscreen/minimized).
            /// @return True if it is.
            inline virtual bool is_normalized() const = 0;

            /// @brief Returns if the window is actively focused (platform dependent).
            /// @return True if it is.
            inline virtual bool is_active() const = 0;

            /// @brief Returns if the window is not the active focus.
            /// @return True if it is.
            inline virtual bool is_inactive() const = 0;

            /// @brief  Retrieves the list of Vulkan instance extensions the windowing
            ///         system requires to create presentation surfaces. The returned list
            ///         is owned by the platform layer and must not be freed by the caller.
            ///         Platforms which can not present with Vulkan return
            ///         WindowStatus_VulkanUnsupported; this default is overridden by
            ///         specializations which can.
            /// @param extension_list Receives the extension name list.
            /// @param extension_count Receives the number of entries in the list.
            /// @return WindowStatus_OK if the list was populated.
            inline virtual WindowStatus
            get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const
            {

                if (extension_list == NULL || extension_count == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *extension_list = NULL;
                *extension_count = 0;
                return WindowStatus_VulkanUnsupported;

            }

            /// @brief  Creates a Vulkan presentation surface for this window. The caller
            ///         owns the surface and is responsible for destroying it before the
            ///         instance is destroyed. Platforms which can not present with Vulkan
            ///         return WindowStatus_VulkanUnsupported; this default is overridden
            ///         by specializations which can.
            /// @param instance The Vulkan instance to create the surface against.
            /// @param allocator The Vulkan allocation callbacks, or NULL for the defaults.
            /// @param surface Receives the created surface.
            /// @return WindowStatus_OK if the surface was created.
            inline virtual WindowStatus
            create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const
            {

                if (surface == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *surface = VK_NULL_HANDLE;
                return WindowStatus_VulkanUnsupported;

            }

            /// @brief  Retrieves the platform-native window handle. On win32 platforms this
            ///         is the HWND required to create DX11/DX12 (DXGI) swap chains. Platforms
            ///         without a retrievable native handle return
            ///         WindowStatus_NativeHandleUnavailable; this default is overridden by
            ///         specializations which have one.
            /// @param native_handle Receives the native handle.
            /// @return WindowStatus_OK if the handle was retrieved.
            inline virtual WindowStatus
            get_native_handle(void **native_handle) const
            {

                if (native_handle == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *native_handle = NULL;
                return WindowStatus_NativeHandleUnavailable;

            }

    };

}