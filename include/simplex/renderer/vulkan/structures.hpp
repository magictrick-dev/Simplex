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

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkInstanceCreateInfo mixin.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkInstanceCreateInfo>
    {

        VkStructureType             sType                   { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO    };
        const void*                 pNext                   { nullptr                                   };
        VkInstanceCreateFlags       flags                   {                                           };
        const vk_struct_base<VkApplicationInfo>* pApplicationInfo { nullptr                             };
        uint32_t                    enabledLayerCount       {                                           };
        const char* const*          ppEnabledLayerNames     { nullptr                                   };
        uint32_t                    enabledExtensionCount   {                                           };
        const char* const*          ppEnabledExtensionNames { nullptr                                   };

        inline const void*                  get_next() const                { return this->pNext;                                                       }
        inline VkInstanceCreateFlags        get_flags() const               { return this->flags;                                                       }
        inline const VkApplicationInfo*     get_application_info() const    { return this->pApplicationInfo;                                            }
        inline spx::array_view<const char*> get_layers() const              { return { this->ppEnabledLayerNames, this->enabledLayerCount };            }
        inline spx::array_view<const char*> get_extensions() const          { return { this->ppEnabledExtensionNames, this->enabledExtensionCount };    }

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

    /// @brief VkDebugUtilsMessengerCreateInfoEXT mixin (debug_utils_messenger_create_info_t).
    ///
    /// The pfnUserCallback is left for the caller to supply (set_user_callback); the actual callback
    /// implementation lives outside this layer so it can be wired into the logging system.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkDebugUtilsMessengerCreateInfoEXT>
    {

        VkStructureType                         sType            { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        const void*                             pNext            { nullptr };
        VkDebugUtilsMessengerCreateFlagsEXT     flags            {         };
        VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity  {         };
        VkDebugUtilsMessageTypeFlagsEXT         messageType      {         };
        PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback  { nullptr };
        void*                                   pUserData        { nullptr };

        inline const void*                          get_next() const            { return this->pNext;            }
        inline VkDebugUtilsMessengerCreateFlagsEXT  get_flags() const           { return this->flags;            }
        inline VkDebugUtilsMessageSeverityFlagsEXT  get_message_severity() const{ return this->messageSeverity;  }
        inline VkDebugUtilsMessageTypeFlagsEXT      get_message_type() const    { return this->messageType;      }
        inline PFN_vkDebugUtilsMessengerCallbackEXT get_user_callback() const   { return this->pfnUserCallback;  }
        inline void*                                get_user_data() const       { return this->pUserData;        }

        inline derived_t& set_next(const void* next)                                    { this->pNext = next; return *s();                  }
        inline derived_t& set_flags(VkDebugUtilsMessengerCreateFlagsEXT flags)          { this->flags = flags; return *s();                 }
        inline derived_t& set_message_severity(VkDebugUtilsMessageSeverityFlagsEXT severity) { this->messageSeverity = severity; return *s();    }
        inline derived_t& set_message_type(VkDebugUtilsMessageTypeFlagsEXT type)        { this->messageType = type; return *s();            }
        inline derived_t& set_user_callback(PFN_vkDebugUtilsMessengerCallbackEXT cb)    { this->pfnUserCallback = cb; return *s();          }
        inline derived_t& set_user_data(void* data)                                     { this->pUserData = data; return *s();              }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceFeatures mixin (physical_device_features).
    ///
    /// The non-sType base feature set. These are toggled/read by name, so the bool members are left
    /// public for direct access (e.g. feats.samplerAnisotropy = VK_TRUE) rather than carrying a
    /// fluent setter per field. Reused as the nested member of physical_device_10_features below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceFeatures>
    {

        VkBool32    robustBufferAccess                          {         };
        VkBool32    fullDrawIndexUint32                         {         };
        VkBool32    imageCubeArray                              {         };
        VkBool32    independentBlend                            {         };
        VkBool32    geometryShader                              {         };
        VkBool32    tessellationShader                          {         };
        VkBool32    sampleRateShading                           {         };
        VkBool32    dualSrcBlend                                {         };
        VkBool32    logicOp                                     {         };
        VkBool32    multiDrawIndirect                           {         };
        VkBool32    drawIndirectFirstInstance                   {         };
        VkBool32    depthClamp                                  {         };
        VkBool32    depthBiasClamp                              {         };
        VkBool32    fillModeNonSolid                            {         };
        VkBool32    depthBounds                                 {         };
        VkBool32    wideLines                                   {         };
        VkBool32    largePoints                                 {         };
        VkBool32    alphaToOne                                  {         };
        VkBool32    multiViewport                               {         };
        VkBool32    samplerAnisotropy                           {         };
        VkBool32    textureCompressionETC2                      {         };
        VkBool32    textureCompressionASTC_LDR                  {         };
        VkBool32    textureCompressionBC                        {         };
        VkBool32    occlusionQueryPrecise                       {         };
        VkBool32    pipelineStatisticsQuery                     {         };
        VkBool32    vertexPipelineStoresAndAtomics              {         };
        VkBool32    fragmentStoresAndAtomics                    {         };
        VkBool32    shaderTessellationAndGeometryPointSize      {         };
        VkBool32    shaderImageGatherExtended                   {         };
        VkBool32    shaderStorageImageExtendedFormats           {         };
        VkBool32    shaderStorageImageMultisample               {         };
        VkBool32    shaderStorageImageReadWithoutFormat         {         };
        VkBool32    shaderStorageImageWriteWithoutFormat        {         };
        VkBool32    shaderUniformBufferArrayDynamicIndexing     {         };
        VkBool32    shaderSampledImageArrayDynamicIndexing      {         };
        VkBool32    shaderStorageBufferArrayDynamicIndexing     {         };
        VkBool32    shaderStorageImageArrayDynamicIndexing      {         };
        VkBool32    shaderClipDistance                          {         };
        VkBool32    shaderCullDistance                          {         };
        VkBool32    shaderFloat64                               {         };
        VkBool32    shaderInt64                                 {         };
        VkBool32    shaderInt16                                 {         };
        VkBool32    shaderResourceResidency                     {         };
        VkBool32    shaderResourceMinLod                        {         };
        VkBool32    sparseBinding                               {         };
        VkBool32    sparseResidencyBuffer                       {         };
        VkBool32    sparseResidencyImage2D                      {         };
        VkBool32    sparseResidencyImage3D                      {         };
        VkBool32    sparseResidency2Samples                     {         };
        VkBool32    sparseResidency4Samples                     {         };
        VkBool32    sparseResidency8Samples                     {         };
        VkBool32    sparseResidency16Samples                    {         };
        VkBool32    sparseResidencyAliased                      {         };
        VkBool32    variableMultisampleRate                     {         };
        VkBool32    inheritedQueries                            {         };

    };

    /// @brief VkPhysicalDeviceFeatures2 mixin (physical_device_10_features).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceFeatures2>
    {

        VkStructureType                           sType     { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        void*                                     pNext     { nullptr                                      };
        vk_struct_base<VkPhysicalDeviceFeatures>  features  {                                              };

        inline void*                                            get_next() const     { return this->pNext;       }
        inline const vk_struct_base<VkPhysicalDeviceFeatures>&  get_features() const { return this->features;    }
        inline vk_struct_base<VkPhysicalDeviceFeatures>&        get_features()       { return this->features;    }

        inline derived_t& set_next(void* next) { this->pNext = next; return *s(); }

        /// @brief Returns true if every base feature enabled in `requested` is also enabled in
        ///        `available` -- i.e. `requested` is a subset of `available`. Used to check a
        ///        requested feature set against what a physical device actually supports.
        static inline bool32_t
        is_subset(const derived_t& requested, const derived_t& available)
        {
            const vk_struct_base<VkPhysicalDeviceFeatures>& r = requested.features;
            const vk_struct_base<VkPhysicalDeviceFeatures>& a = available.features;

            if (r.robustBufferAccess && !a.robustBufferAccess) return false;
            if (r.fullDrawIndexUint32 && !a.fullDrawIndexUint32) return false;
            if (r.imageCubeArray && !a.imageCubeArray) return false;
            if (r.independentBlend && !a.independentBlend) return false;
            if (r.geometryShader && !a.geometryShader) return false;
            if (r.tessellationShader && !a.tessellationShader) return false;
            if (r.sampleRateShading && !a.sampleRateShading) return false;
            if (r.dualSrcBlend && !a.dualSrcBlend) return false;
            if (r.logicOp && !a.logicOp) return false;
            if (r.multiDrawIndirect && !a.multiDrawIndirect) return false;
            if (r.drawIndirectFirstInstance && !a.drawIndirectFirstInstance) return false;
            if (r.depthClamp && !a.depthClamp) return false;
            if (r.depthBiasClamp && !a.depthBiasClamp) return false;
            if (r.fillModeNonSolid && !a.fillModeNonSolid) return false;
            if (r.depthBounds && !a.depthBounds) return false;
            if (r.wideLines && !a.wideLines) return false;
            if (r.largePoints && !a.largePoints) return false;
            if (r.alphaToOne && !a.alphaToOne) return false;
            if (r.multiViewport && !a.multiViewport) return false;
            if (r.samplerAnisotropy && !a.samplerAnisotropy) return false;
            if (r.textureCompressionETC2 && !a.textureCompressionETC2) return false;
            if (r.textureCompressionASTC_LDR && !a.textureCompressionASTC_LDR) return false;
            if (r.textureCompressionBC && !a.textureCompressionBC) return false;
            if (r.occlusionQueryPrecise && !a.occlusionQueryPrecise) return false;
            if (r.pipelineStatisticsQuery && !a.pipelineStatisticsQuery) return false;
            if (r.vertexPipelineStoresAndAtomics && !a.vertexPipelineStoresAndAtomics) return false;
            if (r.fragmentStoresAndAtomics && !a.fragmentStoresAndAtomics) return false;
            if (r.shaderTessellationAndGeometryPointSize && !a.shaderTessellationAndGeometryPointSize) return false;
            if (r.shaderImageGatherExtended && !a.shaderImageGatherExtended) return false;
            if (r.shaderStorageImageExtendedFormats && !a.shaderStorageImageExtendedFormats) return false;
            if (r.shaderStorageImageMultisample && !a.shaderStorageImageMultisample) return false;
            if (r.shaderStorageImageReadWithoutFormat && !a.shaderStorageImageReadWithoutFormat) return false;
            if (r.shaderStorageImageWriteWithoutFormat && !a.shaderStorageImageWriteWithoutFormat) return false;
            if (r.shaderUniformBufferArrayDynamicIndexing && !a.shaderUniformBufferArrayDynamicIndexing) return false;
            if (r.shaderSampledImageArrayDynamicIndexing && !a.shaderSampledImageArrayDynamicIndexing) return false;
            if (r.shaderStorageBufferArrayDynamicIndexing && !a.shaderStorageBufferArrayDynamicIndexing) return false;
            if (r.shaderStorageImageArrayDynamicIndexing && !a.shaderStorageImageArrayDynamicIndexing) return false;
            if (r.shaderClipDistance && !a.shaderClipDistance) return false;
            if (r.shaderCullDistance && !a.shaderCullDistance) return false;
            if (r.shaderFloat64 && !a.shaderFloat64) return false;
            if (r.shaderInt64 && !a.shaderInt64) return false;
            if (r.shaderInt16 && !a.shaderInt16) return false;
            if (r.shaderResourceResidency && !a.shaderResourceResidency) return false;
            if (r.shaderResourceMinLod && !a.shaderResourceMinLod) return false;
            if (r.sparseBinding && !a.sparseBinding) return false;
            if (r.sparseResidencyBuffer && !a.sparseResidencyBuffer) return false;
            if (r.sparseResidencyImage2D && !a.sparseResidencyImage2D) return false;
            if (r.sparseResidencyImage3D && !a.sparseResidencyImage3D) return false;
            if (r.sparseResidency2Samples && !a.sparseResidency2Samples) return false;
            if (r.sparseResidency4Samples && !a.sparseResidency4Samples) return false;
            if (r.sparseResidency8Samples && !a.sparseResidency8Samples) return false;
            if (r.sparseResidency16Samples && !a.sparseResidency16Samples) return false;
            if (r.sparseResidencyAliased && !a.sparseResidencyAliased) return false;
            if (r.variableMultisampleRate && !a.variableMultisampleRate) return false;
            if (r.inheritedQueries && !a.inheritedQueries) return false;

            return true;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan11Features mixin (physical_device_11_features).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan11Features>
    {

        VkStructureType     sType                               { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
        void*               pNext                               { nullptr };
        VkBool32            storageBuffer16BitAccess            {         };
        VkBool32            uniformAndStorageBuffer16BitAccess  {         };
        VkBool32            storagePushConstant16               {         };
        VkBool32            storageInputOutput16                {         };
        VkBool32            multiview                           {         };
        VkBool32            multiviewGeometryShader             {         };
        VkBool32            multiviewTessellationShader         {         };
        VkBool32            variablePointersStorageBuffer       {         };
        VkBool32            variablePointers                    {         };
        VkBool32            protectedMemory                     {         };
        VkBool32            samplerYcbcrConversion              {         };
        VkBool32            shaderDrawParameters                {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        /// @brief Returns true if every feature enabled in `requested` is also enabled in
        ///        `available` -- i.e. `requested` is a subset of `available`. Used to check a
        ///        requested feature set against what a physical device actually supports.
        static inline bool32_t
        is_subset(const derived_t& r, const derived_t& a)
        {
            if (r.storageBuffer16BitAccess && !a.storageBuffer16BitAccess) return false;
            if (r.uniformAndStorageBuffer16BitAccess && !a.uniformAndStorageBuffer16BitAccess) return false;
            if (r.storagePushConstant16 && !a.storagePushConstant16) return false;
            if (r.storageInputOutput16 && !a.storageInputOutput16) return false;
            if (r.multiview && !a.multiview) return false;
            if (r.multiviewGeometryShader && !a.multiviewGeometryShader) return false;
            if (r.multiviewTessellationShader && !a.multiviewTessellationShader) return false;
            if (r.variablePointersStorageBuffer && !a.variablePointersStorageBuffer) return false;
            if (r.variablePointers && !a.variablePointers) return false;
            if (r.protectedMemory && !a.protectedMemory) return false;
            if (r.samplerYcbcrConversion && !a.samplerYcbcrConversion) return false;
            if (r.shaderDrawParameters && !a.shaderDrawParameters) return false;

            return true;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan12Features mixin (physical_device_12_features).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan12Features>
    {

        VkStructureType     sType                                           { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        void*               pNext                                           { nullptr };
        VkBool32            samplerMirrorClampToEdge                        {         };
        VkBool32            drawIndirectCount                               {         };
        VkBool32            storageBuffer8BitAccess                         {         };
        VkBool32            uniformAndStorageBuffer8BitAccess               {         };
        VkBool32            storagePushConstant8                            {         };
        VkBool32            shaderBufferInt64Atomics                        {         };
        VkBool32            shaderSharedInt64Atomics                        {         };
        VkBool32            shaderFloat16                                   {         };
        VkBool32            shaderInt8                                      {         };
        VkBool32            descriptorIndexing                              {         };
        VkBool32            shaderInputAttachmentArrayDynamicIndexing       {         };
        VkBool32            shaderUniformTexelBufferArrayDynamicIndexing    {         };
        VkBool32            shaderStorageTexelBufferArrayDynamicIndexing    {         };
        VkBool32            shaderUniformBufferArrayNonUniformIndexing      {         };
        VkBool32            shaderSampledImageArrayNonUniformIndexing       {         };
        VkBool32            shaderStorageBufferArrayNonUniformIndexing      {         };
        VkBool32            shaderStorageImageArrayNonUniformIndexing       {         };
        VkBool32            shaderInputAttachmentArrayNonUniformIndexing    {         };
        VkBool32            shaderUniformTexelBufferArrayNonUniformIndexing {         };
        VkBool32            shaderStorageTexelBufferArrayNonUniformIndexing {         };
        VkBool32            descriptorBindingUniformBufferUpdateAfterBind   {         };
        VkBool32            descriptorBindingSampledImageUpdateAfterBind    {         };
        VkBool32            descriptorBindingStorageImageUpdateAfterBind    {         };
        VkBool32            descriptorBindingStorageBufferUpdateAfterBind   {         };
        VkBool32            descriptorBindingUniformTexelBufferUpdateAfterBind  {     };
        VkBool32            descriptorBindingStorageTexelBufferUpdateAfterBind  {     };
        VkBool32            descriptorBindingUpdateUnusedWhilePending       {         };
        VkBool32            descriptorBindingPartiallyBound                 {         };
        VkBool32            descriptorBindingVariableDescriptorCount        {         };
        VkBool32            runtimeDescriptorArray                          {         };
        VkBool32            samplerFilterMinmax                             {         };
        VkBool32            scalarBlockLayout                               {         };
        VkBool32            imagelessFramebuffer                            {         };
        VkBool32            uniformBufferStandardLayout                     {         };
        VkBool32            shaderSubgroupExtendedTypes                     {         };
        VkBool32            separateDepthStencilLayouts                     {         };
        VkBool32            hostQueryReset                                  {         };
        VkBool32            timelineSemaphore                               {         };
        VkBool32            bufferDeviceAddress                             {         };
        VkBool32            bufferDeviceAddressCaptureReplay                {         };
        VkBool32            bufferDeviceAddressMultiDevice                  {         };
        VkBool32            vulkanMemoryModel                               {         };
        VkBool32            vulkanMemoryModelDeviceScope                    {         };
        VkBool32            vulkanMemoryModelAvailabilityVisibilityChains   {         };
        VkBool32            shaderOutputViewportIndex                       {         };
        VkBool32            shaderOutputLayer                               {         };
        VkBool32            subgroupBroadcastDynamicId                      {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        /// @brief Returns true if every feature enabled in `requested` is also enabled in
        ///        `available` -- i.e. `requested` is a subset of `available`. Used to check a
        ///        requested feature set against what a physical device actually supports.
        static inline bool32_t
        is_subset(const derived_t& r, const derived_t& a)
        {
            if (r.samplerMirrorClampToEdge && !a.samplerMirrorClampToEdge) return false;
            if (r.drawIndirectCount && !a.drawIndirectCount) return false;
            if (r.storageBuffer8BitAccess && !a.storageBuffer8BitAccess) return false;
            if (r.uniformAndStorageBuffer8BitAccess && !a.uniformAndStorageBuffer8BitAccess) return false;
            if (r.storagePushConstant8 && !a.storagePushConstant8) return false;
            if (r.shaderBufferInt64Atomics && !a.shaderBufferInt64Atomics) return false;
            if (r.shaderSharedInt64Atomics && !a.shaderSharedInt64Atomics) return false;
            if (r.shaderFloat16 && !a.shaderFloat16) return false;
            if (r.shaderInt8 && !a.shaderInt8) return false;
            if (r.descriptorIndexing && !a.descriptorIndexing) return false;
            if (r.shaderInputAttachmentArrayDynamicIndexing && !a.shaderInputAttachmentArrayDynamicIndexing) return false;
            if (r.shaderUniformTexelBufferArrayDynamicIndexing && !a.shaderUniformTexelBufferArrayDynamicIndexing) return false;
            if (r.shaderStorageTexelBufferArrayDynamicIndexing && !a.shaderStorageTexelBufferArrayDynamicIndexing) return false;
            if (r.shaderUniformBufferArrayNonUniformIndexing && !a.shaderUniformBufferArrayNonUniformIndexing) return false;
            if (r.shaderSampledImageArrayNonUniformIndexing && !a.shaderSampledImageArrayNonUniformIndexing) return false;
            if (r.shaderStorageBufferArrayNonUniformIndexing && !a.shaderStorageBufferArrayNonUniformIndexing) return false;
            if (r.shaderStorageImageArrayNonUniformIndexing && !a.shaderStorageImageArrayNonUniformIndexing) return false;
            if (r.shaderInputAttachmentArrayNonUniformIndexing && !a.shaderInputAttachmentArrayNonUniformIndexing) return false;
            if (r.shaderUniformTexelBufferArrayNonUniformIndexing && !a.shaderUniformTexelBufferArrayNonUniformIndexing) return false;
            if (r.shaderStorageTexelBufferArrayNonUniformIndexing && !a.shaderStorageTexelBufferArrayNonUniformIndexing) return false;
            if (r.descriptorBindingUniformBufferUpdateAfterBind && !a.descriptorBindingUniformBufferUpdateAfterBind) return false;
            if (r.descriptorBindingSampledImageUpdateAfterBind && !a.descriptorBindingSampledImageUpdateAfterBind) return false;
            if (r.descriptorBindingStorageImageUpdateAfterBind && !a.descriptorBindingStorageImageUpdateAfterBind) return false;
            if (r.descriptorBindingStorageBufferUpdateAfterBind && !a.descriptorBindingStorageBufferUpdateAfterBind) return false;
            if (r.descriptorBindingUniformTexelBufferUpdateAfterBind && !a.descriptorBindingUniformTexelBufferUpdateAfterBind) return false;
            if (r.descriptorBindingStorageTexelBufferUpdateAfterBind && !a.descriptorBindingStorageTexelBufferUpdateAfterBind) return false;
            if (r.descriptorBindingUpdateUnusedWhilePending && !a.descriptorBindingUpdateUnusedWhilePending) return false;
            if (r.descriptorBindingPartiallyBound && !a.descriptorBindingPartiallyBound) return false;
            if (r.descriptorBindingVariableDescriptorCount && !a.descriptorBindingVariableDescriptorCount) return false;
            if (r.runtimeDescriptorArray && !a.runtimeDescriptorArray) return false;
            if (r.samplerFilterMinmax && !a.samplerFilterMinmax) return false;
            if (r.scalarBlockLayout && !a.scalarBlockLayout) return false;
            if (r.imagelessFramebuffer && !a.imagelessFramebuffer) return false;
            if (r.uniformBufferStandardLayout && !a.uniformBufferStandardLayout) return false;
            if (r.shaderSubgroupExtendedTypes && !a.shaderSubgroupExtendedTypes) return false;
            if (r.separateDepthStencilLayouts && !a.separateDepthStencilLayouts) return false;
            if (r.hostQueryReset && !a.hostQueryReset) return false;
            if (r.timelineSemaphore && !a.timelineSemaphore) return false;
            if (r.bufferDeviceAddress && !a.bufferDeviceAddress) return false;
            if (r.bufferDeviceAddressCaptureReplay && !a.bufferDeviceAddressCaptureReplay) return false;
            if (r.bufferDeviceAddressMultiDevice && !a.bufferDeviceAddressMultiDevice) return false;
            if (r.vulkanMemoryModel && !a.vulkanMemoryModel) return false;
            if (r.vulkanMemoryModelDeviceScope && !a.vulkanMemoryModelDeviceScope) return false;
            if (r.vulkanMemoryModelAvailabilityVisibilityChains && !a.vulkanMemoryModelAvailabilityVisibilityChains) return false;
            if (r.shaderOutputViewportIndex && !a.shaderOutputViewportIndex) return false;
            if (r.shaderOutputLayer && !a.shaderOutputLayer) return false;
            if (r.subgroupBroadcastDynamicId && !a.subgroupBroadcastDynamicId) return false;

            return true;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan13Features mixin (physical_device_13_features).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan13Features>
    {

        VkStructureType     sType                                               { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        void*               pNext                                               { nullptr };
        VkBool32            robustImageAccess                                   {         };
        VkBool32            inlineUniformBlock                                  {         };
        VkBool32            descriptorBindingInlineUniformBlockUpdateAfterBind  {         };
        VkBool32            pipelineCreationCacheControl                        {         };
        VkBool32            privateData                                         {         };
        VkBool32            shaderDemoteToHelperInvocation                      {         };
        VkBool32            shaderTerminateInvocation                           {         };
        VkBool32            subgroupSizeControl                                 {         };
        VkBool32            computeFullSubgroups                                {         };
        VkBool32            synchronization2                                    {         };
        VkBool32            textureCompressionASTC_HDR                          {         };
        VkBool32            shaderZeroInitializeWorkgroupMemory                 {         };
        VkBool32            dynamicRendering                                    {         };
        VkBool32            shaderIntegerDotProduct                             {         };
        VkBool32            maintenance4                                        {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        /// @brief Returns true if every feature enabled in `requested` is also enabled in
        ///        `available` -- i.e. `requested` is a subset of `available`. Used to check a
        ///        requested feature set against what a physical device actually supports.
        static inline bool32_t
        is_subset(const derived_t& r, const derived_t& a)
        {
            if (r.robustImageAccess && !a.robustImageAccess) return false;
            if (r.inlineUniformBlock && !a.inlineUniformBlock) return false;
            if (r.descriptorBindingInlineUniformBlockUpdateAfterBind && !a.descriptorBindingInlineUniformBlockUpdateAfterBind) return false;
            if (r.pipelineCreationCacheControl && !a.pipelineCreationCacheControl) return false;
            if (r.privateData && !a.privateData) return false;
            if (r.shaderDemoteToHelperInvocation && !a.shaderDemoteToHelperInvocation) return false;
            if (r.shaderTerminateInvocation && !a.shaderTerminateInvocation) return false;
            if (r.subgroupSizeControl && !a.subgroupSizeControl) return false;
            if (r.computeFullSubgroups && !a.computeFullSubgroups) return false;
            if (r.synchronization2 && !a.synchronization2) return false;
            if (r.textureCompressionASTC_HDR && !a.textureCompressionASTC_HDR) return false;
            if (r.shaderZeroInitializeWorkgroupMemory && !a.shaderZeroInitializeWorkgroupMemory) return false;
            if (r.dynamicRendering && !a.dynamicRendering) return false;
            if (r.shaderIntegerDotProduct && !a.shaderIntegerDotProduct) return false;
            if (r.maintenance4 && !a.maintenance4) return false;

            return true;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan14Features mixin (physical_device_14_features).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan14Features>
    {

        VkStructureType     sType                                   { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES };
        void*               pNext                                   { nullptr };
        VkBool32            globalPriorityQuery                     {         };
        VkBool32            shaderSubgroupRotate                    {         };
        VkBool32            shaderSubgroupRotateClustered           {         };
        VkBool32            shaderFloatControls2                    {         };
        VkBool32            shaderExpectAssume                      {         };
        VkBool32            rectangularLines                        {         };
        VkBool32            bresenhamLines                          {         };
        VkBool32            smoothLines                             {         };
        VkBool32            stippledRectangularLines                {         };
        VkBool32            stippledBresenhamLines                  {         };
        VkBool32            stippledSmoothLines                     {         };
        VkBool32            vertexAttributeInstanceRateDivisor      {         };
        VkBool32            vertexAttributeInstanceRateZeroDivisor  {         };
        VkBool32            indexTypeUint8                          {         };
        VkBool32            dynamicRenderingLocalRead               {         };
        VkBool32            maintenance5                            {         };
        VkBool32            maintenance6                            {         };
        VkBool32            pipelineProtectedAccess                 {         };
        VkBool32            pipelineRobustness                      {         };
        VkBool32            hostImageCopy                           {         };
        VkBool32            pushDescriptor                          {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        /// @brief Returns true if every feature enabled in `requested` is also enabled in
        ///        `available` -- i.e. `requested` is a subset of `available`. Used to check a
        ///        requested feature set against what a physical device actually supports.
        static inline bool32_t
        is_subset(const derived_t& r, const derived_t& a)
        {
            if (r.globalPriorityQuery && !a.globalPriorityQuery) return false;
            if (r.shaderSubgroupRotate && !a.shaderSubgroupRotate) return false;
            if (r.shaderSubgroupRotateClustered && !a.shaderSubgroupRotateClustered) return false;
            if (r.shaderFloatControls2 && !a.shaderFloatControls2) return false;
            if (r.shaderExpectAssume && !a.shaderExpectAssume) return false;
            if (r.rectangularLines && !a.rectangularLines) return false;
            if (r.bresenhamLines && !a.bresenhamLines) return false;
            if (r.smoothLines && !a.smoothLines) return false;
            if (r.stippledRectangularLines && !a.stippledRectangularLines) return false;
            if (r.stippledBresenhamLines && !a.stippledBresenhamLines) return false;
            if (r.stippledSmoothLines && !a.stippledSmoothLines) return false;
            if (r.vertexAttributeInstanceRateDivisor && !a.vertexAttributeInstanceRateDivisor) return false;
            if (r.vertexAttributeInstanceRateZeroDivisor && !a.vertexAttributeInstanceRateZeroDivisor) return false;
            if (r.indexTypeUint8 && !a.indexTypeUint8) return false;
            if (r.dynamicRenderingLocalRead && !a.dynamicRenderingLocalRead) return false;
            if (r.maintenance5 && !a.maintenance5) return false;
            if (r.maintenance6 && !a.maintenance6) return false;
            if (r.pipelineProtectedAccess && !a.pipelineProtectedAccess) return false;
            if (r.pipelineRobustness && !a.pipelineRobustness) return false;
            if (r.hostImageCopy && !a.hostImageCopy) return false;
            if (r.pushDescriptor && !a.pushDescriptor) return false;

            return true;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceLimits mixin (physical_device_limits).
    ///
    /// A large flat block of device limits read by name, so the members are left public for direct
    /// access rather than carrying a getter per field. Reused as the nested `limits` member of
    /// physical_device_properties below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceLimits>
    {

        uint32_t              maxImageDimension1D                               {         };
        uint32_t              maxImageDimension2D                               {         };
        uint32_t              maxImageDimension3D                               {         };
        uint32_t              maxImageDimensionCube                             {         };
        uint32_t              maxImageArrayLayers                               {         };
        uint32_t              maxTexelBufferElements                            {         };
        uint32_t              maxUniformBufferRange                             {         };
        uint32_t              maxStorageBufferRange                             {         };
        uint32_t              maxPushConstantsSize                              {         };
        uint32_t              maxMemoryAllocationCount                          {         };
        uint32_t              maxSamplerAllocationCount                         {         };
        VkDeviceSize          bufferImageGranularity                            {         };
        VkDeviceSize          sparseAddressSpaceSize                            {         };
        uint32_t              maxBoundDescriptorSets                            {         };
        uint32_t              maxPerStageDescriptorSamplers                     {         };
        uint32_t              maxPerStageDescriptorUniformBuffers               {         };
        uint32_t              maxPerStageDescriptorStorageBuffers               {         };
        uint32_t              maxPerStageDescriptorSampledImages                {         };
        uint32_t              maxPerStageDescriptorStorageImages                {         };
        uint32_t              maxPerStageDescriptorInputAttachments             {         };
        uint32_t              maxPerStageResources                              {         };
        uint32_t              maxDescriptorSetSamplers                          {         };
        uint32_t              maxDescriptorSetUniformBuffers                    {         };
        uint32_t              maxDescriptorSetUniformBuffersDynamic             {         };
        uint32_t              maxDescriptorSetStorageBuffers                    {         };
        uint32_t              maxDescriptorSetStorageBuffersDynamic             {         };
        uint32_t              maxDescriptorSetSampledImages                     {         };
        uint32_t              maxDescriptorSetStorageImages                     {         };
        uint32_t              maxDescriptorSetInputAttachments                  {         };
        uint32_t              maxVertexInputAttributes                          {         };
        uint32_t              maxVertexInputBindings                            {         };
        uint32_t              maxVertexInputAttributeOffset                     {         };
        uint32_t              maxVertexInputBindingStride                       {         };
        uint32_t              maxVertexOutputComponents                         {         };
        uint32_t              maxTessellationGenerationLevel                    {         };
        uint32_t              maxTessellationPatchSize                          {         };
        uint32_t              maxTessellationControlPerVertexInputComponents    {         };
        uint32_t              maxTessellationControlPerVertexOutputComponents   {         };
        uint32_t              maxTessellationControlPerPatchOutputComponents    {         };
        uint32_t              maxTessellationControlTotalOutputComponents       {         };
        uint32_t              maxTessellationEvaluationInputComponents          {         };
        uint32_t              maxTessellationEvaluationOutputComponents         {         };
        uint32_t              maxGeometryShaderInvocations                      {         };
        uint32_t              maxGeometryInputComponents                        {         };
        uint32_t              maxGeometryOutputComponents                       {         };
        uint32_t              maxGeometryOutputVertices                         {         };
        uint32_t              maxGeometryTotalOutputComponents                  {         };
        uint32_t              maxFragmentInputComponents                        {         };
        uint32_t              maxFragmentOutputAttachments                      {         };
        uint32_t              maxFragmentDualSrcAttachments                     {         };
        uint32_t              maxFragmentCombinedOutputResources                {         };
        uint32_t              maxComputeSharedMemorySize                        {         };
        uint32_t              maxComputeWorkGroupCount[3]                       {         };
        uint32_t              maxComputeWorkGroupInvocations                    {         };
        uint32_t              maxComputeWorkGroupSize[3]                        {         };
        uint32_t              subPixelPrecisionBits                             {         };
        uint32_t              subTexelPrecisionBits                             {         };
        uint32_t              mipmapPrecisionBits                               {         };
        uint32_t              maxDrawIndexedIndexValue                          {         };
        uint32_t              maxDrawIndirectCount                              {         };
        float                 maxSamplerLodBias                                 {         };
        float                 maxSamplerAnisotropy                              {         };
        uint32_t              maxViewports                                      {         };
        uint32_t              maxViewportDimensions[2]                          {         };
        float                 viewportBoundsRange[2]                            {         };
        uint32_t              viewportSubPixelBits                              {         };
        size_t                minMemoryMapAlignment                             {         };
        VkDeviceSize          minTexelBufferOffsetAlignment                     {         };
        VkDeviceSize          minUniformBufferOffsetAlignment                   {         };
        VkDeviceSize          minStorageBufferOffsetAlignment                   {         };
        int32_t               minTexelOffset                                    {         };
        uint32_t              maxTexelOffset                                    {         };
        int32_t               minTexelGatherOffset                              {         };
        uint32_t              maxTexelGatherOffset                              {         };
        float                 minInterpolationOffset                            {         };
        float                 maxInterpolationOffset                            {         };
        uint32_t              subPixelInterpolationOffsetBits                   {         };
        uint32_t              maxFramebufferWidth                               {         };
        uint32_t              maxFramebufferHeight                              {         };
        uint32_t              maxFramebufferLayers                              {         };
        VkSampleCountFlags    framebufferColorSampleCounts                      {         };
        VkSampleCountFlags    framebufferDepthSampleCounts                      {         };
        VkSampleCountFlags    framebufferStencilSampleCounts                    {         };
        VkSampleCountFlags    framebufferNoAttachmentsSampleCounts              {         };
        uint32_t              maxColorAttachments                               {         };
        VkSampleCountFlags    sampledImageColorSampleCounts                     {         };
        VkSampleCountFlags    sampledImageIntegerSampleCounts                   {         };
        VkSampleCountFlags    sampledImageDepthSampleCounts                     {         };
        VkSampleCountFlags    sampledImageStencilSampleCounts                   {         };
        VkSampleCountFlags    storageImageSampleCounts                          {         };
        uint32_t              maxSampleMaskWords                                {         };
        VkBool32              timestampComputeAndGraphics                       {         };
        float                 timestampPeriod                                   {         };
        uint32_t              maxClipDistances                                  {         };
        uint32_t              maxCullDistances                                  {         };
        uint32_t              maxCombinedClipAndCullDistances                   {         };
        uint32_t              discreteQueuePriorities                           {         };
        float                 pointSizeRange[2]                                 {         };
        float                 lineWidthRange[2]                                 {         };
        float                 pointSizeGranularity                              {         };
        float                 lineWidthGranularity                              {         };
        VkBool32              strictLines                                       {         };
        VkBool32              standardSampleLocations                           {         };
        VkDeviceSize          optimalBufferCopyOffsetAlignment                  {         };
        VkDeviceSize          optimalBufferCopyRowPitchAlignment                {         };
        VkDeviceSize          nonCoherentAtomSize                               {         };

    };

    /// @brief VkPhysicalDeviceSparseProperties mixin (physical_device_sparse_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceSparseProperties>
    {

        VkBool32    residencyStandard2DBlockShape               {         };
        VkBool32    residencyStandard2DMultisampleBlockShape    {         };
        VkBool32    residencyStandard3DBlockShape               {         };
        VkBool32    residencyAlignedMipSize                     {         };
        VkBool32    residencyNonResidentStrict                  {         };

        inline VkBool32 get_residency_standard_2d_block_shape() const               { return this->residencyStandard2DBlockShape;               }
        inline VkBool32 get_residency_standard_2d_multisample_block_shape() const   { return this->residencyStandard2DMultisampleBlockShape;    }
        inline VkBool32 get_residency_standard_3d_block_shape() const               { return this->residencyStandard3DBlockShape;               }
        inline VkBool32 get_residency_aligned_mip_size() const                      { return this->residencyAlignedMipSize;                     }
        inline VkBool32 get_residency_non_resident_strict() const                   { return this->residencyNonResidentStrict;                  }

    };

    /// @brief VkConformanceVersion mixin (conformance_version).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkConformanceVersion>
    {

        uint8_t major       {         };
        uint8_t minor       {         };
        uint8_t subminor    {         };
        uint8_t patch       {         };

        inline uint8_t get_major() const    { return this->major;       }
        inline uint8_t get_minor() const    { return this->minor;       }
        inline uint8_t get_subminor() const { return this->subminor;    }
        inline uint8_t get_patch() const    { return this->patch;       }

    };

    /// @brief VkPhysicalDeviceProperties mixin (physical_device_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceProperties>
    {

        uint32_t                            apiVersion                                      {         };
        uint32_t                            driverVersion                                   {         };
        uint32_t                            vendorID                                        {         };
        uint32_t                            deviceID                                        {         };
        VkPhysicalDeviceType                deviceType                                      {         };
        char                                            deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]    {         };
        uint8_t                                         pipelineCacheUUID[VK_UUID_SIZE]                 {         };
        vk_struct_base<VkPhysicalDeviceLimits>          limits                                          {         };
        vk_struct_base<VkPhysicalDeviceSparseProperties> sparseProperties                               {         };

        inline uint32_t                                                 get_api_version() const         { return this->apiVersion;          }
        inline uint32_t                                                 get_driver_version() const      { return this->driverVersion;       }
        inline uint32_t                                                 get_vendor_id() const           { return this->vendorID;            }
        inline uint32_t                                                 get_device_id() const           { return this->deviceID;            }
        inline VkPhysicalDeviceType                                     get_device_type() const         { return this->deviceType;          }
        inline const char*                                              get_device_name() const         { return this->deviceName;          }
        inline const vk_struct_base<VkPhysicalDeviceLimits>&            get_limits() const              { return this->limits;              }
        inline const vk_struct_base<VkPhysicalDeviceSparseProperties>&  get_sparse_properties() const   { return this->sparseProperties;    }

    };

    /// @brief VkPhysicalDeviceProperties2 mixin (physical_device_10_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceProperties2>
    {

        VkStructureType                             sType       { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        void*                                       pNext       { nullptr                                        };
        vk_struct_base<VkPhysicalDeviceProperties>  properties  {                                                };

        inline void*                                              get_next() const       { return this->pNext;       }
        inline const vk_struct_base<VkPhysicalDeviceProperties>&  get_properties() const { return this->properties;  }

        inline derived_t& set_next(void* next) { this->pNext = next; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan11Properties mixin (physical_device_11_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan11Properties>
    {

        VkStructureType             sType                               { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
        void*                       pNext                               { nullptr };
        uint8_t                     deviceUUID[VK_UUID_SIZE]            {         };
        uint8_t                     driverUUID[VK_UUID_SIZE]            {         };
        uint8_t                     deviceLUID[VK_LUID_SIZE]            {         };
        uint32_t                    deviceNodeMask                      {         };
        VkBool32                    deviceLUIDValid                     {         };
        uint32_t                    subgroupSize                        {         };
        VkShaderStageFlags          subgroupSupportedStages             {         };
        VkSubgroupFeatureFlags      subgroupSupportedOperations         {         };
        VkBool32                    subgroupQuadOperationsInAllStages   {         };
        VkPointClippingBehavior     pointClippingBehavior               {         };
        uint32_t                    maxMultiviewViewCount               {         };
        uint32_t                    maxMultiviewInstanceIndex           {         };
        VkBool32                    protectedNoFault                    {         };
        uint32_t                    maxPerSetDescriptors                {         };
        VkDeviceSize                maxMemoryAllocationSize             {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan12Properties mixin (physical_device_12_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan12Properties>
    {

        VkStructureType                     sType                                                   { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
        void*                               pNext                                                   { nullptr };
        VkDriverId                          driverID                                                {         };
        char                                driverName[VK_MAX_DRIVER_NAME_SIZE]                     {         };
        char                                driverInfo[VK_MAX_DRIVER_INFO_SIZE]                     {         };
        vk_struct_base<VkConformanceVersion> conformanceVersion                                     {         };
        VkShaderFloatControlsIndependence   denormBehaviorIndependence                              {         };
        VkShaderFloatControlsIndependence   roundingModeIndependence                                {         };
        VkBool32                            shaderSignedZeroInfNanPreserveFloat16                   {         };
        VkBool32                            shaderSignedZeroInfNanPreserveFloat32                   {         };
        VkBool32                            shaderSignedZeroInfNanPreserveFloat64                   {         };
        VkBool32                            shaderDenormPreserveFloat16                             {         };
        VkBool32                            shaderDenormPreserveFloat32                             {         };
        VkBool32                            shaderDenormPreserveFloat64                             {         };
        VkBool32                            shaderDenormFlushToZeroFloat16                          {         };
        VkBool32                            shaderDenormFlushToZeroFloat32                          {         };
        VkBool32                            shaderDenormFlushToZeroFloat64                          {         };
        VkBool32                            shaderRoundingModeRTEFloat16                            {         };
        VkBool32                            shaderRoundingModeRTEFloat32                            {         };
        VkBool32                            shaderRoundingModeRTEFloat64                            {         };
        VkBool32                            shaderRoundingModeRTZFloat16                            {         };
        VkBool32                            shaderRoundingModeRTZFloat32                            {         };
        VkBool32                            shaderRoundingModeRTZFloat64                            {         };
        uint32_t                            maxUpdateAfterBindDescriptorsInAllPools                 {         };
        VkBool32                            shaderUniformBufferArrayNonUniformIndexingNative        {         };
        VkBool32                            shaderSampledImageArrayNonUniformIndexingNative         {         };
        VkBool32                            shaderStorageBufferArrayNonUniformIndexingNative        {         };
        VkBool32                            shaderStorageImageArrayNonUniformIndexingNative         {         };
        VkBool32                            shaderInputAttachmentArrayNonUniformIndexingNative      {         };
        VkBool32                            robustBufferAccessUpdateAfterBind                       {         };
        VkBool32                            quadDivergentImplicitLod                                {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindSamplers            {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindUniformBuffers      {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindStorageBuffers      {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindSampledImages       {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindStorageImages       {         };
        uint32_t                            maxPerStageDescriptorUpdateAfterBindInputAttachments    {         };
        uint32_t                            maxPerStageUpdateAfterBindResources                     {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindSamplers                 {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindUniformBuffers           {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindUniformBuffersDynamic    {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindStorageBuffers           {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindStorageBuffersDynamic    {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindSampledImages            {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindStorageImages            {         };
        uint32_t                            maxDescriptorSetUpdateAfterBindInputAttachments         {         };
        VkResolveModeFlags                  supportedDepthResolveModes                              {         };
        VkResolveModeFlags                  supportedStencilResolveModes                            {         };
        VkBool32                            independentResolveNone                                  {         };
        VkBool32                            independentResolve                                      {         };
        VkBool32                            filterMinmaxSingleComponentFormats                      {         };
        VkBool32                            filterMinmaxImageComponentMapping                       {         };
        uint64_t                            maxTimelineSemaphoreValueDifference                     {         };
        VkSampleCountFlags                  framebufferIntegerColorSampleCounts                     {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan13Properties mixin (physical_device_13_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan13Properties>
    {

        VkStructureType     sType                                                                       { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES };
        void*               pNext                                                                       { nullptr };
        uint32_t            minSubgroupSize                                                             {         };
        uint32_t            maxSubgroupSize                                                             {         };
        uint32_t            maxComputeWorkgroupSubgroups                                                {         };
        VkShaderStageFlags  requiredSubgroupSizeStages                                                  {         };
        uint32_t            maxInlineUniformBlockSize                                                   {         };
        uint32_t            maxPerStageDescriptorInlineUniformBlocks                                    {         };
        uint32_t            maxPerStageDescriptorUpdateAfterBindInlineUniformBlocks                     {         };
        uint32_t            maxDescriptorSetInlineUniformBlocks                                         {         };
        uint32_t            maxDescriptorSetUpdateAfterBindInlineUniformBlocks                          {         };
        uint32_t            maxInlineUniformTotalSize                                                   {         };
        VkBool32            integerDotProduct8BitUnsignedAccelerated                                    {         };
        VkBool32            integerDotProduct8BitSignedAccelerated                                      {         };
        VkBool32            integerDotProduct8BitMixedSignednessAccelerated                             {         };
        VkBool32            integerDotProduct4x8BitPackedUnsignedAccelerated                            {         };
        VkBool32            integerDotProduct4x8BitPackedSignedAccelerated                              {         };
        VkBool32            integerDotProduct4x8BitPackedMixedSignednessAccelerated                     {         };
        VkBool32            integerDotProduct16BitUnsignedAccelerated                                   {         };
        VkBool32            integerDotProduct16BitSignedAccelerated                                     {         };
        VkBool32            integerDotProduct16BitMixedSignednessAccelerated                            {         };
        VkBool32            integerDotProduct32BitUnsignedAccelerated                                   {         };
        VkBool32            integerDotProduct32BitSignedAccelerated                                     {         };
        VkBool32            integerDotProduct32BitMixedSignednessAccelerated                            {         };
        VkBool32            integerDotProduct64BitUnsignedAccelerated                                   {         };
        VkBool32            integerDotProduct64BitSignedAccelerated                                     {         };
        VkBool32            integerDotProduct64BitMixedSignednessAccelerated                            {         };
        VkBool32            integerDotProductAccumulatingSaturating8BitUnsignedAccelerated              {         };
        VkBool32            integerDotProductAccumulatingSaturating8BitSignedAccelerated                {         };
        VkBool32            integerDotProductAccumulatingSaturating8BitMixedSignednessAccelerated       {         };
        VkBool32            integerDotProductAccumulatingSaturating4x8BitPackedUnsignedAccelerated      {         };
        VkBool32            integerDotProductAccumulatingSaturating4x8BitPackedSignedAccelerated        {         };
        VkBool32            integerDotProductAccumulatingSaturating4x8BitPackedMixedSignednessAccelerated {       };
        VkBool32            integerDotProductAccumulatingSaturating16BitUnsignedAccelerated             {         };
        VkBool32            integerDotProductAccumulatingSaturating16BitSignedAccelerated               {         };
        VkBool32            integerDotProductAccumulatingSaturating16BitMixedSignednessAccelerated      {         };
        VkBool32            integerDotProductAccumulatingSaturating32BitUnsignedAccelerated             {         };
        VkBool32            integerDotProductAccumulatingSaturating32BitSignedAccelerated               {         };
        VkBool32            integerDotProductAccumulatingSaturating32BitMixedSignednessAccelerated      {         };
        VkBool32            integerDotProductAccumulatingSaturating64BitUnsignedAccelerated             {         };
        VkBool32            integerDotProductAccumulatingSaturating64BitSignedAccelerated               {         };
        VkBool32            integerDotProductAccumulatingSaturating64BitMixedSignednessAccelerated      {         };
        VkDeviceSize        storageTexelBufferOffsetAlignmentBytes                                      {         };
        VkBool32            storageTexelBufferOffsetSingleTexelAlignment                                {         };
        VkDeviceSize        uniformTexelBufferOffsetAlignmentBytes                                      {         };
        VkBool32            uniformTexelBufferOffsetSingleTexelAlignment                                {         };
        VkDeviceSize        maxBufferSize                                                               {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPhysicalDeviceVulkan14Properties mixin (physical_device_14_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceVulkan14Properties>
    {

        VkStructureType                     sType                                                   { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES };
        void*                               pNext                                                   { nullptr };
        uint32_t                            lineSubPixelPrecisionBits                               {         };
        uint32_t                            maxVertexAttribDivisor                                  {         };
        VkBool32                            supportsNonZeroFirstInstance                            {         };
        uint32_t                            maxPushDescriptors                                      {         };
        VkBool32                            dynamicRenderingLocalReadDepthStencilAttachments        {         };
        VkBool32                            dynamicRenderingLocalReadMultisampledAttachments        {         };
        VkBool32                            earlyFragmentMultisampleCoverageAfterSampleCounting     {         };
        VkBool32                            earlyFragmentSampleMaskTestBeforeSampleCounting         {         };
        VkBool32                            depthStencilSwizzleOneSupport                           {         };
        VkBool32                            polygonModePointSize                                    {         };
        VkBool32                            nonStrictSinglePixelWideLinesUseParallelogram           {         };
        VkBool32                            nonStrictWideLinesUseParallelogram                      {         };
        VkBool32                            blockTexelViewCompatibleMultipleLayers                  {         };
        uint32_t                            maxCombinedImageSamplerDescriptorCount                  {         };
        VkBool32                            fragmentShadingRateClampCombinerInputs                  {         };
        VkPipelineRobustnessBufferBehavior  defaultRobustnessStorageBuffers                         {         };
        VkPipelineRobustnessBufferBehavior  defaultRobustnessUniformBuffers                         {         };
        VkPipelineRobustnessBufferBehavior  defaultRobustnessVertexInputs                           {         };
        VkPipelineRobustnessImageBehavior   defaultRobustnessImages                                 {         };
        uint32_t                            copySrcLayoutCount                                      {         };
        VkImageLayout*                      pCopySrcLayouts                                         { nullptr };
        uint32_t                            copyDstLayoutCount                                      {         };
        VkImageLayout*                      pCopyDstLayouts                                         { nullptr };
        uint8_t                             optimalTilingLayoutUUID[VK_UUID_SIZE]                   {         };
        VkBool32                            identicalMemoryTypeRequirements                         {         };

        inline void*      get_next() const     { return this->pNext;               }
        inline derived_t& set_next(void* next)  { this->pNext = next; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkExtent2D mixin (extent_2d_t).
    ///
    /// A 2D size used throughout the surface/swapchain structs; reused as the nested extent members
    /// of surface_capabilities_t below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkExtent2D>
    {

        uint32_t    width   {         };
        uint32_t    height  {         };

        inline uint32_t get_width() const   { return this->width;   }
        inline uint32_t get_height() const  { return this->height;  }

        inline derived_t& set_width(uint32_t w)  { this->width = w; return *s();  }
        inline derived_t& set_height(uint32_t h) { this->height = h; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSurfaceCapabilitiesKHR mixin (surface_capabilities_t).
    ///
    /// An output struct filled by vkGetPhysicalDeviceSurfaceCapabilitiesKHR; members are public for
    /// direct read, with the extent members wrapped as extent_2d_t for typed access.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSurfaceCapabilitiesKHR>
    {

        uint32_t                        minImageCount           {         };
        uint32_t                        maxImageCount           {         };
        vk_struct_base<VkExtent2D>      currentExtent           {         };
        vk_struct_base<VkExtent2D>      minImageExtent          {         };
        vk_struct_base<VkExtent2D>      maxImageExtent          {         };
        uint32_t                        maxImageArrayLayers     {         };
        VkSurfaceTransformFlagsKHR      supportedTransforms     {         };
        VkSurfaceTransformFlagBitsKHR   currentTransform        {         };
        VkCompositeAlphaFlagsKHR        supportedCompositeAlpha {         };
        VkImageUsageFlags               supportedUsageFlags     {         };

        inline uint32_t                             get_min_image_count() const         { return this->minImageCount;           }
        inline uint32_t                             get_max_image_count() const         { return this->maxImageCount;           }
        inline const vk_struct_base<VkExtent2D>&    get_current_extent() const          { return this->currentExtent;           }
        inline const vk_struct_base<VkExtent2D>&    get_min_image_extent() const        { return this->minImageExtent;          }
        inline const vk_struct_base<VkExtent2D>&    get_max_image_extent() const        { return this->maxImageExtent;          }
        inline uint32_t                             get_max_image_array_layers() const  { return this->maxImageArrayLayers;     }
        inline VkSurfaceTransformFlagsKHR           get_supported_transforms() const    { return this->supportedTransforms;     }
        inline VkSurfaceTransformFlagBitsKHR        get_current_transform() const       { return this->currentTransform;        }
        inline VkCompositeAlphaFlagsKHR             get_supported_composite_alpha() const { return this->supportedCompositeAlpha; }
        inline VkImageUsageFlags                    get_supported_usage_flags() const   { return this->supportedUsageFlags;     }

    };

    /// @brief VkSurfaceFormatKHR mixin (surface_format_t).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSurfaceFormatKHR>
    {

        VkFormat        format      {         };
        VkColorSpaceKHR colorSpace  {         };

        inline VkFormat         get_format() const      { return this->format;      }
        inline VkColorSpaceKHR  get_color_space() const { return this->colorSpace;  }

    };

    /// @brief VkExtent3D mixin (extent_3d_t).
    ///
    /// A 3D size; reused as the nested transfer-granularity member of queue_family_properties_t.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkExtent3D>
    {

        uint32_t    width   {         };
        uint32_t    height  {         };
        uint32_t    depth   {         };

        inline uint32_t get_width() const   { return this->width;   }
        inline uint32_t get_height() const  { return this->height;  }
        inline uint32_t get_depth() const   { return this->depth;   }

        inline derived_t& set_width(uint32_t w)  { this->width = w; return *s();  }
        inline derived_t& set_height(uint32_t h) { this->height = h; return *s(); }
        inline derived_t& set_depth(uint32_t d)  { this->depth = d; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkQueueFamilyProperties mixin (queue_family_properties_t).
    ///
    /// Output struct from vkGetPhysicalDeviceQueueFamilyProperties; members are public for direct
    /// read, with the transfer granularity wrapped as extent_3d_t. Reused as the nested member of
    /// queue_family_properties_2_t below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkQueueFamilyProperties>
    {

        VkQueueFlags                queueFlags                  {         };
        uint32_t                    queueCount                  {         };
        uint32_t                    timestampValidBits          {         };
        vk_struct_base<VkExtent3D>  minImageTransferGranularity {         };

        inline VkQueueFlags                         get_queue_flags() const             { return this->queueFlags;          }
        inline uint32_t                             get_queue_count() const             { return this->queueCount;          }
        inline uint32_t                             get_timestamp_valid_bits() const    { return this->timestampValidBits;  }
        inline const vk_struct_base<VkExtent3D>&    get_min_image_transfer_granularity() const { return this->minImageTransferGranularity; }

        /// @brief Per-capability checks against queueFlags. Each returns true if the family exposes
        ///        that capability. Note a graphics- or compute-capable family implicitly supports
        ///        transfer even when the TRANSFER bit is not explicitly set (per the spec); these
        ///        helpers report the raw flag and do not fold in that implication.
        inline bool32_t supports_graphics() const       { return (this->queueFlags & VK_QUEUE_GRAPHICS_BIT)       != 0; }
        inline bool32_t supports_compute() const         { return (this->queueFlags & VK_QUEUE_COMPUTE_BIT)        != 0; }
        inline bool32_t supports_transfer() const        { return (this->queueFlags & VK_QUEUE_TRANSFER_BIT)       != 0; }
        inline bool32_t supports_sparse_binding() const  { return (this->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0; }
        inline bool32_t supports_protected() const       { return (this->queueFlags & VK_QUEUE_PROTECTED_BIT)      != 0; }

        /// @brief Queries whether this queue family can present to the given surface.
        ///
        /// Presentation support is a property of the (physical device, queue family, surface) triple
        /// rather than of the family's flags, so it can't be read from queueFlags -- it requires a
        /// live call to vkGetPhysicalDeviceSurfaceSupportKHR. The family does not carry its own index,
        /// so the caller supplies it (and the owning device/surface). Raw native handles are used here
        /// because structures.hpp sits below the handle wrappers in the include graph.
        /// @param device             The physical device this family belongs to.
        /// @param queue_family_index The index of this family within that device.
        /// @param surface            The surface to test presentation against.
        /// @return True if the family supports presentation to the surface, false otherwise.
        inline bool32_t
        supports_presentation(VkPhysicalDevice device, uint32_t queue_family_index, VkSurfaceKHR surface) const
        {
            VkBool32 supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_family_index, surface, &supported);
            return supported == VK_TRUE;
        }

    };

    /// @brief VkQueueFamilyProperties2 mixin (queue_family_properties_2_t).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkQueueFamilyProperties2>
    {

        VkStructureType                          sType                  { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 };
        void*                                    pNext                  { nullptr };
        vk_struct_base<VkQueueFamilyProperties>  queueFamilyProperties  {         };

        inline void*                                          get_next() const                   { return this->pNext;                  }
        inline const vk_struct_base<VkQueueFamilyProperties>& get_queue_family_properties() const { return this->queueFamilyProperties;  }

        inline derived_t& set_next(void* next) { this->pNext = next; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkDeviceQueueCreateInfo mixin (device_queue_create_info_t).
    ///
    /// Input struct describing the queues to allocate when creating a logical device. set_queue_-
    /// priorities keeps queueCount in sync with the supplied priority list.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkDeviceQueueCreateInfo>
    {

        VkStructureType             sType               { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        const void*                 pNext               { nullptr };
        VkDeviceQueueCreateFlags    flags               {         };
        uint32_t                    queueFamilyIndex    {         };
        uint32_t                    queueCount          {         };
        const float*                pQueuePriorities    { nullptr };

        inline const void*              get_next() const                { return this->pNext;             }
        inline VkDeviceQueueCreateFlags get_flags() const               { return this->flags;             }
        inline uint32_t                 get_queue_family_index() const  { return this->queueFamilyIndex;  }
        inline uint32_t                 get_queue_count() const         { return this->queueCount;        }
        inline spx::array_view<float>   get_queue_priorities() const    { return { this->pQueuePriorities, this->queueCount }; }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();             }
        inline derived_t& set_flags(VkDeviceQueueCreateFlags flags) { this->flags = flags; return *s();            }
        inline derived_t& set_queue_family_index(uint32_t index)    { this->queueFamilyIndex = index; return *s(); }

        inline derived_t& set_queue_priorities(spx::array_view<float> priorities)
        {
            this->pQueuePriorities = priorities.data();
            this->queueCount       = static_cast<uint32_t>(priorities.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkDeviceCreateInfo mixin (device_create_info_t).
    ///
    /// Input struct describing the logical device to create: the queues to allocate (each a
    /// device_queue_create_info_t), the device extensions to enable, and optionally a base feature
    /// set. The enabledLayerCount/ppEnabledLayerNames pair is deprecated and ignored by modern
    /// loaders, so no setter is exposed for it. To request feature structs beyond the base set,
    /// chain a physical_device_10_features_t off pNext and leave pEnabledFeatures null.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkDeviceCreateInfo>
    {

        VkStructureType                                   sType                   { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        const void*                                       pNext                   { nullptr };
        VkDeviceCreateFlags                               flags                   {         };
        uint32_t                                          queueCreateInfoCount    {         };
        const vk_struct_base<VkDeviceQueueCreateInfo>*    pQueueCreateInfos       { nullptr };
        uint32_t                                          enabledLayerCount       {         };
        const char* const*                                ppEnabledLayerNames     { nullptr };
        uint32_t                                          enabledExtensionCount   {         };
        const char* const*                                ppEnabledExtensionNames { nullptr };
        const vk_struct_base<VkPhysicalDeviceFeatures>*   pEnabledFeatures        { nullptr };

        inline const void*          get_next() const            { return this->pNext;             }
        inline VkDeviceCreateFlags  get_flags() const           { return this->flags;             }
        inline spx::array_view<const char*> get_extensions() const { return { this->ppEnabledExtensionNames, this->enabledExtensionCount }; }
        inline const vk_struct_base<VkDeviceQueueCreateInfo>* get_queue_create_infos() const { return this->pQueueCreateInfos; }
        inline const vk_struct_base<VkPhysicalDeviceFeatures>* get_enabled_features() const  { return this->pEnabledFeatures;  }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkDeviceCreateFlags flags)      { this->flags = flags; return *s(); }

        inline derived_t& set_queue_create_infos(spx::array_view<const vk_struct_base<VkDeviceQueueCreateInfo>> infos)
        {
            this->pQueueCreateInfos    = infos.data();
            this->queueCreateInfoCount = static_cast<uint32_t>(infos.size());
            return *s();
        }

        inline derived_t& set_extensions(spx::array_view<const char*> extensions)
        {
            this->ppEnabledExtensionNames = extensions.data();
            this->enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            return *s();
        }

        inline derived_t& set_enabled_features(const vk_struct_base<VkPhysicalDeviceFeatures>* features)
        {
            this->pEnabledFeatures = features;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkComponentMapping mixin (component_mapping_t).
    ///
    /// The per-channel swizzle applied by an image view. Defaults to identity on every channel
    /// (VK_COMPONENT_SWIZZLE_IDENTITY is zero, so a value-initialized struct is already identity),
    /// which is what the vast majority of views want. Reused as the nested `components` member of
    /// image_view_create_info_t below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkComponentMapping>
    {

        VkComponentSwizzle  r   { VK_COMPONENT_SWIZZLE_IDENTITY };
        VkComponentSwizzle  g   { VK_COMPONENT_SWIZZLE_IDENTITY };
        VkComponentSwizzle  b   { VK_COMPONENT_SWIZZLE_IDENTITY };
        VkComponentSwizzle  a   { VK_COMPONENT_SWIZZLE_IDENTITY };

        inline VkComponentSwizzle get_r() const { return this->r; }
        inline VkComponentSwizzle get_g() const { return this->g; }
        inline VkComponentSwizzle get_b() const { return this->b; }
        inline VkComponentSwizzle get_a() const { return this->a; }

        inline derived_t& set_r(VkComponentSwizzle s) { this->r = s; return *self(); }
        inline derived_t& set_g(VkComponentSwizzle s) { this->g = s; return *self(); }
        inline derived_t& set_b(VkComponentSwizzle s) { this->b = s; return *self(); }
        inline derived_t& set_a(VkComponentSwizzle s) { this->a = s; return *self(); }

        /// @brief Sets all four channels back to identity.
        inline derived_t& set_identity()
        {
            this->r = this->g = this->b = this->a = VK_COMPONENT_SWIZZLE_IDENTITY;
            return *self();
        }

        private:
            inline derived_t* self() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkImageSubresourceRange mixin (image_subresource_range_t).
    ///
    /// Describes which mip levels / array layers (and which aspect) of an image a view covers.
    /// Reused as the nested `subresourceRange` member of image_view_create_info_t below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkImageSubresourceRange>
    {

        VkImageAspectFlags  aspectMask      {         };
        uint32_t            baseMipLevel    {         };
        uint32_t            levelCount      {         };
        uint32_t            baseArrayLayer  {         };
        uint32_t            layerCount      {         };

        inline VkImageAspectFlags   get_aspect_mask() const         { return this->aspectMask;      }
        inline uint32_t             get_base_mip_level() const      { return this->baseMipLevel;    }
        inline uint32_t             get_level_count() const         { return this->levelCount;      }
        inline uint32_t             get_base_array_layer() const    { return this->baseArrayLayer;  }
        inline uint32_t             get_layer_count() const         { return this->layerCount;      }

        inline derived_t& set_aspect_mask(VkImageAspectFlags mask)  { this->aspectMask = mask; return *s();     }
        inline derived_t& set_base_mip_level(uint32_t level)        { this->baseMipLevel = level; return *s();  }
        inline derived_t& set_level_count(uint32_t count)           { this->levelCount = count; return *s();    }
        inline derived_t& set_base_array_layer(uint32_t layer)      { this->baseArrayLayer = layer; return *s();}
        inline derived_t& set_layer_count(uint32_t count)           { this->layerCount = count; return *s();    }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkImageViewCreateInfo mixin (image_view_create_info_t).
    ///
    /// Input struct describing the view to create over an image. The image member is the raw
    /// VkImage handle (views are created over swapchain images, which the application does not own
    /// as wrappers). The color_2d factory builds the common case used for swapchain image views.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkImageViewCreateInfo>
    {

        VkStructureType                             sType               { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        const void*                                 pNext               { nullptr };
        VkImageViewCreateFlags                      flags               {         };
        VkImage                                     image               { VK_NULL_HANDLE };
        VkImageViewType                             viewType            {         };
        VkFormat                                    format              {         };
        vk_struct_base<VkComponentMapping>          components          {         };
        vk_struct_base<VkImageSubresourceRange>     subresourceRange    {         };

        inline const void*              get_next() const        { return this->pNext;       }
        inline VkImageViewCreateFlags   get_flags() const       { return this->flags;       }
        inline VkImage                  get_image() const       { return this->image;       }
        inline VkImageViewType          get_view_type() const   { return this->viewType;    }
        inline VkFormat                 get_format() const      { return this->format;      }
        inline const vk_struct_base<VkComponentMapping>&      get_components() const         { return this->components;       }
        inline const vk_struct_base<VkImageSubresourceRange>& get_subresource_range() const  { return this->subresourceRange; }

        inline derived_t& set_next(const void* next)            { this->pNext = next; return *s();      }
        inline derived_t& set_flags(VkImageViewCreateFlags f)   { this->flags = f; return *s();         }
        inline derived_t& set_image(VkImage img)                { this->image = img; return *s();       }
        inline derived_t& set_view_type(VkImageViewType type)   { this->viewType = type; return *s();   }
        inline derived_t& set_format(VkFormat fmt)              { this->format = fmt; return *s();      }

        inline derived_t& set_components(const vk_struct_base<VkComponentMapping>& components)
        {
            this->components = components;
            return *s();
        }

        inline derived_t& set_subresource_range(const vk_struct_base<VkImageSubresourceRange>& range)
        {
            this->subresourceRange = range;
            return *s();
        }

        /// @brief Builds a create-info for the common case: a single-mip, single-layer 2D color view
        ///        over the given image with identity swizzles. This is what swapchain image views use.
        /// @param image  The image (typically a swapchain image) to view.
        /// @param format The format to interpret the image with (the swapchain's format).
        /// @return A fully-populated create-info ready to hand to create_image_view.
        static inline derived_t
        color_2d(VkImage image, VkFormat format)
        {
            derived_t info { };
            info.image    = image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format   = format;
            info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel   = 0;
            info.subresourceRange.levelCount     = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount     = 1;
            return info;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSwapchainCreateInfoKHR mixin (swapchain_create_info_t).
    ///
    /// Input struct describing the swapchain to create. surface and oldSwapchain are raw handles
    /// (the wrappers sit above this layer). set_concurrent_queue_families / set_exclusive_queue_family
    /// keep the sharing mode and the queue-family index list in sync, mirroring how the queue-priority
    /// and extension setters elsewhere keep their counts in sync.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSwapchainCreateInfoKHR>
    {

        VkStructureType                 sType                   { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        const void*                     pNext                   { nullptr };
        VkSwapchainCreateFlagsKHR       flags                   {         };
        VkSurfaceKHR                    surface                 { VK_NULL_HANDLE };
        uint32_t                        minImageCount           {         };
        VkFormat                        imageFormat             {         };
        VkColorSpaceKHR                 imageColorSpace         {         };
        vk_struct_base<VkExtent2D>      imageExtent             {         };
        uint32_t                        imageArrayLayers        {         };
        VkImageUsageFlags               imageUsage              {         };
        VkSharingMode                   imageSharingMode        {         };
        uint32_t                        queueFamilyIndexCount   {         };
        const uint32_t*                 pQueueFamilyIndices     { nullptr };
        VkSurfaceTransformFlagBitsKHR   preTransform            {         };
        VkCompositeAlphaFlagBitsKHR     compositeAlpha          {         };
        VkPresentModeKHR                presentMode             {         };
        VkBool32                        clipped                 {         };
        VkSwapchainKHR                  oldSwapchain            { VK_NULL_HANDLE };

        inline const void*                  get_next() const                { return this->pNext;               }
        inline VkSwapchainCreateFlagsKHR    get_flags() const               { return this->flags;               }
        inline VkSurfaceKHR                 get_surface() const             { return this->surface;             }
        inline uint32_t                     get_min_image_count() const     { return this->minImageCount;       }
        inline VkFormat                     get_image_format() const        { return this->imageFormat;         }
        inline VkColorSpaceKHR              get_image_color_space() const   { return this->imageColorSpace;     }
        inline const vk_struct_base<VkExtent2D>& get_image_extent() const   { return this->imageExtent;         }
        inline uint32_t                     get_image_array_layers() const  { return this->imageArrayLayers;    }
        inline VkImageUsageFlags            get_image_usage() const         { return this->imageUsage;          }
        inline VkSharingMode                get_image_sharing_mode() const  { return this->imageSharingMode;    }
        inline VkSurfaceTransformFlagBitsKHR get_pre_transform() const      { return this->preTransform;        }
        inline VkCompositeAlphaFlagBitsKHR  get_composite_alpha() const     { return this->compositeAlpha;      }
        inline VkPresentModeKHR             get_present_mode() const        { return this->presentMode;         }
        inline VkBool32                     get_clipped() const             { return this->clipped;             }
        inline VkSwapchainKHR               get_old_swapchain() const       { return this->oldSwapchain;        }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();              }
        inline derived_t& set_flags(VkSwapchainCreateFlagsKHR f)            { this->flags = f; return *s();                 }
        inline derived_t& set_surface(VkSurfaceKHR s)                       { this->surface = s; return *self();            }
        inline derived_t& set_min_image_count(uint32_t count)              { this->minImageCount = count; return *s();     }
        inline derived_t& set_image_format(VkFormat fmt)                    { this->imageFormat = fmt; return *s();         }
        inline derived_t& set_image_color_space(VkColorSpaceKHR space)      { this->imageColorSpace = space; return *s();   }
        inline derived_t& set_image_extent(const vk_struct_base<VkExtent2D>& extent) { this->imageExtent = extent; return *s(); }
        inline derived_t& set_image_array_layers(uint32_t layers)          { this->imageArrayLayers = layers; return *s(); }
        inline derived_t& set_image_usage(VkImageUsageFlags usage)          { this->imageUsage = usage; return *s();        }
        inline derived_t& set_pre_transform(VkSurfaceTransformFlagBitsKHR t){ this->preTransform = t; return *s();          }
        inline derived_t& set_composite_alpha(VkCompositeAlphaFlagBitsKHR a){ this->compositeAlpha = a; return *s();        }
        inline derived_t& set_present_mode(VkPresentModeKHR mode)           { this->presentMode = mode; return *s();        }
        inline derived_t& set_clipped(VkBool32 clipped)                     { this->clipped = clipped; return *s();         }
        inline derived_t& set_old_swapchain(VkSwapchainKHR old)             { this->oldSwapchain = old; return *s();        }

        /// @brief Use concurrent sharing across the given queue families. Required when the graphics
        ///        and presentation queues come from different families so images can be used by both
        ///        without explicit ownership transfers. The supplied index list must outlive the
        ///        create call (it is referenced, not copied).
        inline derived_t& set_concurrent_queue_families(spx::array_view<uint32_t> indices)
        {
            this->imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            this->queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
            this->pQueueFamilyIndices   = indices.data();
            return *s();
        }

        /// @brief Use exclusive sharing (the common single-family case). Clears the index list.
        inline derived_t& set_exclusive_queue_family()
        {
            this->imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            this->queueFamilyIndexCount = 0;
            this->pQueueFamilyIndices   = nullptr;
            return *s();
        }

        private:
            // surface members would shadow a private s() named member elsewhere; keep two spellings so
            // the surface setter (which takes a parameter named s) doesn't collide with the helper.
            inline derived_t* s()    { return reinterpret_cast<derived_t*>(this); }
            inline derived_t* self() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkShaderModuleCreateInfo mixin (shader_module_create_info_t).
    ///
    /// Input struct wrapping a SPIR-V blob for vkCreateShaderModule. codeSize is measured in bytes
    /// (and must be a multiple of 4) while pCode is a uint32_t word pointer -- set_code / from_spirv
    /// bridge that split from a raw byte buffer. The SPIR-V storage is referenced, not copied, so it
    /// must outlive the create call; the driver copies what it needs during vkCreateShaderModule, so
    /// the source buffer can be freed once that returns.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkShaderModuleCreateInfo>
    {

        VkStructureType             sType       { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        const void*                 pNext       { nullptr };
        VkShaderModuleCreateFlags   flags       {         };
        size_t                      codeSize    {         };
        const uint32_t*             pCode       { nullptr };

        inline const void*               get_next() const       { return this->pNext;       }
        inline VkShaderModuleCreateFlags get_flags() const      { return this->flags;       }
        inline size_t                    get_code_size() const   { return this->codeSize;    }
        inline const uint32_t*           get_code() const        { return this->pCode;       }

        inline derived_t& set_next(const void* next)             { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkShaderModuleCreateFlags f) { this->flags = f; return *s();     }

        /// @brief Points the create-info at a SPIR-V blob. The blob is referenced, not copied.
        /// @param code       Pointer to the SPIR-V words. Must be 4-byte aligned -- the engine's
        ///                   allocator over-aligns, so any simplex_memory_alloc'd buffer qualifies.
        /// @param size_bytes Size of the blob in bytes; must be a multiple of 4.
        inline derived_t& set_code(const void* code, size_t size_bytes)
        {
            this->codeSize = size_bytes;
            this->pCode    = reinterpret_cast<const uint32_t*>(code);
            return *s();
        }

        /// @brief Builds a create-info pointing at the given SPIR-V blob. See set_code for the
        ///        alignment/size contract; the blob must outlive the create call.
        /// @param code       Pointer to the SPIR-V words.
        /// @param size_bytes Size of the blob in bytes (a multiple of 4).
        /// @return A create-info ready to hand to create_shader_module.
        static inline derived_t
        from_spirv(const void* code, size_t size_bytes)
        {
            derived_t info { };
            info.codeSize = size_bytes;
            info.pCode    = reinterpret_cast<const uint32_t*>(code);
            return info;
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

    using application_info_t                    = vk_struct_base<VkApplicationInfo>;
    using instance_create_info_t                = vk_struct_base<VkInstanceCreateInfo>;
    using debug_utils_messenger_create_info_t   = vk_struct_base<VkDebugUtilsMessengerCreateInfoEXT>;

    using physical_device_features_t            = vk_struct_base<VkPhysicalDeviceFeatures>;
    using physical_device_10_features_t         = vk_struct_base<VkPhysicalDeviceFeatures2>;
    using physical_device_11_features_t         = vk_struct_base<VkPhysicalDeviceVulkan11Features>;
    using physical_device_12_features_t         = vk_struct_base<VkPhysicalDeviceVulkan12Features>;
    using physical_device_13_features_t         = vk_struct_base<VkPhysicalDeviceVulkan13Features>;
    using physical_device_14_features_t         = vk_struct_base<VkPhysicalDeviceVulkan14Features>;

    using physical_device_limits_t              = vk_struct_base<VkPhysicalDeviceLimits>;
    using physical_device_sparse_properties_t   = vk_struct_base<VkPhysicalDeviceSparseProperties>;
    using conformance_version_t                 = vk_struct_base<VkConformanceVersion>;

    using physical_device_properties_t          = vk_struct_base<VkPhysicalDeviceProperties>;
    using physical_device_10_properties_t       = vk_struct_base<VkPhysicalDeviceProperties2>;
    using physical_device_11_properties_t       = vk_struct_base<VkPhysicalDeviceVulkan11Properties>;
    using physical_device_12_properties_t       = vk_struct_base<VkPhysicalDeviceVulkan12Properties>;
    using physical_device_13_properties_t       = vk_struct_base<VkPhysicalDeviceVulkan13Properties>;
    using physical_device_14_properties_t       = vk_struct_base<VkPhysicalDeviceVulkan14Properties>;

    using extent_2d_t                           = vk_struct_base<VkExtent2D>;
    using extent_3d_t                           = vk_struct_base<VkExtent3D>;
    using surface_capabilities_t                = vk_struct_base<VkSurfaceCapabilitiesKHR>;
    using surface_format_t                      = vk_struct_base<VkSurfaceFormatKHR>;

    using queue_family_properties_t             = vk_struct_base<VkQueueFamilyProperties>;
    using queue_family_properties_2_t           = vk_struct_base<VkQueueFamilyProperties2>;
    using device_queue_create_info_t            = vk_struct_base<VkDeviceQueueCreateInfo>;
    using device_create_info_t                  = vk_struct_base<VkDeviceCreateInfo>;

    using component_mapping_t                   = vk_struct_base<VkComponentMapping>;
    using image_subresource_range_t             = vk_struct_base<VkImageSubresourceRange>;
    using image_view_create_info_t              = vk_struct_base<VkImageViewCreateInfo>;
    using swapchain_create_info_t               = vk_struct_base<VkSwapchainCreateInfoKHR>;
    using shader_module_create_info_t           = vk_struct_base<VkShaderModuleCreateInfo>;

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
    static_assert(std::is_standard_layout_v<application_info_t>, "application_info must be standard-layout for native interop.");
    static_assert(sizeof(application_info_t) == sizeof(VkApplicationInfo), "application_info layout diverged from VkApplicationInfo.");
    static_assert(offsetof(application_info_t, sType) == offsetof(VkApplicationInfo, sType));
    static_assert(offsetof(application_info_t, pNext) == offsetof(VkApplicationInfo, pNext));
    static_assert(offsetof(application_info_t, pApplicationName) == offsetof(VkApplicationInfo, pApplicationName));
    static_assert(offsetof(application_info_t, applicationVersion) == offsetof(VkApplicationInfo, applicationVersion));
    static_assert(offsetof(application_info_t, pEngineName) == offsetof(VkApplicationInfo, pEngineName));
    static_assert(offsetof(application_info_t, engineVersion) == offsetof(VkApplicationInfo, engineVersion));
    static_assert(offsetof(application_info_t, apiVersion) == offsetof(VkApplicationInfo, apiVersion));

    // VkInstanceCreateInfo checks.
    static_assert(std::is_standard_layout_v<instance_create_info_t>, "instance_create_info must be standard-layout for native interop.");
    static_assert(sizeof(instance_create_info_t) == sizeof(VkInstanceCreateInfo), "instance_create_info layout diverged from VkInstanceCreateInfo.");
    static_assert(offsetof(instance_create_info_t, sType) == offsetof(VkInstanceCreateInfo, sType));
    static_assert(offsetof(instance_create_info_t, pNext) == offsetof(VkInstanceCreateInfo, pNext));
    static_assert(offsetof(instance_create_info_t, flags) == offsetof(VkInstanceCreateInfo, flags));
    static_assert(offsetof(instance_create_info_t, pApplicationInfo) == offsetof(VkInstanceCreateInfo, pApplicationInfo));
    static_assert(offsetof(instance_create_info_t, enabledLayerCount) == offsetof(VkInstanceCreateInfo, enabledLayerCount));
    static_assert(offsetof(instance_create_info_t, ppEnabledLayerNames) == offsetof(VkInstanceCreateInfo, ppEnabledLayerNames));
    static_assert(offsetof(instance_create_info_t, enabledExtensionCount) == offsetof(VkInstanceCreateInfo, enabledExtensionCount));
    static_assert(offsetof(instance_create_info_t, ppEnabledExtensionNames) == offsetof(VkInstanceCreateInfo, ppEnabledExtensionNames));

    // VkDebugUtilsMessengerCreateInfoEXT checks.
    static_assert(std::is_standard_layout_v<debug_utils_messenger_create_info_t>, "debug_utils_messenger_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(debug_utils_messenger_create_info_t) == sizeof(VkDebugUtilsMessengerCreateInfoEXT), "debug_utils_messenger_create_info_t layout diverged from VkDebugUtilsMessengerCreateInfoEXT.");
    static_assert(offsetof(debug_utils_messenger_create_info_t, sType) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, sType));
    static_assert(offsetof(debug_utils_messenger_create_info_t, pNext) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, pNext));
    static_assert(offsetof(debug_utils_messenger_create_info_t, flags) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, flags));
    static_assert(offsetof(debug_utils_messenger_create_info_t, messageSeverity) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, messageSeverity));
    static_assert(offsetof(debug_utils_messenger_create_info_t, messageType) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, messageType));
    static_assert(offsetof(debug_utils_messenger_create_info_t, pfnUserCallback) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, pfnUserCallback));
    static_assert(offsetof(debug_utils_messenger_create_info_t, pUserData) == offsetof(VkDebugUtilsMessengerCreateInfoEXT, pUserData));

    // Physical device feature/property checks. The version-specific structs are large, so beyond
    // sType/pNext these probe the first and last data members: any reorder, retype, or SDK bump
    // shifts sizeof or the trailing offset and trips a guard.

    // VkPhysicalDeviceFeatures checks.
    static_assert(std::is_standard_layout_v<physical_device_features_t>, "physical_device_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_features_t) == sizeof(VkPhysicalDeviceFeatures), "physical_device_features layout diverged from VkPhysicalDeviceFeatures.");
    static_assert(offsetof(physical_device_features_t, robustBufferAccess) == offsetof(VkPhysicalDeviceFeatures, robustBufferAccess));
    static_assert(offsetof(physical_device_features_t, inheritedQueries) == offsetof(VkPhysicalDeviceFeatures, inheritedQueries));

    // VkPhysicalDevicefeatures_t2 checks.
    static_assert(std::is_standard_layout_v<physical_device_10_features_t>, "physical_device_10_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_10_features_t) == sizeof(VkPhysicalDeviceFeatures2), "physical_device_10_features layout diverged from VkPhysicalDeviceFeatures2.");
    static_assert(offsetof(physical_device_10_features_t, sType) == offsetof(VkPhysicalDeviceFeatures2, sType));
    static_assert(offsetof(physical_device_10_features_t, pNext) == offsetof(VkPhysicalDeviceFeatures2, pNext));
    static_assert(offsetof(physical_device_10_features_t, features) == offsetof(VkPhysicalDeviceFeatures2, features));

    // VkPhysicalDeviceVulkan11features_t checks.
    static_assert(std::is_standard_layout_v<physical_device_11_features_t>, "physical_device_11_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_11_features_t) == sizeof(VkPhysicalDeviceVulkan11Features), "physical_device_11_features layout diverged from VkPhysicalDeviceVulkan11Features.");
    static_assert(offsetof(physical_device_11_features_t, sType) == offsetof(VkPhysicalDeviceVulkan11Features, sType));
    static_assert(offsetof(physical_device_11_features_t, pNext) == offsetof(VkPhysicalDeviceVulkan11Features, pNext));
    static_assert(offsetof(physical_device_11_features_t, storageBuffer16BitAccess) == offsetof(VkPhysicalDeviceVulkan11Features, storageBuffer16BitAccess));
    static_assert(offsetof(physical_device_11_features_t, shaderDrawParameters) == offsetof(VkPhysicalDeviceVulkan11Features, shaderDrawParameters));

    // VkPhysicalDeviceVulkan12features_t checks.
    static_assert(std::is_standard_layout_v<physical_device_12_features_t>, "physical_device_12_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_12_features_t) == sizeof(VkPhysicalDeviceVulkan12Features), "physical_device_12_features layout diverged from VkPhysicalDeviceVulkan12Features.");
    static_assert(offsetof(physical_device_12_features_t, sType) == offsetof(VkPhysicalDeviceVulkan12Features, sType));
    static_assert(offsetof(physical_device_12_features_t, pNext) == offsetof(VkPhysicalDeviceVulkan12Features, pNext));
    static_assert(offsetof(physical_device_12_features_t, samplerMirrorClampToEdge) == offsetof(VkPhysicalDeviceVulkan12Features, samplerMirrorClampToEdge));
    static_assert(offsetof(physical_device_12_features_t, subgroupBroadcastDynamicId) == offsetof(VkPhysicalDeviceVulkan12Features, subgroupBroadcastDynamicId));

    // VkPhysicalDeviceVulkan13features_t checks.
    static_assert(std::is_standard_layout_v<physical_device_13_features_t>, "physical_device_13_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_13_features_t) == sizeof(VkPhysicalDeviceVulkan13Features), "physical_device_13_features layout diverged from VkPhysicalDeviceVulkan13Features.");
    static_assert(offsetof(physical_device_13_features_t, sType) == offsetof(VkPhysicalDeviceVulkan13Features, sType));
    static_assert(offsetof(physical_device_13_features_t, pNext) == offsetof(VkPhysicalDeviceVulkan13Features, pNext));
    static_assert(offsetof(physical_device_13_features_t, robustImageAccess) == offsetof(VkPhysicalDeviceVulkan13Features, robustImageAccess));
    static_assert(offsetof(physical_device_13_features_t, maintenance4) == offsetof(VkPhysicalDeviceVulkan13Features, maintenance4));

    // VkPhysicalDeviceVulkan14features_t checks.
    static_assert(std::is_standard_layout_v<physical_device_14_features_t>, "physical_device_14_features must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_14_features_t) == sizeof(VkPhysicalDeviceVulkan14Features), "physical_device_14_features layout diverged from VkPhysicalDeviceVulkan14Features.");
    static_assert(offsetof(physical_device_14_features_t, sType) == offsetof(VkPhysicalDeviceVulkan14Features, sType));
    static_assert(offsetof(physical_device_14_features_t, pNext) == offsetof(VkPhysicalDeviceVulkan14Features, pNext));
    static_assert(offsetof(physical_device_14_features_t, globalPriorityQuery) == offsetof(VkPhysicalDeviceVulkan14Features, globalPriorityQuery));
    static_assert(offsetof(physical_device_14_features_t, pushDescriptor) == offsetof(VkPhysicalDeviceVulkan14Features, pushDescriptor));

    // VkPhysicalDeviceLimits checks.
    static_assert(std::is_standard_layout_v<physical_device_limits_t>, "physical_device_limits must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_limits_t) == sizeof(VkPhysicalDeviceLimits), "physical_device_limits layout diverged from VkPhysicalDeviceLimits.");
    static_assert(offsetof(physical_device_limits_t, maxImageDimension1D) == offsetof(VkPhysicalDeviceLimits, maxImageDimension1D));
    static_assert(offsetof(physical_device_limits_t, minMemoryMapAlignment) == offsetof(VkPhysicalDeviceLimits, minMemoryMapAlignment));
    static_assert(offsetof(physical_device_limits_t, nonCoherentAtomSize) == offsetof(VkPhysicalDeviceLimits, nonCoherentAtomSize));

    // VkPhysicalDeviceSparseProperties checks.
    static_assert(std::is_standard_layout_v<physical_device_sparse_properties_t>, "physical_device_sparse_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_sparse_properties_t) == sizeof(VkPhysicalDeviceSparseProperties), "physical_device_sparse_properties layout diverged from VkPhysicalDeviceSparseProperties.");
    static_assert(offsetof(physical_device_sparse_properties_t, residencyStandard2DBlockShape) == offsetof(VkPhysicalDeviceSparseProperties, residencyStandard2DBlockShape));
    static_assert(offsetof(physical_device_sparse_properties_t, residencyNonResidentStrict) == offsetof(VkPhysicalDeviceSparseProperties, residencyNonResidentStrict));

    // VkConformanceVersion checks.
    static_assert(std::is_standard_layout_v<conformance_version_t>, "conformance_version must be standard-layout for native interop.");
    static_assert(sizeof(conformance_version_t) == sizeof(VkConformanceVersion), "conformance_version layout diverged from VkConformanceVersion.");
    static_assert(offsetof(conformance_version_t, major) == offsetof(VkConformanceVersion, major));
    static_assert(offsetof(conformance_version_t, patch) == offsetof(VkConformanceVersion, patch));

    // VkPhysicalDeviceproperties_t checks.
    static_assert(std::is_standard_layout_v<physical_device_properties_t>, "physical_device_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_properties_t) == sizeof(VkPhysicalDeviceProperties), "physical_device_properties layout diverged from VkPhysicalDeviceProperties.");
    static_assert(offsetof(physical_device_properties_t, apiVersion) == offsetof(VkPhysicalDeviceProperties, apiVersion));
    static_assert(offsetof(physical_device_properties_t, deviceType) == offsetof(VkPhysicalDeviceProperties, deviceType));
    static_assert(offsetof(physical_device_properties_t, deviceName) == offsetof(VkPhysicalDeviceProperties, deviceName));
    static_assert(offsetof(physical_device_properties_t, pipelineCacheUUID) == offsetof(VkPhysicalDeviceProperties, pipelineCacheUUID));
    static_assert(offsetof(physical_device_properties_t, limits) == offsetof(VkPhysicalDeviceProperties, limits));
    static_assert(offsetof(physical_device_properties_t, sparseProperties) == offsetof(VkPhysicalDeviceProperties, sparseProperties));

    // VkPhysicalDeviceproperties_t2 checks.
    static_assert(std::is_standard_layout_v<physical_device_10_properties_t>, "physical_device_10_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_10_properties_t) == sizeof(VkPhysicalDeviceProperties2), "physical_device_10_properties layout diverged from VkPhysicalDeviceProperties2.");
    static_assert(offsetof(physical_device_10_properties_t, sType) == offsetof(VkPhysicalDeviceProperties2, sType));
    static_assert(offsetof(physical_device_10_properties_t, pNext) == offsetof(VkPhysicalDeviceProperties2, pNext));
    static_assert(offsetof(physical_device_10_properties_t, properties) == offsetof(VkPhysicalDeviceProperties2, properties));

    // VkPhysicalDeviceVulkan11properties_t checks.
    static_assert(std::is_standard_layout_v<physical_device_11_properties_t>, "physical_device_11_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_11_properties_t) == sizeof(VkPhysicalDeviceVulkan11Properties), "physical_device_11_properties layout diverged from VkPhysicalDeviceVulkan11Properties.");
    static_assert(offsetof(physical_device_11_properties_t, sType) == offsetof(VkPhysicalDeviceVulkan11Properties, sType));
    static_assert(offsetof(physical_device_11_properties_t, pNext) == offsetof(VkPhysicalDeviceVulkan11Properties, pNext));
    static_assert(offsetof(physical_device_11_properties_t, deviceUUID) == offsetof(VkPhysicalDeviceVulkan11Properties, deviceUUID));
    static_assert(offsetof(physical_device_11_properties_t, maxMemoryAllocationSize) == offsetof(VkPhysicalDeviceVulkan11Properties, maxMemoryAllocationSize));

    // VkPhysicalDeviceVulkan12properties_t checks.
    static_assert(std::is_standard_layout_v<physical_device_12_properties_t>, "physical_device_12_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_12_properties_t) == sizeof(VkPhysicalDeviceVulkan12Properties), "physical_device_12_properties layout diverged from VkPhysicalDeviceVulkan12Properties.");
    static_assert(offsetof(physical_device_12_properties_t, sType) == offsetof(VkPhysicalDeviceVulkan12Properties, sType));
    static_assert(offsetof(physical_device_12_properties_t, pNext) == offsetof(VkPhysicalDeviceVulkan12Properties, pNext));
    static_assert(offsetof(physical_device_12_properties_t, driverID) == offsetof(VkPhysicalDeviceVulkan12Properties, driverID));
    static_assert(offsetof(physical_device_12_properties_t, framebufferIntegerColorSampleCounts) == offsetof(VkPhysicalDeviceVulkan12Properties, framebufferIntegerColorSampleCounts));

    // VkPhysicalDeviceVulkan13properties_t checks.
    static_assert(std::is_standard_layout_v<physical_device_13_properties_t>, "physical_device_13_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_13_properties_t) == sizeof(VkPhysicalDeviceVulkan13Properties), "physical_device_13_properties layout diverged from VkPhysicalDeviceVulkan13Properties.");
    static_assert(offsetof(physical_device_13_properties_t, sType) == offsetof(VkPhysicalDeviceVulkan13Properties, sType));
    static_assert(offsetof(physical_device_13_properties_t, pNext) == offsetof(VkPhysicalDeviceVulkan13Properties, pNext));
    static_assert(offsetof(physical_device_13_properties_t, minSubgroupSize) == offsetof(VkPhysicalDeviceVulkan13Properties, minSubgroupSize));
    static_assert(offsetof(physical_device_13_properties_t, maxBufferSize) == offsetof(VkPhysicalDeviceVulkan13Properties, maxBufferSize));

    // VkPhysicalDeviceVulkan14properties_t checks.
    static_assert(std::is_standard_layout_v<physical_device_14_properties_t>, "physical_device_14_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_14_properties_t) == sizeof(VkPhysicalDeviceVulkan14Properties), "physical_device_14_properties layout diverged from VkPhysicalDeviceVulkan14Properties.");
    static_assert(offsetof(physical_device_14_properties_t, sType) == offsetof(VkPhysicalDeviceVulkan14Properties, sType));
    static_assert(offsetof(physical_device_14_properties_t, pNext) == offsetof(VkPhysicalDeviceVulkan14Properties, pNext));
    static_assert(offsetof(physical_device_14_properties_t, lineSubPixelPrecisionBits) == offsetof(VkPhysicalDeviceVulkan14Properties, lineSubPixelPrecisionBits));
    static_assert(offsetof(physical_device_14_properties_t, identicalMemoryTypeRequirements) == offsetof(VkPhysicalDeviceVulkan14Properties, identicalMemoryTypeRequirements));

    // VkExtent2D checks.
    static_assert(std::is_standard_layout_v<extent_2d_t>, "extent_2d_t must be standard-layout for native interop.");
    static_assert(sizeof(extent_2d_t) == sizeof(VkExtent2D), "extent_2d_t layout diverged from VkExtent2D.");
    static_assert(offsetof(extent_2d_t, width) == offsetof(VkExtent2D, width));
    static_assert(offsetof(extent_2d_t, height) == offsetof(VkExtent2D, height));

    // VkSurfaceCapabilitiesKHR checks.
    static_assert(std::is_standard_layout_v<surface_capabilities_t>, "surface_capabilities_t must be standard-layout for native interop.");
    static_assert(sizeof(surface_capabilities_t) == sizeof(VkSurfaceCapabilitiesKHR), "surface_capabilities_t layout diverged from VkSurfaceCapabilitiesKHR.");
    static_assert(offsetof(surface_capabilities_t, minImageCount) == offsetof(VkSurfaceCapabilitiesKHR, minImageCount));
    static_assert(offsetof(surface_capabilities_t, currentExtent) == offsetof(VkSurfaceCapabilitiesKHR, currentExtent));
    static_assert(offsetof(surface_capabilities_t, maxImageExtent) == offsetof(VkSurfaceCapabilitiesKHR, maxImageExtent));
    static_assert(offsetof(surface_capabilities_t, supportedUsageFlags) == offsetof(VkSurfaceCapabilitiesKHR, supportedUsageFlags));

    // VkSurfaceFormatKHR checks. The array-writing wrapper for vkGetPhysicalDeviceSurfaceFormatsKHR
    // (functions.hpp) reinterprets a buffer of these as VkSurfaceFormatKHR, so this must be exact.
    static_assert(std::is_standard_layout_v<surface_format_t>, "surface_format_t must be standard-layout for native interop.");
    static_assert(sizeof(surface_format_t) == sizeof(VkSurfaceFormatKHR), "surface_format_t layout diverged from VkSurfaceFormatKHR.");
    static_assert(offsetof(surface_format_t, format) == offsetof(VkSurfaceFormatKHR, format));
    static_assert(offsetof(surface_format_t, colorSpace) == offsetof(VkSurfaceFormatKHR, colorSpace));

    // VkExtent3D checks.
    static_assert(std::is_standard_layout_v<extent_3d_t>, "extent_3d_t must be standard-layout for native interop.");
    static_assert(sizeof(extent_3d_t) == sizeof(VkExtent3D), "extent_3d_t layout diverged from VkExtent3D.");
    static_assert(offsetof(extent_3d_t, width) == offsetof(VkExtent3D, width));
    static_assert(offsetof(extent_3d_t, depth) == offsetof(VkExtent3D, depth));

    // VkQueueFamilyProperties checks. The array-writing wrapper for
    // vkGetPhysicalDeviceQueueFamilyProperties reinterprets a buffer of these, so this must be exact.
    static_assert(std::is_standard_layout_v<queue_family_properties_t>, "queue_family_properties_t must be standard-layout for native interop.");
    static_assert(sizeof(queue_family_properties_t) == sizeof(VkQueueFamilyProperties), "queue_family_properties_t layout diverged from VkQueueFamilyProperties.");
    static_assert(offsetof(queue_family_properties_t, queueFlags) == offsetof(VkQueueFamilyProperties, queueFlags));
    static_assert(offsetof(queue_family_properties_t, minImageTransferGranularity) == offsetof(VkQueueFamilyProperties, minImageTransferGranularity));

    // VkQueueFamilyProperties2 checks. Same array-reinterpret concern for the _2 query wrapper.
    static_assert(std::is_standard_layout_v<queue_family_properties_2_t>, "queue_family_properties_2_t must be standard-layout for native interop.");
    static_assert(sizeof(queue_family_properties_2_t) == sizeof(VkQueueFamilyProperties2), "queue_family_properties_2_t layout diverged from VkQueueFamilyProperties2.");
    static_assert(offsetof(queue_family_properties_2_t, sType) == offsetof(VkQueueFamilyProperties2, sType));
    static_assert(offsetof(queue_family_properties_2_t, pNext) == offsetof(VkQueueFamilyProperties2, pNext));
    static_assert(offsetof(queue_family_properties_2_t, queueFamilyProperties) == offsetof(VkQueueFamilyProperties2, queueFamilyProperties));

    // VkDeviceQueueCreateInfo checks.
    static_assert(std::is_standard_layout_v<device_queue_create_info_t>, "device_queue_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(device_queue_create_info_t) == sizeof(VkDeviceQueueCreateInfo), "device_queue_create_info_t layout diverged from VkDeviceQueueCreateInfo.");
    static_assert(offsetof(device_queue_create_info_t, sType) == offsetof(VkDeviceQueueCreateInfo, sType));
    static_assert(offsetof(device_queue_create_info_t, pNext) == offsetof(VkDeviceQueueCreateInfo, pNext));
    static_assert(offsetof(device_queue_create_info_t, flags) == offsetof(VkDeviceQueueCreateInfo, flags));
    static_assert(offsetof(device_queue_create_info_t, queueFamilyIndex) == offsetof(VkDeviceQueueCreateInfo, queueFamilyIndex));
    static_assert(offsetof(device_queue_create_info_t, queueCount) == offsetof(VkDeviceQueueCreateInfo, queueCount));
    static_assert(offsetof(device_queue_create_info_t, pQueuePriorities) == offsetof(VkDeviceQueueCreateInfo, pQueuePriorities));

    // VkDeviceCreateInfo checks.
    static_assert(std::is_standard_layout_v<device_create_info_t>, "device_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(device_create_info_t) == sizeof(VkDeviceCreateInfo), "device_create_info_t layout diverged from VkDeviceCreateInfo.");
    static_assert(offsetof(device_create_info_t, sType) == offsetof(VkDeviceCreateInfo, sType));
    static_assert(offsetof(device_create_info_t, pNext) == offsetof(VkDeviceCreateInfo, pNext));
    static_assert(offsetof(device_create_info_t, flags) == offsetof(VkDeviceCreateInfo, flags));
    static_assert(offsetof(device_create_info_t, queueCreateInfoCount) == offsetof(VkDeviceCreateInfo, queueCreateInfoCount));
    static_assert(offsetof(device_create_info_t, pQueueCreateInfos) == offsetof(VkDeviceCreateInfo, pQueueCreateInfos));
    static_assert(offsetof(device_create_info_t, enabledLayerCount) == offsetof(VkDeviceCreateInfo, enabledLayerCount));
    static_assert(offsetof(device_create_info_t, ppEnabledLayerNames) == offsetof(VkDeviceCreateInfo, ppEnabledLayerNames));
    static_assert(offsetof(device_create_info_t, enabledExtensionCount) == offsetof(VkDeviceCreateInfo, enabledExtensionCount));
    static_assert(offsetof(device_create_info_t, ppEnabledExtensionNames) == offsetof(VkDeviceCreateInfo, ppEnabledExtensionNames));
    static_assert(offsetof(device_create_info_t, pEnabledFeatures) == offsetof(VkDeviceCreateInfo, pEnabledFeatures));

    // VkComponentMapping checks.
    static_assert(std::is_standard_layout_v<component_mapping_t>, "component_mapping_t must be standard-layout for native interop.");
    static_assert(sizeof(component_mapping_t) == sizeof(VkComponentMapping), "component_mapping_t layout diverged from VkComponentMapping.");
    static_assert(offsetof(component_mapping_t, r) == offsetof(VkComponentMapping, r));
    static_assert(offsetof(component_mapping_t, a) == offsetof(VkComponentMapping, a));

    // VkImageSubresourceRange checks.
    static_assert(std::is_standard_layout_v<image_subresource_range_t>, "image_subresource_range_t must be standard-layout for native interop.");
    static_assert(sizeof(image_subresource_range_t) == sizeof(VkImageSubresourceRange), "image_subresource_range_t layout diverged from VkImageSubresourceRange.");
    static_assert(offsetof(image_subresource_range_t, aspectMask) == offsetof(VkImageSubresourceRange, aspectMask));
    static_assert(offsetof(image_subresource_range_t, layerCount) == offsetof(VkImageSubresourceRange, layerCount));

    // VkImageViewCreateInfo checks.
    static_assert(std::is_standard_layout_v<image_view_create_info_t>, "image_view_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(image_view_create_info_t) == sizeof(VkImageViewCreateInfo), "image_view_create_info_t layout diverged from VkImageViewCreateInfo.");
    static_assert(offsetof(image_view_create_info_t, sType) == offsetof(VkImageViewCreateInfo, sType));
    static_assert(offsetof(image_view_create_info_t, pNext) == offsetof(VkImageViewCreateInfo, pNext));
    static_assert(offsetof(image_view_create_info_t, flags) == offsetof(VkImageViewCreateInfo, flags));
    static_assert(offsetof(image_view_create_info_t, image) == offsetof(VkImageViewCreateInfo, image));
    static_assert(offsetof(image_view_create_info_t, viewType) == offsetof(VkImageViewCreateInfo, viewType));
    static_assert(offsetof(image_view_create_info_t, format) == offsetof(VkImageViewCreateInfo, format));
    static_assert(offsetof(image_view_create_info_t, components) == offsetof(VkImageViewCreateInfo, components));
    static_assert(offsetof(image_view_create_info_t, subresourceRange) == offsetof(VkImageViewCreateInfo, subresourceRange));

    // VkSwapchainCreateInfoKHR checks.
    static_assert(std::is_standard_layout_v<swapchain_create_info_t>, "swapchain_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(swapchain_create_info_t) == sizeof(VkSwapchainCreateInfoKHR), "swapchain_create_info_t layout diverged from VkSwapchainCreateInfoKHR.");
    static_assert(offsetof(swapchain_create_info_t, sType) == offsetof(VkSwapchainCreateInfoKHR, sType));
    static_assert(offsetof(swapchain_create_info_t, pNext) == offsetof(VkSwapchainCreateInfoKHR, pNext));
    static_assert(offsetof(swapchain_create_info_t, flags) == offsetof(VkSwapchainCreateInfoKHR, flags));
    static_assert(offsetof(swapchain_create_info_t, surface) == offsetof(VkSwapchainCreateInfoKHR, surface));
    static_assert(offsetof(swapchain_create_info_t, minImageCount) == offsetof(VkSwapchainCreateInfoKHR, minImageCount));
    static_assert(offsetof(swapchain_create_info_t, imageFormat) == offsetof(VkSwapchainCreateInfoKHR, imageFormat));
    static_assert(offsetof(swapchain_create_info_t, imageColorSpace) == offsetof(VkSwapchainCreateInfoKHR, imageColorSpace));
    static_assert(offsetof(swapchain_create_info_t, imageExtent) == offsetof(VkSwapchainCreateInfoKHR, imageExtent));
    static_assert(offsetof(swapchain_create_info_t, imageArrayLayers) == offsetof(VkSwapchainCreateInfoKHR, imageArrayLayers));
    static_assert(offsetof(swapchain_create_info_t, imageUsage) == offsetof(VkSwapchainCreateInfoKHR, imageUsage));
    static_assert(offsetof(swapchain_create_info_t, imageSharingMode) == offsetof(VkSwapchainCreateInfoKHR, imageSharingMode));
    static_assert(offsetof(swapchain_create_info_t, queueFamilyIndexCount) == offsetof(VkSwapchainCreateInfoKHR, queueFamilyIndexCount));
    static_assert(offsetof(swapchain_create_info_t, pQueueFamilyIndices) == offsetof(VkSwapchainCreateInfoKHR, pQueueFamilyIndices));
    static_assert(offsetof(swapchain_create_info_t, preTransform) == offsetof(VkSwapchainCreateInfoKHR, preTransform));
    static_assert(offsetof(swapchain_create_info_t, compositeAlpha) == offsetof(VkSwapchainCreateInfoKHR, compositeAlpha));
    static_assert(offsetof(swapchain_create_info_t, presentMode) == offsetof(VkSwapchainCreateInfoKHR, presentMode));
    static_assert(offsetof(swapchain_create_info_t, clipped) == offsetof(VkSwapchainCreateInfoKHR, clipped));
    static_assert(offsetof(swapchain_create_info_t, oldSwapchain) == offsetof(VkSwapchainCreateInfoKHR, oldSwapchain));

    // VkShaderModuleCreateInfo checks.
    static_assert(std::is_standard_layout_v<shader_module_create_info_t>, "shader_module_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(shader_module_create_info_t) == sizeof(VkShaderModuleCreateInfo), "shader_module_create_info_t layout diverged from VkShaderModuleCreateInfo.");
    static_assert(offsetof(shader_module_create_info_t, sType) == offsetof(VkShaderModuleCreateInfo, sType));
    static_assert(offsetof(shader_module_create_info_t, pNext) == offsetof(VkShaderModuleCreateInfo, pNext));
    static_assert(offsetof(shader_module_create_info_t, flags) == offsetof(VkShaderModuleCreateInfo, flags));
    static_assert(offsetof(shader_module_create_info_t, codeSize) == offsetof(VkShaderModuleCreateInfo, codeSize));
    static_assert(offsetof(shader_module_create_info_t, pCode) == offsetof(VkShaderModuleCreateInfo, pCode));


}
