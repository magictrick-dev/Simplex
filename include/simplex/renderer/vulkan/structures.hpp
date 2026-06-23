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
    // Graphics pipeline fixed-function state.
    //
    // The structs below describe the fixed-function stages of a graphics pipeline (vertex input,
    // input assembly, viewport/scissor, rasterization, multisampling, depth/stencil, color blending),
    // plus the dynamic-state declaration and the pipeline layout. They are inputs assembled into a
    // VkGraphicsPipelineCreateInfo; only the pipeline layout produces a handle of its own (the rest
    // are transient descriptions). Defaults are chosen to be valid out of the box -- notably the
    // rasterizer's lineWidth (1.0, required to be non-zero) and the color blend attachment's RGBA
    // write mask (zero would silently discard all output).
    // ---------------------------------------------------------------------------------------------

    /// @brief VkOffset2D mixin (offset_2d_t). The integer origin of a VkRect2D (e.g. a scissor).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkOffset2D>
    {

        int32_t x   {         };
        int32_t y   {         };

        inline int32_t get_x() const { return this->x; }
        inline int32_t get_y() const { return this->y; }

        inline derived_t& set_x(int32_t v) { this->x = v; return *s(); }
        inline derived_t& set_y(int32_t v) { this->y = v; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkRect2D mixin (rect_2d_t). An integer rectangle (offset + extent); the scissor type.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkRect2D>
    {

        vk_struct_base<VkOffset2D>  offset  {         };
        vk_struct_base<VkExtent2D>  extent  {         };

        inline const vk_struct_base<VkOffset2D>& get_offset() const { return this->offset; }
        inline const vk_struct_base<VkExtent2D>& get_extent() const { return this->extent; }

        inline derived_t& set_offset(const vk_struct_base<VkOffset2D>& o) { this->offset = o; return *s(); }
        inline derived_t& set_extent(const vk_struct_base<VkExtent2D>& e) { this->extent = e; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkViewport mixin (viewport_t). x/y/width/height in pixels, depth range in [0, 1].
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkViewport>
    {

        float   x           {         };
        float   y           {         };
        float   width       {         };
        float   height      {         };
        float   minDepth    {         };
        float   maxDepth    {         };

        inline float get_x() const          { return this->x;           }
        inline float get_y() const          { return this->y;           }
        inline float get_width() const      { return this->width;       }
        inline float get_height() const     { return this->height;      }
        inline float get_min_depth() const  { return this->minDepth;    }
        inline float get_max_depth() const  { return this->maxDepth;    }

        inline derived_t& set_x(float v)          { this->x = v; return *s();         }
        inline derived_t& set_y(float v)          { this->y = v; return *s();         }
        inline derived_t& set_width(float v)      { this->width = v; return *s();     }
        inline derived_t& set_height(float v)     { this->height = v; return *s();    }
        inline derived_t& set_min_depth(float v)  { this->minDepth = v; return *s();  }
        inline derived_t& set_max_depth(float v)  { this->maxDepth = v; return *s();  }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkVertexInputBindingDescription mixin (vertex_input_binding_description_t).
    ///
    /// Describes one bound vertex buffer: its binding number, the per-vertex (or per-instance) stride,
    /// and whether it advances per vertex or per instance.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkVertexInputBindingDescription>
    {

        uint32_t            binding     {         };
        uint32_t            stride      {         };
        VkVertexInputRate   inputRate   {         };

        inline uint32_t          get_binding() const    { return this->binding;    }
        inline uint32_t          get_stride() const     { return this->stride;     }
        inline VkVertexInputRate get_input_rate() const { return this->inputRate;  }

        inline derived_t& set_binding(uint32_t b)               { this->binding = b; return *s();   }
        inline derived_t& set_stride(uint32_t s)                { this->stride = s; return *self(); }
        inline derived_t& set_input_rate(VkVertexInputRate r)   { this->inputRate = r; return *s(); }

        private:
            inline derived_t* s()    { return reinterpret_cast<derived_t*>(this); }
            inline derived_t* self() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkVertexInputAttributeDescription mixin (vertex_input_attribute_description_t).
    ///
    /// Describes one vertex attribute: its shader location, which binding it is sourced from, its
    /// format, and its byte offset within the vertex.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkVertexInputAttributeDescription>
    {

        uint32_t    location    {         };
        uint32_t    binding     {         };
        VkFormat    format      {         };
        uint32_t    offset      {         };

        inline uint32_t get_location() const { return this->location;   }
        inline uint32_t get_binding() const  { return this->binding;    }
        inline VkFormat get_format() const   { return this->format;     }
        inline uint32_t get_offset() const   { return this->offset;     }

        inline derived_t& set_location(uint32_t l)  { this->location = l; return *s();  }
        inline derived_t& set_binding(uint32_t b)   { this->binding = b; return *s();   }
        inline derived_t& set_format(VkFormat f)    { this->format = f; return *s();    }
        inline derived_t& set_offset(uint32_t o)    { this->offset = o; return *s();    }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineVertexInputStateCreateInfo mixin (pipeline_vertex_input_state_create_info_t).
    ///
    /// Aggregates the bound vertex buffers (binding descriptions) and their attributes. The default
    /// (no bindings, no attributes) is exactly right for shaders that source their vertices without a
    /// vertex buffer (e.g. a fullscreen triangle generated from gl_VertexIndex). The referenced
    /// description arrays must outlive the pipeline create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineVertexInputStateCreateInfo>
    {

        VkStructureType                                             sType                           { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        const void*                                                 pNext                           { nullptr };
        VkPipelineVertexInputStateCreateFlags                       flags                           {         };
        uint32_t                                                    vertexBindingDescriptionCount   {         };
        const vk_struct_base<VkVertexInputBindingDescription>*      pVertexBindingDescriptions      { nullptr };
        uint32_t                                                    vertexAttributeDescriptionCount {         };
        const vk_struct_base<VkVertexInputAttributeDescription>*    pVertexAttributeDescriptions    { nullptr };

        inline const void*                       get_next() const   { return this->pNext;   }
        inline VkPipelineVertexInputStateCreateFlags get_flags() const { return this->flags; }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineVertexInputStateCreateFlags f){ this->flags = f; return *s();     }

        inline derived_t& set_binding_descriptions(spx::array_view<const vk_struct_base<VkVertexInputBindingDescription>> bindings)
        {
            this->pVertexBindingDescriptions     = bindings.data();
            this->vertexBindingDescriptionCount  = static_cast<uint32_t>(bindings.size());
            return *s();
        }

        inline derived_t& set_attribute_descriptions(spx::array_view<const vk_struct_base<VkVertexInputAttributeDescription>> attributes)
        {
            this->pVertexAttributeDescriptions     = attributes.data();
            this->vertexAttributeDescriptionCount  = static_cast<uint32_t>(attributes.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineInputAssemblyStateCreateInfo mixin
    ///        (pipeline_input_assembly_state_create_info_t).
    ///
    /// How vertices are assembled into primitives. Defaults to a triangle list with primitive restart
    /// disabled -- the common case.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineInputAssemblyStateCreateInfo>
    {

        VkStructureType                          sType                  { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        const void*                              pNext                  { nullptr };
        VkPipelineInputAssemblyStateCreateFlags  flags                  {         };
        VkPrimitiveTopology                      topology               { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
        VkBool32                                 primitiveRestartEnable  {         };

        inline const void*          get_next() const                    { return this->pNext;                   }
        inline VkPrimitiveTopology  get_topology() const                { return this->topology;                }
        inline VkBool32             get_primitive_restart_enable() const{ return this->primitiveRestartEnable;  }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();          }
        inline derived_t& set_flags(VkPipelineInputAssemblyStateCreateFlags f) { this->flags = f; return *s();          }
        inline derived_t& set_topology(VkPrimitiveTopology t)               { this->topology = t; return *s();          }
        inline derived_t& set_primitive_restart_enable(VkBool32 enable)     { this->primitiveRestartEnable = enable; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineViewportStateCreateInfo mixin (pipeline_viewport_state_create_info_t).
    ///
    /// Declares the viewport and scissor counts (defaulting to one each). When the viewport/scissor
    /// are dynamic state -- the recommended setup -- the counts are all that matter here and the
    /// pointers stay null, with the actual rectangles supplied at draw time via vkCmdSetViewport /
    /// vkCmdSetScissor. Otherwise the rectangle arrays are baked in and must outlive the create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineViewportStateCreateInfo>
    {

        VkStructureType                     sType           { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        const void*                         pNext           { nullptr };
        VkPipelineViewportStateCreateFlags  flags           {         };
        uint32_t                            viewportCount   { 1       };
        const vk_struct_base<VkViewport>*   pViewports      { nullptr };
        uint32_t                            scissorCount    { 1       };
        const vk_struct_base<VkRect2D>*     pScissors       { nullptr };

        inline const void*  get_next() const            { return this->pNext;           }
        inline uint32_t     get_viewport_count() const  { return this->viewportCount;   }
        inline uint32_t     get_scissor_count() const   { return this->scissorCount;    }

        inline derived_t& set_next(const void* next)                    { this->pNext = next; return *s();          }
        inline derived_t& set_flags(VkPipelineViewportStateCreateFlags f) { this->flags = f; return *s();           }
        inline derived_t& set_viewport_count(uint32_t count)            { this->viewportCount = count; return *s(); }
        inline derived_t& set_scissor_count(uint32_t count)             { this->scissorCount = count; return *s();  }

        inline derived_t& set_viewports(spx::array_view<const vk_struct_base<VkViewport>> viewports)
        {
            this->pViewports    = viewports.data();
            this->viewportCount = static_cast<uint32_t>(viewports.size());
            return *s();
        }

        inline derived_t& set_scissors(spx::array_view<const vk_struct_base<VkRect2D>> scissors)
        {
            this->pScissors    = scissors.data();
            this->scissorCount = static_cast<uint32_t>(scissors.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineRasterizationStateCreateInfo mixin
    ///        (pipeline_rasterization_state_create_info_t).
    ///
    /// Turns primitives into fragments. lineWidth defaults to 1.0 (the spec requires a non-zero width;
    /// zero would be invalid). polygonMode defaults to fill, cullMode to none, and frontFace to
    /// counter-clockwise (all the zero-valued enumerators) -- a culling-disabled config that draws
    /// regardless of winding, which is the safest starting point. Enable back-face culling explicitly
    /// once the winding of your geometry is settled.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineRasterizationStateCreateInfo>
    {

        VkStructureType                          sType                      { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        const void*                              pNext                      { nullptr };
        VkPipelineRasterizationStateCreateFlags  flags                      {         };
        VkBool32                                 depthClampEnable           {         };
        VkBool32                                 rasterizerDiscardEnable    {         };
        VkPolygonMode                            polygonMode                {         };
        VkCullModeFlags                          cullMode                   {         };
        VkFrontFace                              frontFace                  {         };
        VkBool32                                 depthBiasEnable            {         };
        float                                    depthBiasConstantFactor    {         };
        float                                    depthBiasClamp             {         };
        float                                    depthBiasSlopeFactor       {         };
        float                                    lineWidth                  { 1.0f    };

        inline const void*      get_next() const                        { return this->pNext;                   }
        inline VkBool32         get_depth_clamp_enable() const          { return this->depthClampEnable;        }
        inline VkBool32         get_rasterizer_discard_enable() const   { return this->rasterizerDiscardEnable; }
        inline VkPolygonMode    get_polygon_mode() const                { return this->polygonMode;             }
        inline VkCullModeFlags  get_cull_mode() const                   { return this->cullMode;                }
        inline VkFrontFace      get_front_face() const                  { return this->frontFace;               }
        inline VkBool32         get_depth_bias_enable() const           { return this->depthBiasEnable;         }
        inline float            get_line_width() const                  { return this->lineWidth;               }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();              }
        inline derived_t& set_flags(VkPipelineRasterizationStateCreateFlags f) { this->flags = f; return *s();              }
        inline derived_t& set_depth_clamp_enable(VkBool32 enable)           { this->depthClampEnable = enable; return *s(); }
        inline derived_t& set_rasterizer_discard_enable(VkBool32 enable)    { this->rasterizerDiscardEnable = enable; return *s(); }
        inline derived_t& set_polygon_mode(VkPolygonMode mode)              { this->polygonMode = mode; return *s();        }
        inline derived_t& set_cull_mode(VkCullModeFlags mode)               { this->cullMode = mode; return *s();           }
        inline derived_t& set_front_face(VkFrontFace face)                  { this->frontFace = face; return *s();          }
        inline derived_t& set_depth_bias_enable(VkBool32 enable)            { this->depthBiasEnable = enable; return *s();  }
        inline derived_t& set_depth_bias_constant_factor(float v)           { this->depthBiasConstantFactor = v; return *s(); }
        inline derived_t& set_depth_bias_clamp(float v)                     { this->depthBiasClamp = v; return *s();        }
        inline derived_t& set_depth_bias_slope_factor(float v)              { this->depthBiasSlopeFactor = v; return *s();  }
        inline derived_t& set_line_width(float w)                           { this->lineWidth = w; return *s();             }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineMultisampleStateCreateInfo mixin
    ///        (pipeline_multisample_state_create_info_t).
    ///
    /// Defaults to multisampling disabled (one sample per pixel), which is the no-MSAA configuration.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineMultisampleStateCreateInfo>
    {

        VkStructureType                         sType                   { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        const void*                             pNext                   { nullptr };
        VkPipelineMultisampleStateCreateFlags   flags                   {         };
        VkSampleCountFlagBits                   rasterizationSamples    { VK_SAMPLE_COUNT_1_BIT };
        VkBool32                                sampleShadingEnable     {         };
        float                                   minSampleShading        { 1.0f    };
        const VkSampleMask*                     pSampleMask             { nullptr };
        VkBool32                                alphaToCoverageEnable   {         };
        VkBool32                                alphaToOneEnable        {         };

        inline const void*              get_next() const                    { return this->pNext;                   }
        inline VkSampleCountFlagBits    get_rasterization_samples() const   { return this->rasterizationSamples;    }
        inline VkBool32                 get_sample_shading_enable() const    { return this->sampleShadingEnable;     }
        inline float                    get_min_sample_shading() const       { return this->minSampleShading;        }
        inline VkBool32                 get_alpha_to_coverage_enable() const { return this->alphaToCoverageEnable;   }
        inline VkBool32                 get_alpha_to_one_enable() const      { return this->alphaToOneEnable;        }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();      }
        inline derived_t& set_flags(VkPipelineMultisampleStateCreateFlags f){ this->flags = f; return *s();         }
        inline derived_t& set_rasterization_samples(VkSampleCountFlagBits s){ this->rasterizationSamples = s; return *self(); }
        inline derived_t& set_sample_shading_enable(VkBool32 enable)        { this->sampleShadingEnable = enable; return *s(); }
        inline derived_t& set_min_sample_shading(float v)                   { this->minSampleShading = v; return *s();  }
        inline derived_t& set_sample_mask(const VkSampleMask* mask)         { this->pSampleMask = mask; return *s();    }
        inline derived_t& set_alpha_to_coverage_enable(VkBool32 enable)     { this->alphaToCoverageEnable = enable; return *s(); }
        inline derived_t& set_alpha_to_one_enable(VkBool32 enable)          { this->alphaToOneEnable = enable; return *s(); }

        private:
            inline derived_t* s()    { return reinterpret_cast<derived_t*>(this); }
            inline derived_t* self() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkStencilOpState mixin (stencil_op_state_t). The front/back stencil configuration nested
    ///        in the depth/stencil state below.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkStencilOpState>
    {

        VkStencilOp     failOp      {         };
        VkStencilOp     passOp      {         };
        VkStencilOp     depthFailOp {         };
        VkCompareOp     compareOp   {         };
        uint32_t        compareMask {         };
        uint32_t        writeMask   {         };
        uint32_t        reference   {         };

        inline VkStencilOp  get_fail_op() const         { return this->failOp;      }
        inline VkStencilOp  get_pass_op() const         { return this->passOp;      }
        inline VkStencilOp  get_depth_fail_op() const   { return this->depthFailOp; }
        inline VkCompareOp  get_compare_op() const      { return this->compareOp;   }
        inline uint32_t     get_compare_mask() const    { return this->compareMask; }
        inline uint32_t     get_write_mask() const      { return this->writeMask;   }
        inline uint32_t     get_reference() const       { return this->reference;   }

        inline derived_t& set_fail_op(VkStencilOp op)       { this->failOp = op; return *s();       }
        inline derived_t& set_pass_op(VkStencilOp op)       { this->passOp = op; return *s();       }
        inline derived_t& set_depth_fail_op(VkStencilOp op) { this->depthFailOp = op; return *s();  }
        inline derived_t& set_compare_op(VkCompareOp op)    { this->compareOp = op; return *s();    }
        inline derived_t& set_compare_mask(uint32_t mask)   { this->compareMask = mask; return *s();}
        inline derived_t& set_write_mask(uint32_t mask)     { this->writeMask = mask; return *s();  }
        inline derived_t& set_reference(uint32_t ref)       { this->reference = ref; return *s();   }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineDepthStencilStateCreateInfo mixin
    ///        (pipeline_depth_stencil_state_create_info_t).
    ///
    /// Defaults to depth and stencil testing disabled. Enable depth testing and set depthCompareOp
    /// (commonly VK_COMPARE_OP_LESS) when rendering with a depth attachment.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineDepthStencilStateCreateInfo>
    {

        VkStructureType                         sType                   { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        const void*                             pNext                   { nullptr };
        VkPipelineDepthStencilStateCreateFlags  flags                   {         };
        VkBool32                                depthTestEnable         {         };
        VkBool32                                depthWriteEnable        {         };
        VkCompareOp                             depthCompareOp          {         };
        VkBool32                                depthBoundsTestEnable   {         };
        VkBool32                                stencilTestEnable       {         };
        vk_struct_base<VkStencilOpState>        front                   {         };
        vk_struct_base<VkStencilOpState>        back                    {         };
        float                                   minDepthBounds          {         };
        float                                   maxDepthBounds          {         };

        inline const void*  get_next() const                    { return this->pNext;                   }
        inline VkBool32     get_depth_test_enable() const       { return this->depthTestEnable;         }
        inline VkBool32     get_depth_write_enable() const      { return this->depthWriteEnable;        }
        inline VkCompareOp  get_depth_compare_op() const        { return this->depthCompareOp;          }
        inline VkBool32     get_depth_bounds_test_enable() const{ return this->depthBoundsTestEnable;   }
        inline VkBool32     get_stencil_test_enable() const     { return this->stencilTestEnable;       }
        inline const vk_struct_base<VkStencilOpState>& get_front() const { return this->front;          }
        inline const vk_struct_base<VkStencilOpState>& get_back() const  { return this->back;           }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();          }
        inline derived_t& set_flags(VkPipelineDepthStencilStateCreateFlags f) { this->flags = f; return *s();           }
        inline derived_t& set_depth_test_enable(VkBool32 enable)            { this->depthTestEnable = enable; return *s(); }
        inline derived_t& set_depth_write_enable(VkBool32 enable)           { this->depthWriteEnable = enable; return *s(); }
        inline derived_t& set_depth_compare_op(VkCompareOp op)              { this->depthCompareOp = op; return *s();   }
        inline derived_t& set_depth_bounds_test_enable(VkBool32 enable)     { this->depthBoundsTestEnable = enable; return *s(); }
        inline derived_t& set_stencil_test_enable(VkBool32 enable)          { this->stencilTestEnable = enable; return *s(); }
        inline derived_t& set_front(const vk_struct_base<VkStencilOpState>& f) { this->front = f; return *s();          }
        inline derived_t& set_back(const vk_struct_base<VkStencilOpState>& b)  { this->back = b; return *s();           }
        inline derived_t& set_min_depth_bounds(float v)                     { this->minDepthBounds = v; return *s();    }
        inline derived_t& set_max_depth_bounds(float v)                     { this->maxDepthBounds = v; return *s();    }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineColorBlendAttachmentState mixin (pipeline_color_blend_attachment_state_t).
    ///
    /// Per-attachment blend configuration. colorWriteMask defaults to all four channels (a zero mask
    /// would discard every fragment's output, a frequent silent bug). blendEnable defaults to false
    /// (writes replace the destination). The opaque() and alpha_blend() factories cover the two common
    /// cases.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineColorBlendAttachmentState>
    {

        VkBool32                blendEnable         {         };
        VkBlendFactor           srcColorBlendFactor {         };
        VkBlendFactor           dstColorBlendFactor {         };
        VkBlendOp               colorBlendOp        {         };
        VkBlendFactor           srcAlphaBlendFactor {         };
        VkBlendFactor           dstAlphaBlendFactor {         };
        VkBlendOp               alphaBlendOp        {         };
        VkColorComponentFlags   colorWriteMask      { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };

        inline VkBool32                 get_blend_enable() const            { return this->blendEnable;         }
        inline VkBlendFactor            get_src_color_blend_factor() const  { return this->srcColorBlendFactor; }
        inline VkBlendFactor            get_dst_color_blend_factor() const  { return this->dstColorBlendFactor; }
        inline VkBlendOp                get_color_blend_op() const          { return this->colorBlendOp;        }
        inline VkBlendFactor            get_src_alpha_blend_factor() const  { return this->srcAlphaBlendFactor; }
        inline VkBlendFactor            get_dst_alpha_blend_factor() const  { return this->dstAlphaBlendFactor; }
        inline VkBlendOp                get_alpha_blend_op() const          { return this->alphaBlendOp;        }
        inline VkColorComponentFlags    get_color_write_mask() const        { return this->colorWriteMask;      }

        inline derived_t& set_blend_enable(VkBool32 enable)             { this->blendEnable = enable; return *s();          }
        inline derived_t& set_src_color_blend_factor(VkBlendFactor f)   { this->srcColorBlendFactor = f; return *s();       }
        inline derived_t& set_dst_color_blend_factor(VkBlendFactor f)   { this->dstColorBlendFactor = f; return *s();       }
        inline derived_t& set_color_blend_op(VkBlendOp op)              { this->colorBlendOp = op; return *s();             }
        inline derived_t& set_src_alpha_blend_factor(VkBlendFactor f)   { this->srcAlphaBlendFactor = f; return *s();       }
        inline derived_t& set_dst_alpha_blend_factor(VkBlendFactor f)   { this->dstAlphaBlendFactor = f; return *s();       }
        inline derived_t& set_alpha_blend_op(VkBlendOp op)              { this->alphaBlendOp = op; return *s();             }
        inline derived_t& set_color_write_mask(VkColorComponentFlags m) { this->colorWriteMask = m; return *s();            }

        /// @brief Opaque (no blending): fragment output replaces the destination, all channels written.
        static inline derived_t
        opaque()
        {
            derived_t state { };
            state.blendEnable = VK_FALSE;
            return state;
        }

        /// @brief Standard source-over alpha blending: out = src.a * src + (1 - src.a) * dst.
        static inline derived_t
        alpha_blend()
        {
            derived_t state { };
            state.blendEnable         = VK_TRUE;
            state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            state.colorBlendOp        = VK_BLEND_OP_ADD;
            state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            state.alphaBlendOp        = VK_BLEND_OP_ADD;
            return state;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineColorBlendStateCreateInfo mixin (pipeline_color_blend_state_create_info_t).
    ///
    /// Aggregates the per-attachment blend states and the (rarely used) logic-op path. The attachment
    /// array must outlive the pipeline create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineColorBlendStateCreateInfo>
    {

        VkStructureType                                             sType           { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        const void*                                                 pNext           { nullptr };
        VkPipelineColorBlendStateCreateFlags                        flags           {         };
        VkBool32                                                    logicOpEnable   {         };
        VkLogicOp                                                   logicOp         {         };
        uint32_t                                                    attachmentCount {         };
        const vk_struct_base<VkPipelineColorBlendAttachmentState>*  pAttachments    { nullptr };
        float                                                       blendConstants[4] {       };

        inline const void*  get_next() const            { return this->pNext;           }
        inline VkBool32     get_logic_op_enable() const { return this->logicOpEnable;   }
        inline VkLogicOp    get_logic_op() const        { return this->logicOp;         }
        inline const float* get_blend_constants() const { return this->blendConstants;  }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();          }
        inline derived_t& set_flags(VkPipelineColorBlendStateCreateFlags f) { this->flags = f; return *s();             }
        inline derived_t& set_logic_op_enable(VkBool32 enable)              { this->logicOpEnable = enable; return *s();}
        inline derived_t& set_logic_op(VkLogicOp op)                        { this->logicOp = op; return *s();          }

        inline derived_t& set_attachments(spx::array_view<const vk_struct_base<VkPipelineColorBlendAttachmentState>> attachments)
        {
            this->pAttachments    = attachments.data();
            this->attachmentCount = static_cast<uint32_t>(attachments.size());
            return *s();
        }

        inline derived_t& set_blend_constants(float r, float g, float b, float a)
        {
            this->blendConstants[0] = r;
            this->blendConstants[1] = g;
            this->blendConstants[2] = b;
            this->blendConstants[3] = a;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineDynamicStateCreateInfo mixin (pipeline_dynamic_state_create_info_t).
    ///
    /// Lists the pipeline state that is supplied at command-record time rather than baked into the
    /// pipeline. The usual pair is VK_DYNAMIC_STATE_VIEWPORT and VK_DYNAMIC_STATE_SCISSOR, which lets
    /// one pipeline survive a swapchain resize without being rebuilt. The state array must outlive the
    /// pipeline create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineDynamicStateCreateInfo>
    {

        VkStructureType                     sType               { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        const void*                         pNext               { nullptr };
        VkPipelineDynamicStateCreateFlags   flags               {         };
        uint32_t                            dynamicStateCount   {         };
        const VkDynamicState*               pDynamicStates      { nullptr };

        inline const void*  get_next() const                { return this->pNext;               }
        inline uint32_t     get_dynamic_state_count() const { return this->dynamicStateCount;   }

        inline derived_t& set_next(const void* next)                    { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineDynamicStateCreateFlags f){ this->flags = f; return *s();     }

        inline derived_t& set_dynamic_states(spx::array_view<VkDynamicState> states)
        {
            this->pDynamicStates    = states.data();
            this->dynamicStateCount = static_cast<uint32_t>(states.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPushConstantRange mixin (push_constant_range_t). One push-constant block visible to a
    ///        set of shader stages, by byte offset and size.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPushConstantRange>
    {

        VkShaderStageFlags  stageFlags  {         };
        uint32_t            offset      {         };
        uint32_t            size        {         };

        inline VkShaderStageFlags   get_stage_flags() const { return this->stageFlags;  }
        inline uint32_t             get_offset() const      { return this->offset;      }
        inline uint32_t             get_size() const        { return this->size;        }

        inline derived_t& set_stage_flags(VkShaderStageFlags flags) { this->stageFlags = flags; return *s(); }
        inline derived_t& set_offset(uint32_t o)                    { this->offset = o; return *s();         }
        inline derived_t& set_size(uint32_t sz)                     { this->size = sz; return *s();          }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineLayoutCreateInfo mixin (pipeline_layout_create_info_t).
    ///
    /// Declares the descriptor set layouts and push-constant ranges a pipeline reads through. The
    /// default (no sets, no ranges) is a valid empty layout. Both referenced arrays must outlive the
    /// create call. This is the one fixed-function input that yields a handle (pipeline_layout_t).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineLayoutCreateInfo>
    {

        VkStructureType                             sType                   { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        const void*                                 pNext                   { nullptr };
        VkPipelineLayoutCreateFlags                 flags                   {         };
        uint32_t                                    setLayoutCount          {         };
        const VkDescriptorSetLayout*                pSetLayouts             { nullptr };
        uint32_t                                    pushConstantRangeCount  {         };
        const vk_struct_base<VkPushConstantRange>*  pPushConstantRanges     { nullptr };

        inline const void*                  get_next() const    { return this->pNext;   }
        inline VkPipelineLayoutCreateFlags  get_flags() const   { return this->flags;   }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineLayoutCreateFlags f)  { this->flags = f; return *s();     }

        inline derived_t& set_descriptor_set_layouts(spx::array_view<VkDescriptorSetLayout> layouts)
        {
            this->pSetLayouts    = layouts.data();
            this->setLayoutCount = static_cast<uint32_t>(layouts.size());
            return *s();
        }

        inline derived_t& set_push_constant_ranges(spx::array_view<const vk_struct_base<VkPushConstantRange>> ranges)
        {
            this->pPushConstantRanges    = ranges.data();
            this->pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSpecializationMapEntry mixin (specialization_map_entry_t). Maps one shader constant
    ///        (by ID) to a byte range within a VkSpecializationInfo's data blob.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSpecializationMapEntry>
    {

        uint32_t    constantID  {         };
        uint32_t    offset      {         };
        size_t      size        {         };

        inline uint32_t get_constant_id() const { return this->constantID; }
        inline uint32_t get_offset() const      { return this->offset;     }
        inline size_t   get_size() const        { return this->size;       }

        inline derived_t& set_constant_id(uint32_t id)  { this->constantID = id; return *s();   }
        inline derived_t& set_offset(uint32_t o)        { this->offset = o; return *s();        }
        inline derived_t& set_size(size_t sz)           { this->size = sz; return *s();         }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSpecializationInfo mixin (specialization_info_t).
    ///
    /// Supplies values for a shader's specialization constants at pipeline-creation time. The map
    /// entries and the data blob they index into must both outlive the pipeline create call. Left null
    /// on a shader stage when no specialization constants are used (the common case).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSpecializationInfo>
    {

        uint32_t                                            mapEntryCount   {         };
        const vk_struct_base<VkSpecializationMapEntry>*     pMapEntries     { nullptr };
        size_t                                              dataSize        {         };
        const void*                                         pData           { nullptr };

        inline uint32_t     get_map_entry_count() const { return this->mapEntryCount;   }
        inline size_t       get_data_size() const       { return this->dataSize;        }
        inline const void*  get_data() const            { return this->pData;           }

        inline derived_t& set_map_entries(spx::array_view<const vk_struct_base<VkSpecializationMapEntry>> entries)
        {
            this->pMapEntries   = entries.data();
            this->mapEntryCount = static_cast<uint32_t>(entries.size());
            return *s();
        }

        inline derived_t& set_data(const void* data, size_t size_bytes)
        {
            this->pData    = data;
            this->dataSize = size_bytes;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineShaderStageCreateInfo mixin (pipeline_shader_stage_create_info_t).
    ///
    /// Binds one shader entry point into a pipeline: which stage it is, the module it lives in, and
    /// the entry-point name. Because Slang compiles every entry point into a single module, the same
    /// module handle is typically reused across stages with only the stage bit and name differing --
    /// the from_module factory builds one such stage. The name string and any specialization info
    /// must outlive the pipeline create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineShaderStageCreateInfo>
    {

        VkStructureType                                 sType               { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        const void*                                     pNext               { nullptr };
        VkPipelineShaderStageCreateFlags                flags               {         };
        VkShaderStageFlagBits                           stage               {         };
        VkShaderModule                                  module              { VK_NULL_HANDLE };
        const char*                                     pName               { nullptr };
        const vk_struct_base<VkSpecializationInfo>*     pSpecializationInfo { nullptr };

        inline const void*              get_next() const                { return this->pNext;       }
        inline VkPipelineShaderStageCreateFlags get_flags() const       { return this->flags;       }
        inline VkShaderStageFlagBits    get_stage() const               { return this->stage;       }
        inline VkShaderModule           get_module() const              { return this->module;      }
        inline const char*              get_name() const                { return this->pName;       }

        inline derived_t& set_next(const void* next)                    { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineShaderStageCreateFlags f) { this->flags = f; return *s();     }
        inline derived_t& set_stage(VkShaderStageFlagBits stage)        { this->stage = stage; return *s(); }
        inline derived_t& set_module(VkShaderModule shader_module)      { this->module = shader_module; return *s(); }
        inline derived_t& set_name(const char* name)                    { this->pName = name; return *s();  }

        inline derived_t& set_specialization_info(const vk_struct_base<VkSpecializationInfo>* info)
        {
            this->pSpecializationInfo = info;
            return *s();
        }

        /// @brief Builds a stage for one entry point of a module. The entry-point name string must
        ///        outlive the pipeline create call (a string literal or a stable buffer).
        /// @param shader_module The module containing the entry point.
        /// @param stage         The pipeline stage this entry point runs at.
        /// @param entry_point   The entry-point name (e.g. "vertexMain", "fragmentMain").
        /// @return A stage ready to place in a graphics pipeline's stage array.
        static inline derived_t
        from_module(VkShaderModule shader_module, VkShaderStageFlagBits stage, const char* entry_point)
        {
            derived_t info { };
            info.stage  = stage;
            info.module = shader_module;
            info.pName  = entry_point;
            return info;
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineTessellationStateCreateInfo mixin (pipeline_tessellation_state_create_info_t).
    ///
    /// Only relevant when the pipeline has tessellation shader stages; otherwise the graphics pipeline
    /// leaves pTessellationState null. Carries the patch control point count.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineTessellationStateCreateInfo>
    {

        VkStructureType                         sType               { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
        const void*                             pNext               { nullptr };
        VkPipelineTessellationStateCreateFlags  flags               {         };
        uint32_t                                patchControlPoints  {         };

        inline const void*  get_next() const                    { return this->pNext;               }
        inline uint32_t     get_patch_control_points() const    { return this->patchControlPoints;  }

        inline derived_t& set_next(const void* next)                        { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineTessellationStateCreateFlags f){ this->flags = f; return *s();    }
        inline derived_t& set_patch_control_points(uint32_t points)         { this->patchControlPoints = points; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPipelineRenderingCreateInfo mixin (pipeline_rendering_create_info_t).
    ///
    /// The dynamic-rendering replacement for a render pass at pipeline-build time. Chained off a
    /// graphics pipeline create info's pNext (with renderPass left VK_NULL_HANDLE), it declares the
    /// attachment formats the pipeline will render to. Core since Vulkan 1.3. The color-format array
    /// must outlive the pipeline create call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPipelineRenderingCreateInfo>
    {

        VkStructureType     sType                   { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        const void*         pNext                   { nullptr };
        uint32_t            viewMask                {         };
        uint32_t            colorAttachmentCount    {         };
        const VkFormat*     pColorAttachmentFormats { nullptr };
        VkFormat            depthAttachmentFormat   { VK_FORMAT_UNDEFINED };
        VkFormat            stencilAttachmentFormat { VK_FORMAT_UNDEFINED };

        inline const void*  get_next() const                    { return this->pNext;                   }
        inline uint32_t     get_view_mask() const               { return this->viewMask;                }
        inline VkFormat     get_depth_attachment_format() const { return this->depthAttachmentFormat;   }
        inline VkFormat     get_stencil_attachment_format() const { return this->stencilAttachmentFormat; }
        inline spx::array_view<VkFormat> get_color_attachment_formats() const { return { this->pColorAttachmentFormats, this->colorAttachmentCount }; }

        inline derived_t& set_next(const void* next)                    { this->pNext = next; return *s();              }
        inline derived_t& set_view_mask(uint32_t mask)                  { this->viewMask = mask; return *s();           }
        inline derived_t& set_depth_attachment_format(VkFormat format)  { this->depthAttachmentFormat = format; return *s(); }
        inline derived_t& set_stencil_attachment_format(VkFormat format){ this->stencilAttachmentFormat = format; return *s(); }

        inline derived_t& set_color_attachment_formats(spx::array_view<VkFormat> formats)
        {
            this->pColorAttachmentFormats = formats.data();
            this->colorAttachmentCount    = static_cast<uint32_t>(formats.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkGraphicsPipelineCreateInfo mixin (graphics_pipeline_create_info_t).
    ///
    /// The full description of a graphics pipeline. Each fixed-function state pointer references one
    /// of the wrapped state structs above (layout-identical, so the wrapped pointer is bit-compatible
    /// with the native pointer the API expects). Every referenced struct/array must outlive the
    /// pipeline create call. For dynamic rendering, leave renderPass VK_NULL_HANDLE and chain a
    /// pipeline_rendering_create_info_t off pNext.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkGraphicsPipelineCreateInfo>
    {

        VkStructureType                                                 sType               { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        const void*                                                     pNext               { nullptr };
        VkPipelineCreateFlags                                           flags               {         };
        uint32_t                                                        stageCount          {         };
        const vk_struct_base<VkPipelineShaderStageCreateInfo>*          pStages             { nullptr };
        const vk_struct_base<VkPipelineVertexInputStateCreateInfo>*     pVertexInputState   { nullptr };
        const vk_struct_base<VkPipelineInputAssemblyStateCreateInfo>*   pInputAssemblyState { nullptr };
        const vk_struct_base<VkPipelineTessellationStateCreateInfo>*    pTessellationState  { nullptr };
        const vk_struct_base<VkPipelineViewportStateCreateInfo>*        pViewportState      { nullptr };
        const vk_struct_base<VkPipelineRasterizationStateCreateInfo>*   pRasterizationState { nullptr };
        const vk_struct_base<VkPipelineMultisampleStateCreateInfo>*     pMultisampleState   { nullptr };
        const vk_struct_base<VkPipelineDepthStencilStateCreateInfo>*    pDepthStencilState  { nullptr };
        const vk_struct_base<VkPipelineColorBlendStateCreateInfo>*      pColorBlendState    { nullptr };
        const vk_struct_base<VkPipelineDynamicStateCreateInfo>*         pDynamicState       { nullptr };
        VkPipelineLayout                                                layout              { VK_NULL_HANDLE };
        VkRenderPass                                                    renderPass          { VK_NULL_HANDLE };
        uint32_t                                                        subpass             {         };
        VkPipeline                                                      basePipelineHandle  { VK_NULL_HANDLE };
        int32_t                                                         basePipelineIndex   { -1 };

        inline const void*              get_next() const    { return this->pNext;   }
        inline VkPipelineCreateFlags    get_flags() const   { return this->flags;   }
        inline VkPipelineLayout         get_layout() const  { return this->layout;  }
        inline VkRenderPass             get_render_pass() const { return this->renderPass; }
        inline uint32_t                 get_subpass() const { return this->subpass; }

        inline derived_t& set_next(const void* next)            { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkPipelineCreateFlags f)    { this->flags = f; return *s();     }

        inline derived_t& set_stages(spx::array_view<const vk_struct_base<VkPipelineShaderStageCreateInfo>> stages)
        {
            this->pStages    = stages.data();
            this->stageCount = static_cast<uint32_t>(stages.size());
            return *s();
        }

        inline derived_t& set_vertex_input_state(const vk_struct_base<VkPipelineVertexInputStateCreateInfo>* state)   { this->pVertexInputState = state; return *s(); }
        inline derived_t& set_input_assembly_state(const vk_struct_base<VkPipelineInputAssemblyStateCreateInfo>* state){ this->pInputAssemblyState = state; return *s(); }
        inline derived_t& set_tessellation_state(const vk_struct_base<VkPipelineTessellationStateCreateInfo>* state)  { this->pTessellationState = state; return *s(); }
        inline derived_t& set_viewport_state(const vk_struct_base<VkPipelineViewportStateCreateInfo>* state)          { this->pViewportState = state; return *s(); }
        inline derived_t& set_rasterization_state(const vk_struct_base<VkPipelineRasterizationStateCreateInfo>* state){ this->pRasterizationState = state; return *s(); }
        inline derived_t& set_multisample_state(const vk_struct_base<VkPipelineMultisampleStateCreateInfo>* state)    { this->pMultisampleState = state; return *s(); }
        inline derived_t& set_depth_stencil_state(const vk_struct_base<VkPipelineDepthStencilStateCreateInfo>* state) { this->pDepthStencilState = state; return *s(); }
        inline derived_t& set_color_blend_state(const vk_struct_base<VkPipelineColorBlendStateCreateInfo>* state)     { this->pColorBlendState = state; return *s(); }
        inline derived_t& set_dynamic_state(const vk_struct_base<VkPipelineDynamicStateCreateInfo>* state)            { this->pDynamicState = state; return *s(); }

        inline derived_t& set_layout(VkPipelineLayout pipeline_layout)       { this->layout = pipeline_layout; return *s();  }
        inline derived_t& set_render_pass(VkRenderPass render_pass)          { this->renderPass = render_pass; return *s();  }
        inline derived_t& set_subpass(uint32_t subpass_index)               { this->subpass = subpass_index; return *s();   }
        inline derived_t& set_base_pipeline_handle(VkPipeline base)         { this->basePipelineHandle = base; return *s(); }
        inline derived_t& set_base_pipeline_index(int32_t index)            { this->basePipelineIndex = index; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    // ---------------------------------------------------------------------------------------------
    // Command, synchronization, dynamic-rendering, submission and buffer/memory state.
    //
    // These carry a frame from a built pipeline through to on-screen presentation, and stand up the
    // buffer/memory machinery vertex buffers are built on. Because the renderer uses dynamic
    // rendering (VkPipelineRenderingCreateInfo, Vulkan 1.3 core) there is no render pass or
    // framebuffer object -- the per-frame attachment set is described inline by rendering_info_t /
    // rendering_attachment_info_t and handed to vkCmdBeginRendering instead.
    // ---------------------------------------------------------------------------------------------

    /// @brief VkCommandPoolCreateInfo mixin (command_pool_create_info_t).
    ///
    /// Input struct for the pool command buffers are allocated from. A pool is bound to one queue
    /// family; buffers allocated from it may only be submitted to a queue of that family. The
    /// RESET_COMMAND_BUFFER flag lets individual buffers be re-recorded, which a per-frame draw loop
    /// relies on.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkCommandPoolCreateInfo>
    {

        VkStructureType             sType               { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        const void*                 pNext               { nullptr };
        VkCommandPoolCreateFlags    flags               {         };
        uint32_t                    queueFamilyIndex    {         };

        inline const void*               get_next() const               { return this->pNext;            }
        inline VkCommandPoolCreateFlags  get_flags() const              { return this->flags;            }
        inline uint32_t                  get_queue_family_index() const { return this->queueFamilyIndex; }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();             }
        inline derived_t& set_flags(VkCommandPoolCreateFlags f)     { this->flags = f; return *s();                }
        inline derived_t& set_queue_family_index(uint32_t index)    { this->queueFamilyIndex = index; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkCommandBufferAllocateInfo mixin (command_buffer_allocate_info_t).
    ///
    /// Input struct describing the command buffers to allocate from a pool. Defaults to a single
    /// primary buffer (the common case); commandPool is a raw handle (the wrappers sit above this
    /// layer).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkCommandBufferAllocateInfo>
    {

        VkStructureType         sType               { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        const void*             pNext               { nullptr };
        VkCommandPool           commandPool         { VK_NULL_HANDLE };
        VkCommandBufferLevel    level               { VK_COMMAND_BUFFER_LEVEL_PRIMARY };
        uint32_t                commandBufferCount  { 1 };

        inline const void*           get_next() const                  { return this->pNext;              }
        inline VkCommandPool         get_command_pool() const          { return this->commandPool;        }
        inline VkCommandBufferLevel  get_level() const                 { return this->level;              }
        inline uint32_t              get_command_buffer_count() const  { return this->commandBufferCount; }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();             }
        inline derived_t& set_command_pool(VkCommandPool pool)      { this->commandPool = pool; return *s();       }
        inline derived_t& set_level(VkCommandBufferLevel level)     { this->level = level; return *s();            }
        inline derived_t& set_command_buffer_count(uint32_t count)  { this->commandBufferCount = count; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkCommandBufferBeginInfo mixin (command_buffer_begin_info_t).
    ///
    /// Input struct for vkBeginCommandBuffer. pInheritanceInfo is a raw pointer and is only used by
    /// secondary buffers; primary buffers (all this layer needs) leave it null. ONE_TIME_SUBMIT is
    /// the typical flag for a buffer re-recorded every frame.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkCommandBufferBeginInfo>
    {

        VkStructureType                          sType            { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        const void*                              pNext            { nullptr };
        VkCommandBufferUsageFlags                flags            {         };
        const VkCommandBufferInheritanceInfo*    pInheritanceInfo { nullptr };

        inline const void*                get_next() const  { return this->pNext; }
        inline VkCommandBufferUsageFlags  get_flags() const { return this->flags; }

        inline derived_t& set_next(const void* next)                            { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkCommandBufferUsageFlags f)                { this->flags = f; return *s();     }
        inline derived_t& set_inheritance_info(const VkCommandBufferInheritanceInfo* info) { this->pInheritanceInfo = info; return *s(); }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkRenderingAttachmentInfo mixin (rendering_attachment_info_t).
    ///
    /// Describes one attachment (color, depth or stencil) for a dynamic-rendering pass. imageView /
    /// resolveImageView are raw handles. clearValue is the native VkClearValue union, left raw since
    /// it is a plain POD union; set_clear_color is provided for the common color case. The image
    /// the view refers to must be in imageLayout when rendering begins.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkRenderingAttachmentInfo>
    {

        VkStructureType         sType               { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        const void*             pNext               { nullptr };
        VkImageView             imageView           { VK_NULL_HANDLE };
        VkImageLayout           imageLayout         {         };
        VkResolveModeFlagBits   resolveMode         {         };
        VkImageView             resolveImageView    { VK_NULL_HANDLE };
        VkImageLayout           resolveImageLayout  {         };
        VkAttachmentLoadOp      loadOp              {         };
        VkAttachmentStoreOp     storeOp             {         };
        VkClearValue            clearValue          {         };

        inline const void*           get_next() const              { return this->pNext;            }
        inline VkImageView           get_image_view() const        { return this->imageView;        }
        inline VkImageLayout         get_image_layout() const      { return this->imageLayout;      }
        inline VkAttachmentLoadOp    get_load_op() const           { return this->loadOp;           }
        inline VkAttachmentStoreOp   get_store_op() const          { return this->storeOp;          }
        inline const VkClearValue&   get_clear_value() const       { return this->clearValue;       }

        inline derived_t& set_next(const void* next)                    { this->pNext = next; return *s();             }
        inline derived_t& set_image_view(VkImageView view)             { this->imageView = view; return *s();         }
        inline derived_t& set_image_layout(VkImageLayout layout)       { this->imageLayout = layout; return *s();     }
        inline derived_t& set_resolve_mode(VkResolveModeFlagBits mode) { this->resolveMode = mode; return *s();       }
        inline derived_t& set_resolve_image_view(VkImageView view)     { this->resolveImageView = view; return *s();  }
        inline derived_t& set_resolve_image_layout(VkImageLayout l)    { this->resolveImageLayout = l; return *s();   }
        inline derived_t& set_load_op(VkAttachmentLoadOp op)           { this->loadOp = op; return *s();              }
        inline derived_t& set_store_op(VkAttachmentStoreOp op)         { this->storeOp = op; return *s();             }
        inline derived_t& set_clear_value(const VkClearValue& value)   { this->clearValue = value; return *s();       }

        /// @brief Sets the clear color (used when loadOp is VK_ATTACHMENT_LOAD_OP_CLEAR).
        inline derived_t& set_clear_color(float r, float g, float b, float a)
        {
            this->clearValue.color.float32[0] = r;
            this->clearValue.color.float32[1] = g;
            this->clearValue.color.float32[2] = b;
            this->clearValue.color.float32[3] = a;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkRenderingInfo mixin (rendering_info_t).
    ///
    /// The dynamic-rendering counterpart of a render-pass begin: describes the render area and the
    /// color/depth/stencil attachments for a vkCmdBeginRendering scope. layerCount defaults to 1.
    /// The attachment arrays/pointers are referenced, not copied, and must outlive the begin call.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkRenderingInfo>
    {

        VkStructureType                                     sType                { VK_STRUCTURE_TYPE_RENDERING_INFO };
        const void*                                         pNext                { nullptr };
        VkRenderingFlags                                    flags                {         };
        vk_struct_base<VkRect2D>                            renderArea           {         };
        uint32_t                                            layerCount           { 1       };
        uint32_t                                            viewMask             {         };
        uint32_t                                            colorAttachmentCount {         };
        const vk_struct_base<VkRenderingAttachmentInfo>*    pColorAttachments    { nullptr };
        const vk_struct_base<VkRenderingAttachmentInfo>*    pDepthAttachment     { nullptr };
        const vk_struct_base<VkRenderingAttachmentInfo>*    pStencilAttachment   { nullptr };

        inline const void*              get_next() const        { return this->pNext;       }
        inline VkRenderingFlags         get_flags() const       { return this->flags;       }
        inline const vk_struct_base<VkRect2D>& get_render_area() const { return this->renderArea; }
        inline uint32_t                 get_layer_count() const { return this->layerCount;  }
        inline uint32_t                 get_view_mask() const   { return this->viewMask;    }

        inline derived_t& set_next(const void* next)                                 { this->pNext = next; return *s();      }
        inline derived_t& set_flags(VkRenderingFlags f)                              { this->flags = f; return *s();         }
        inline derived_t& set_render_area(const vk_struct_base<VkRect2D>& area)      { this->renderArea = area; return *s(); }
        inline derived_t& set_layer_count(uint32_t count)                           { this->layerCount = count; return *s();}
        inline derived_t& set_view_mask(uint32_t mask)                              { this->viewMask = mask; return *s();   }
        inline derived_t& set_depth_attachment(const vk_struct_base<VkRenderingAttachmentInfo>* a)   { this->pDepthAttachment = a; return *s();   }
        inline derived_t& set_stencil_attachment(const vk_struct_base<VkRenderingAttachmentInfo>* a) { this->pStencilAttachment = a; return *s(); }

        inline derived_t& set_color_attachments(spx::array_view<const vk_struct_base<VkRenderingAttachmentInfo>> attachments)
        {
            this->pColorAttachments    = attachments.data();
            this->colorAttachmentCount = static_cast<uint32_t>(attachments.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSemaphoreCreateInfo mixin (semaphore_create_info_t).
    ///
    /// Input struct for a binary semaphore (GPU-GPU ordering, e.g. image-acquired / render-finished).
    /// Carries no parameters beyond flags, which are reserved and normally zero.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSemaphoreCreateInfo>
    {

        VkStructureType         sType   { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        const void*             pNext   { nullptr };
        VkSemaphoreCreateFlags  flags   {         };

        inline const void*              get_next() const  { return this->pNext; }
        inline VkSemaphoreCreateFlags   get_flags() const { return this->flags; }

        inline derived_t& set_next(const void* next)             { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkSemaphoreCreateFlags f)    { this->flags = f; return *s();     }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkFenceCreateInfo mixin (fence_create_info_t).
    ///
    /// Input struct for a fence (GPU-CPU ordering). set_signaled creates it already signaled, which
    /// the frames-in-flight loop uses so the first wait on a never-submitted frame doesn't deadlock.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkFenceCreateInfo>
    {

        VkStructureType     sType   { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        const void*         pNext   { nullptr };
        VkFenceCreateFlags  flags   {         };

        inline const void*          get_next() const  { return this->pNext; }
        inline VkFenceCreateFlags   get_flags() const { return this->flags; }

        inline derived_t& set_next(const void* next)         { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkFenceCreateFlags f)    { this->flags = f; return *s();     }

        /// @brief Creates the fence in the signaled state (flags |= VK_FENCE_CREATE_SIGNALED_BIT).
        inline derived_t& set_signaled()
        {
            this->flags |= VK_FENCE_CREATE_SIGNALED_BIT;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkSubmitInfo mixin (submit_info_t).
    ///
    /// Input struct for vkQueueSubmit. The semaphore/command-buffer arrays are raw native handle
    /// arrays (handles carry no layout guard, unlike the struct wrappers) and are referenced, not
    /// copied. The wait-semaphore and wait-dst-stage-mask arrays must be the same length -- entry i
    /// of the stage mask says which pipeline stage waits on wait-semaphore i.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkSubmitInfo>
    {

        VkStructureType                sType                { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        const void*                    pNext                { nullptr };
        uint32_t                       waitSemaphoreCount   {         };
        const VkSemaphore*             pWaitSemaphores      { nullptr };
        const VkPipelineStageFlags*    pWaitDstStageMask    { nullptr };
        uint32_t                       commandBufferCount   {         };
        const VkCommandBuffer*         pCommandBuffers      { nullptr };
        uint32_t                       signalSemaphoreCount {         };
        const VkSemaphore*             pSignalSemaphores    { nullptr };

        inline const void* get_next() const { return this->pNext; }

        inline derived_t& set_next(const void* next) { this->pNext = next; return *s(); }

        /// @brief Sets the wait semaphores together with the matching per-semaphore stage masks. Both
        ///        spans must be the same length; the count is taken from the semaphore span.
        inline derived_t& set_wait_semaphores(spx::array_view<const VkSemaphore> semaphores,
                                              spx::array_view<const VkPipelineStageFlags> stage_masks)
        {
            this->pWaitSemaphores   = semaphores.data();
            this->pWaitDstStageMask = stage_masks.data();
            this->waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
            return *s();
        }

        inline derived_t& set_command_buffers(spx::array_view<const VkCommandBuffer> buffers)
        {
            this->pCommandBuffers    = buffers.data();
            this->commandBufferCount = static_cast<uint32_t>(buffers.size());
            return *s();
        }

        inline derived_t& set_signal_semaphores(spx::array_view<const VkSemaphore> semaphores)
        {
            this->pSignalSemaphores    = semaphores.data();
            this->signalSemaphoreCount = static_cast<uint32_t>(semaphores.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkPresentInfoKHR mixin (present_info_t).
    ///
    /// Input struct for vkQueuePresentKHR. The semaphore/swapchain arrays are raw native handle
    /// arrays, referenced not copied. pImageIndices selects, per swapchain, which image to present;
    /// pResults optionally receives a per-swapchain VkResult (left null for the single-swapchain case
    /// where the call's own return value suffices).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPresentInfoKHR>
    {

        VkStructureType          sType              { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        const void*              pNext              { nullptr };
        uint32_t                 waitSemaphoreCount {         };
        const VkSemaphore*       pWaitSemaphores    { nullptr };
        uint32_t                 swapchainCount     {         };
        const VkSwapchainKHR*    pSwapchains        { nullptr };
        const uint32_t*          pImageIndices      { nullptr };
        VkResult*                pResults           { nullptr };

        inline const void* get_next() const { return this->pNext; }

        inline derived_t& set_next(const void* next)                { this->pNext = next; return *s();      }
        inline derived_t& set_image_indices(const uint32_t* indices){ this->pImageIndices = indices; return *s(); }
        inline derived_t& set_results(VkResult* results)            { this->pResults = results; return *s(); }

        inline derived_t& set_wait_semaphores(spx::array_view<const VkSemaphore> semaphores)
        {
            this->pWaitSemaphores    = semaphores.data();
            this->waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
            return *s();
        }

        inline derived_t& set_swapchains(spx::array_view<const VkSwapchainKHR> swapchains)
        {
            this->pSwapchains    = swapchains.data();
            this->swapchainCount = static_cast<uint32_t>(swapchains.size());
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkBufferCreateInfo mixin (buffer_create_info_t).
    ///
    /// Input struct for vkCreateBuffer. Defaults to exclusive sharing (the common single-family
    /// case); set_concurrent_queue_families switches to concurrent sharing for buffers touched by
    /// more than one queue family (e.g. a separate transfer queue), mirroring the swapchain helper.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkBufferCreateInfo>
    {

        VkStructureType        sType                 { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        const void*            pNext                 { nullptr };
        VkBufferCreateFlags    flags                 {         };
        VkDeviceSize           size                  {         };
        VkBufferUsageFlags     usage                 {         };
        VkSharingMode          sharingMode           { VK_SHARING_MODE_EXCLUSIVE };
        uint32_t               queueFamilyIndexCount {         };
        const uint32_t*        pQueueFamilyIndices   { nullptr };

        inline const void*          get_next() const    { return this->pNext;  }
        inline VkBufferCreateFlags  get_flags() const   { return this->flags;  }
        inline VkDeviceSize         get_size() const    { return this->size;   }
        inline VkBufferUsageFlags   get_usage() const   { return this->usage;  }
        inline VkSharingMode        get_sharing_mode() const { return this->sharingMode; }

        inline derived_t& set_next(const void* next)            { this->pNext = next; return *s();  }
        inline derived_t& set_flags(VkBufferCreateFlags f)      { this->flags = f; return *s();     }
        inline derived_t& set_size(VkDeviceSize size)           { this->size = size; return *s();   }
        inline derived_t& set_usage(VkBufferUsageFlags usage)   { this->usage = usage; return *s(); }

        /// @brief Use concurrent sharing across the given queue families. The index list is
        ///        referenced, not copied, and must outlive the create call.
        inline derived_t& set_concurrent_queue_families(spx::array_view<uint32_t> indices)
        {
            this->sharingMode           = VK_SHARING_MODE_CONCURRENT;
            this->queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
            this->pQueueFamilyIndices   = indices.data();
            return *s();
        }

        /// @brief Use exclusive sharing (the common single-family case). Clears the index list.
        inline derived_t& set_exclusive_queue_family()
        {
            this->sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
            this->queueFamilyIndexCount = 0;
            this->pQueueFamilyIndices   = nullptr;
            return *s();
        }

        private:
            inline derived_t* s() { return reinterpret_cast<derived_t*>(this); }

    };

    /// @brief VkMemoryRequirements mixin (memory_requirements_t).
    ///
    /// Output struct from vkGetBufferMemoryRequirements: the size and alignment a buffer's backing
    /// allocation needs, plus memoryTypeBits -- a bitmask of which memory types are acceptable, fed
    /// to physical_device_t::find_memory_type to pick a concrete index.
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkMemoryRequirements>
    {

        VkDeviceSize    size            {         };
        VkDeviceSize    alignment       {         };
        uint32_t        memoryTypeBits  {         };

        inline VkDeviceSize get_size() const             { return this->size;           }
        inline VkDeviceSize get_alignment() const        { return this->alignment;      }
        inline uint32_t     get_memory_type_bits() const { return this->memoryTypeBits; }

    };

    /// @brief VkMemoryAllocateInfo mixin (memory_allocate_info_t).
    ///
    /// Input struct for vkAllocateMemory. memoryTypeIndex must be one of the types allowed by the
    /// buffer's memory requirements (see find_memory_type); allocationSize is typically the size from
    /// those requirements (which may exceed the buffer's logical size due to alignment).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkMemoryAllocateInfo>
    {

        VkStructureType     sType           { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        const void*         pNext           { nullptr };
        VkDeviceSize        allocationSize  {         };
        uint32_t            memoryTypeIndex {         };

        inline const void*  get_next() const             { return this->pNext;           }
        inline VkDeviceSize get_allocation_size() const  { return this->allocationSize;  }
        inline uint32_t     get_memory_type_index() const { return this->memoryTypeIndex; }

        inline derived_t& set_next(const void* next)            { this->pNext = next; return *s();             }
        inline derived_t& set_allocation_size(VkDeviceSize s)   { this->allocationSize = s; return *self();    }
        inline derived_t& set_memory_type_index(uint32_t index) { this->memoryTypeIndex = index; return *s();  }

        private:
            inline derived_t* s()    { return reinterpret_cast<derived_t*>(this); }
            inline derived_t* self() { return reinterpret_cast<derived_t*>(this); }

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

    using offset_2d_t                                   = vk_struct_base<VkOffset2D>;
    using rect_2d_t                                     = vk_struct_base<VkRect2D>;
    using viewport_t                                    = vk_struct_base<VkViewport>;
    using vertex_input_binding_description_t            = vk_struct_base<VkVertexInputBindingDescription>;
    using vertex_input_attribute_description_t          = vk_struct_base<VkVertexInputAttributeDescription>;
    using pipeline_vertex_input_state_create_info_t     = vk_struct_base<VkPipelineVertexInputStateCreateInfo>;
    using pipeline_input_assembly_state_create_info_t   = vk_struct_base<VkPipelineInputAssemblyStateCreateInfo>;
    using pipeline_viewport_state_create_info_t         = vk_struct_base<VkPipelineViewportStateCreateInfo>;
    using pipeline_rasterization_state_create_info_t    = vk_struct_base<VkPipelineRasterizationStateCreateInfo>;
    using pipeline_multisample_state_create_info_t      = vk_struct_base<VkPipelineMultisampleStateCreateInfo>;
    using stencil_op_state_t                            = vk_struct_base<VkStencilOpState>;
    using pipeline_depth_stencil_state_create_info_t    = vk_struct_base<VkPipelineDepthStencilStateCreateInfo>;
    using pipeline_color_blend_attachment_state_t       = vk_struct_base<VkPipelineColorBlendAttachmentState>;
    using pipeline_color_blend_state_create_info_t      = vk_struct_base<VkPipelineColorBlendStateCreateInfo>;
    using pipeline_dynamic_state_create_info_t          = vk_struct_base<VkPipelineDynamicStateCreateInfo>;
    using push_constant_range_t                         = vk_struct_base<VkPushConstantRange>;
    using pipeline_layout_create_info_t                 = vk_struct_base<VkPipelineLayoutCreateInfo>;
    using specialization_map_entry_t                    = vk_struct_base<VkSpecializationMapEntry>;
    using specialization_info_t                         = vk_struct_base<VkSpecializationInfo>;
    using pipeline_shader_stage_create_info_t           = vk_struct_base<VkPipelineShaderStageCreateInfo>;
    using pipeline_tessellation_state_create_info_t     = vk_struct_base<VkPipelineTessellationStateCreateInfo>;
    using pipeline_rendering_create_info_t              = vk_struct_base<VkPipelineRenderingCreateInfo>;
    using graphics_pipeline_create_info_t               = vk_struct_base<VkGraphicsPipelineCreateInfo>;

    using command_pool_create_info_t                    = vk_struct_base<VkCommandPoolCreateInfo>;
    using command_buffer_allocate_info_t                = vk_struct_base<VkCommandBufferAllocateInfo>;
    using command_buffer_begin_info_t                   = vk_struct_base<VkCommandBufferBeginInfo>;
    using rendering_attachment_info_t                   = vk_struct_base<VkRenderingAttachmentInfo>;
    using rendering_info_t                              = vk_struct_base<VkRenderingInfo>;
    using semaphore_create_info_t                       = vk_struct_base<VkSemaphoreCreateInfo>;
    using fence_create_info_t                           = vk_struct_base<VkFenceCreateInfo>;
    using submit_info_t                                 = vk_struct_base<VkSubmitInfo>;
    using present_info_t                                = vk_struct_base<VkPresentInfoKHR>;
    using buffer_create_info_t                          = vk_struct_base<VkBufferCreateInfo>;
    using memory_requirements_t                         = vk_struct_base<VkMemoryRequirements>;
    using memory_allocate_info_t                        = vk_struct_base<VkMemoryAllocateInfo>;

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

    // VkOffset2D checks.
    static_assert(std::is_standard_layout_v<offset_2d_t>, "offset_2d_t must be standard-layout for native interop.");
    static_assert(sizeof(offset_2d_t) == sizeof(VkOffset2D), "offset_2d_t layout diverged from VkOffset2D.");
    static_assert(offsetof(offset_2d_t, x) == offsetof(VkOffset2D, x));
    static_assert(offsetof(offset_2d_t, y) == offsetof(VkOffset2D, y));

    // VkRect2D checks.
    static_assert(std::is_standard_layout_v<rect_2d_t>, "rect_2d_t must be standard-layout for native interop.");
    static_assert(sizeof(rect_2d_t) == sizeof(VkRect2D), "rect_2d_t layout diverged from VkRect2D.");
    static_assert(offsetof(rect_2d_t, offset) == offsetof(VkRect2D, offset));
    static_assert(offsetof(rect_2d_t, extent) == offsetof(VkRect2D, extent));

    // VkViewport checks.
    static_assert(std::is_standard_layout_v<viewport_t>, "viewport_t must be standard-layout for native interop.");
    static_assert(sizeof(viewport_t) == sizeof(VkViewport), "viewport_t layout diverged from VkViewport.");
    static_assert(offsetof(viewport_t, x) == offsetof(VkViewport, x));
    static_assert(offsetof(viewport_t, maxDepth) == offsetof(VkViewport, maxDepth));

    // VkVertexInputBindingDescription checks.
    static_assert(std::is_standard_layout_v<vertex_input_binding_description_t>, "vertex_input_binding_description_t must be standard-layout for native interop.");
    static_assert(sizeof(vertex_input_binding_description_t) == sizeof(VkVertexInputBindingDescription), "vertex_input_binding_description_t layout diverged from VkVertexInputBindingDescription.");
    static_assert(offsetof(vertex_input_binding_description_t, binding) == offsetof(VkVertexInputBindingDescription, binding));
    static_assert(offsetof(vertex_input_binding_description_t, inputRate) == offsetof(VkVertexInputBindingDescription, inputRate));

    // VkVertexInputAttributeDescription checks.
    static_assert(std::is_standard_layout_v<vertex_input_attribute_description_t>, "vertex_input_attribute_description_t must be standard-layout for native interop.");
    static_assert(sizeof(vertex_input_attribute_description_t) == sizeof(VkVertexInputAttributeDescription), "vertex_input_attribute_description_t layout diverged from VkVertexInputAttributeDescription.");
    static_assert(offsetof(vertex_input_attribute_description_t, location) == offsetof(VkVertexInputAttributeDescription, location));
    static_assert(offsetof(vertex_input_attribute_description_t, offset) == offsetof(VkVertexInputAttributeDescription, offset));

    // VkPipelineVertexInputStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_vertex_input_state_create_info_t>, "pipeline_vertex_input_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_vertex_input_state_create_info_t) == sizeof(VkPipelineVertexInputStateCreateInfo), "pipeline_vertex_input_state_create_info_t layout diverged from VkPipelineVertexInputStateCreateInfo.");
    static_assert(offsetof(pipeline_vertex_input_state_create_info_t, sType) == offsetof(VkPipelineVertexInputStateCreateInfo, sType));
    static_assert(offsetof(pipeline_vertex_input_state_create_info_t, pNext) == offsetof(VkPipelineVertexInputStateCreateInfo, pNext));
    static_assert(offsetof(pipeline_vertex_input_state_create_info_t, vertexBindingDescriptionCount) == offsetof(VkPipelineVertexInputStateCreateInfo, vertexBindingDescriptionCount));
    static_assert(offsetof(pipeline_vertex_input_state_create_info_t, pVertexBindingDescriptions) == offsetof(VkPipelineVertexInputStateCreateInfo, pVertexBindingDescriptions));
    static_assert(offsetof(pipeline_vertex_input_state_create_info_t, pVertexAttributeDescriptions) == offsetof(VkPipelineVertexInputStateCreateInfo, pVertexAttributeDescriptions));

    // VkPipelineInputAssemblyStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_input_assembly_state_create_info_t>, "pipeline_input_assembly_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_input_assembly_state_create_info_t) == sizeof(VkPipelineInputAssemblyStateCreateInfo), "pipeline_input_assembly_state_create_info_t layout diverged from VkPipelineInputAssemblyStateCreateInfo.");
    static_assert(offsetof(pipeline_input_assembly_state_create_info_t, sType) == offsetof(VkPipelineInputAssemblyStateCreateInfo, sType));
    static_assert(offsetof(pipeline_input_assembly_state_create_info_t, topology) == offsetof(VkPipelineInputAssemblyStateCreateInfo, topology));
    static_assert(offsetof(pipeline_input_assembly_state_create_info_t, primitiveRestartEnable) == offsetof(VkPipelineInputAssemblyStateCreateInfo, primitiveRestartEnable));

    // VkPipelineViewportStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_viewport_state_create_info_t>, "pipeline_viewport_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_viewport_state_create_info_t) == sizeof(VkPipelineViewportStateCreateInfo), "pipeline_viewport_state_create_info_t layout diverged from VkPipelineViewportStateCreateInfo.");
    static_assert(offsetof(pipeline_viewport_state_create_info_t, sType) == offsetof(VkPipelineViewportStateCreateInfo, sType));
    static_assert(offsetof(pipeline_viewport_state_create_info_t, viewportCount) == offsetof(VkPipelineViewportStateCreateInfo, viewportCount));
    static_assert(offsetof(pipeline_viewport_state_create_info_t, pViewports) == offsetof(VkPipelineViewportStateCreateInfo, pViewports));
    static_assert(offsetof(pipeline_viewport_state_create_info_t, scissorCount) == offsetof(VkPipelineViewportStateCreateInfo, scissorCount));
    static_assert(offsetof(pipeline_viewport_state_create_info_t, pScissors) == offsetof(VkPipelineViewportStateCreateInfo, pScissors));

    // VkPipelineRasterizationStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_rasterization_state_create_info_t>, "pipeline_rasterization_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_rasterization_state_create_info_t) == sizeof(VkPipelineRasterizationStateCreateInfo), "pipeline_rasterization_state_create_info_t layout diverged from VkPipelineRasterizationStateCreateInfo.");
    static_assert(offsetof(pipeline_rasterization_state_create_info_t, sType) == offsetof(VkPipelineRasterizationStateCreateInfo, sType));
    static_assert(offsetof(pipeline_rasterization_state_create_info_t, depthClampEnable) == offsetof(VkPipelineRasterizationStateCreateInfo, depthClampEnable));
    static_assert(offsetof(pipeline_rasterization_state_create_info_t, polygonMode) == offsetof(VkPipelineRasterizationStateCreateInfo, polygonMode));
    static_assert(offsetof(pipeline_rasterization_state_create_info_t, lineWidth) == offsetof(VkPipelineRasterizationStateCreateInfo, lineWidth));

    // VkPipelineMultisampleStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_multisample_state_create_info_t>, "pipeline_multisample_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_multisample_state_create_info_t) == sizeof(VkPipelineMultisampleStateCreateInfo), "pipeline_multisample_state_create_info_t layout diverged from VkPipelineMultisampleStateCreateInfo.");
    static_assert(offsetof(pipeline_multisample_state_create_info_t, sType) == offsetof(VkPipelineMultisampleStateCreateInfo, sType));
    static_assert(offsetof(pipeline_multisample_state_create_info_t, rasterizationSamples) == offsetof(VkPipelineMultisampleStateCreateInfo, rasterizationSamples));
    static_assert(offsetof(pipeline_multisample_state_create_info_t, pSampleMask) == offsetof(VkPipelineMultisampleStateCreateInfo, pSampleMask));
    static_assert(offsetof(pipeline_multisample_state_create_info_t, alphaToOneEnable) == offsetof(VkPipelineMultisampleStateCreateInfo, alphaToOneEnable));

    // VkStencilOpState checks.
    static_assert(std::is_standard_layout_v<stencil_op_state_t>, "stencil_op_state_t must be standard-layout for native interop.");
    static_assert(sizeof(stencil_op_state_t) == sizeof(VkStencilOpState), "stencil_op_state_t layout diverged from VkStencilOpState.");
    static_assert(offsetof(stencil_op_state_t, failOp) == offsetof(VkStencilOpState, failOp));
    static_assert(offsetof(stencil_op_state_t, reference) == offsetof(VkStencilOpState, reference));

    // VkPipelineDepthStencilStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_depth_stencil_state_create_info_t>, "pipeline_depth_stencil_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_depth_stencil_state_create_info_t) == sizeof(VkPipelineDepthStencilStateCreateInfo), "pipeline_depth_stencil_state_create_info_t layout diverged from VkPipelineDepthStencilStateCreateInfo.");
    static_assert(offsetof(pipeline_depth_stencil_state_create_info_t, sType) == offsetof(VkPipelineDepthStencilStateCreateInfo, sType));
    static_assert(offsetof(pipeline_depth_stencil_state_create_info_t, depthTestEnable) == offsetof(VkPipelineDepthStencilStateCreateInfo, depthTestEnable));
    static_assert(offsetof(pipeline_depth_stencil_state_create_info_t, front) == offsetof(VkPipelineDepthStencilStateCreateInfo, front));
    static_assert(offsetof(pipeline_depth_stencil_state_create_info_t, back) == offsetof(VkPipelineDepthStencilStateCreateInfo, back));
    static_assert(offsetof(pipeline_depth_stencil_state_create_info_t, maxDepthBounds) == offsetof(VkPipelineDepthStencilStateCreateInfo, maxDepthBounds));

    // VkPipelineColorBlendAttachmentState checks.
    static_assert(std::is_standard_layout_v<pipeline_color_blend_attachment_state_t>, "pipeline_color_blend_attachment_state_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_color_blend_attachment_state_t) == sizeof(VkPipelineColorBlendAttachmentState), "pipeline_color_blend_attachment_state_t layout diverged from VkPipelineColorBlendAttachmentState.");
    static_assert(offsetof(pipeline_color_blend_attachment_state_t, blendEnable) == offsetof(VkPipelineColorBlendAttachmentState, blendEnable));
    static_assert(offsetof(pipeline_color_blend_attachment_state_t, colorWriteMask) == offsetof(VkPipelineColorBlendAttachmentState, colorWriteMask));

    // VkPipelineColorBlendStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_color_blend_state_create_info_t>, "pipeline_color_blend_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_color_blend_state_create_info_t) == sizeof(VkPipelineColorBlendStateCreateInfo), "pipeline_color_blend_state_create_info_t layout diverged from VkPipelineColorBlendStateCreateInfo.");
    static_assert(offsetof(pipeline_color_blend_state_create_info_t, sType) == offsetof(VkPipelineColorBlendStateCreateInfo, sType));
    static_assert(offsetof(pipeline_color_blend_state_create_info_t, logicOpEnable) == offsetof(VkPipelineColorBlendStateCreateInfo, logicOpEnable));
    static_assert(offsetof(pipeline_color_blend_state_create_info_t, attachmentCount) == offsetof(VkPipelineColorBlendStateCreateInfo, attachmentCount));
    static_assert(offsetof(pipeline_color_blend_state_create_info_t, pAttachments) == offsetof(VkPipelineColorBlendStateCreateInfo, pAttachments));
    static_assert(offsetof(pipeline_color_blend_state_create_info_t, blendConstants) == offsetof(VkPipelineColorBlendStateCreateInfo, blendConstants));

    // VkPipelineDynamicStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_dynamic_state_create_info_t>, "pipeline_dynamic_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_dynamic_state_create_info_t) == sizeof(VkPipelineDynamicStateCreateInfo), "pipeline_dynamic_state_create_info_t layout diverged from VkPipelineDynamicStateCreateInfo.");
    static_assert(offsetof(pipeline_dynamic_state_create_info_t, sType) == offsetof(VkPipelineDynamicStateCreateInfo, sType));
    static_assert(offsetof(pipeline_dynamic_state_create_info_t, dynamicStateCount) == offsetof(VkPipelineDynamicStateCreateInfo, dynamicStateCount));
    static_assert(offsetof(pipeline_dynamic_state_create_info_t, pDynamicStates) == offsetof(VkPipelineDynamicStateCreateInfo, pDynamicStates));

    // VkPushConstantRange checks.
    static_assert(std::is_standard_layout_v<push_constant_range_t>, "push_constant_range_t must be standard-layout for native interop.");
    static_assert(sizeof(push_constant_range_t) == sizeof(VkPushConstantRange), "push_constant_range_t layout diverged from VkPushConstantRange.");
    static_assert(offsetof(push_constant_range_t, stageFlags) == offsetof(VkPushConstantRange, stageFlags));
    static_assert(offsetof(push_constant_range_t, size) == offsetof(VkPushConstantRange, size));

    // VkPipelineLayoutCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_layout_create_info_t>, "pipeline_layout_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_layout_create_info_t) == sizeof(VkPipelineLayoutCreateInfo), "pipeline_layout_create_info_t layout diverged from VkPipelineLayoutCreateInfo.");
    static_assert(offsetof(pipeline_layout_create_info_t, sType) == offsetof(VkPipelineLayoutCreateInfo, sType));
    static_assert(offsetof(pipeline_layout_create_info_t, setLayoutCount) == offsetof(VkPipelineLayoutCreateInfo, setLayoutCount));
    static_assert(offsetof(pipeline_layout_create_info_t, pSetLayouts) == offsetof(VkPipelineLayoutCreateInfo, pSetLayouts));
    static_assert(offsetof(pipeline_layout_create_info_t, pushConstantRangeCount) == offsetof(VkPipelineLayoutCreateInfo, pushConstantRangeCount));
    static_assert(offsetof(pipeline_layout_create_info_t, pPushConstantRanges) == offsetof(VkPipelineLayoutCreateInfo, pPushConstantRanges));

    // VkSpecializationMapEntry checks.
    static_assert(std::is_standard_layout_v<specialization_map_entry_t>, "specialization_map_entry_t must be standard-layout for native interop.");
    static_assert(sizeof(specialization_map_entry_t) == sizeof(VkSpecializationMapEntry), "specialization_map_entry_t layout diverged from VkSpecializationMapEntry.");
    static_assert(offsetof(specialization_map_entry_t, constantID) == offsetof(VkSpecializationMapEntry, constantID));
    static_assert(offsetof(specialization_map_entry_t, size) == offsetof(VkSpecializationMapEntry, size));

    // VkSpecializationInfo checks.
    static_assert(std::is_standard_layout_v<specialization_info_t>, "specialization_info_t must be standard-layout for native interop.");
    static_assert(sizeof(specialization_info_t) == sizeof(VkSpecializationInfo), "specialization_info_t layout diverged from VkSpecializationInfo.");
    static_assert(offsetof(specialization_info_t, mapEntryCount) == offsetof(VkSpecializationInfo, mapEntryCount));
    static_assert(offsetof(specialization_info_t, pMapEntries) == offsetof(VkSpecializationInfo, pMapEntries));
    static_assert(offsetof(specialization_info_t, dataSize) == offsetof(VkSpecializationInfo, dataSize));
    static_assert(offsetof(specialization_info_t, pData) == offsetof(VkSpecializationInfo, pData));

    // VkPipelineShaderStageCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_shader_stage_create_info_t>, "pipeline_shader_stage_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_shader_stage_create_info_t) == sizeof(VkPipelineShaderStageCreateInfo), "pipeline_shader_stage_create_info_t layout diverged from VkPipelineShaderStageCreateInfo.");
    static_assert(offsetof(pipeline_shader_stage_create_info_t, sType) == offsetof(VkPipelineShaderStageCreateInfo, sType));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, pNext) == offsetof(VkPipelineShaderStageCreateInfo, pNext));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, flags) == offsetof(VkPipelineShaderStageCreateInfo, flags));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, stage) == offsetof(VkPipelineShaderStageCreateInfo, stage));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, module) == offsetof(VkPipelineShaderStageCreateInfo, module));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, pName) == offsetof(VkPipelineShaderStageCreateInfo, pName));
    static_assert(offsetof(pipeline_shader_stage_create_info_t, pSpecializationInfo) == offsetof(VkPipelineShaderStageCreateInfo, pSpecializationInfo));

    // VkPipelineTessellationStateCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_tessellation_state_create_info_t>, "pipeline_tessellation_state_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_tessellation_state_create_info_t) == sizeof(VkPipelineTessellationStateCreateInfo), "pipeline_tessellation_state_create_info_t layout diverged from VkPipelineTessellationStateCreateInfo.");
    static_assert(offsetof(pipeline_tessellation_state_create_info_t, sType) == offsetof(VkPipelineTessellationStateCreateInfo, sType));
    static_assert(offsetof(pipeline_tessellation_state_create_info_t, pNext) == offsetof(VkPipelineTessellationStateCreateInfo, pNext));
    static_assert(offsetof(pipeline_tessellation_state_create_info_t, patchControlPoints) == offsetof(VkPipelineTessellationStateCreateInfo, patchControlPoints));

    // VkPipelineRenderingCreateInfo checks.
    static_assert(std::is_standard_layout_v<pipeline_rendering_create_info_t>, "pipeline_rendering_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(pipeline_rendering_create_info_t) == sizeof(VkPipelineRenderingCreateInfo), "pipeline_rendering_create_info_t layout diverged from VkPipelineRenderingCreateInfo.");
    static_assert(offsetof(pipeline_rendering_create_info_t, sType) == offsetof(VkPipelineRenderingCreateInfo, sType));
    static_assert(offsetof(pipeline_rendering_create_info_t, pNext) == offsetof(VkPipelineRenderingCreateInfo, pNext));
    static_assert(offsetof(pipeline_rendering_create_info_t, viewMask) == offsetof(VkPipelineRenderingCreateInfo, viewMask));
    static_assert(offsetof(pipeline_rendering_create_info_t, colorAttachmentCount) == offsetof(VkPipelineRenderingCreateInfo, colorAttachmentCount));
    static_assert(offsetof(pipeline_rendering_create_info_t, pColorAttachmentFormats) == offsetof(VkPipelineRenderingCreateInfo, pColorAttachmentFormats));
    static_assert(offsetof(pipeline_rendering_create_info_t, depthAttachmentFormat) == offsetof(VkPipelineRenderingCreateInfo, depthAttachmentFormat));
    static_assert(offsetof(pipeline_rendering_create_info_t, stencilAttachmentFormat) == offsetof(VkPipelineRenderingCreateInfo, stencilAttachmentFormat));

    // VkGraphicsPipelineCreateInfo checks.
    static_assert(std::is_standard_layout_v<graphics_pipeline_create_info_t>, "graphics_pipeline_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(graphics_pipeline_create_info_t) == sizeof(VkGraphicsPipelineCreateInfo), "graphics_pipeline_create_info_t layout diverged from VkGraphicsPipelineCreateInfo.");
    static_assert(offsetof(graphics_pipeline_create_info_t, sType) == offsetof(VkGraphicsPipelineCreateInfo, sType));
    static_assert(offsetof(graphics_pipeline_create_info_t, pNext) == offsetof(VkGraphicsPipelineCreateInfo, pNext));
    static_assert(offsetof(graphics_pipeline_create_info_t, flags) == offsetof(VkGraphicsPipelineCreateInfo, flags));
    static_assert(offsetof(graphics_pipeline_create_info_t, stageCount) == offsetof(VkGraphicsPipelineCreateInfo, stageCount));
    static_assert(offsetof(graphics_pipeline_create_info_t, pStages) == offsetof(VkGraphicsPipelineCreateInfo, pStages));
    static_assert(offsetof(graphics_pipeline_create_info_t, pVertexInputState) == offsetof(VkGraphicsPipelineCreateInfo, pVertexInputState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pInputAssemblyState) == offsetof(VkGraphicsPipelineCreateInfo, pInputAssemblyState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pTessellationState) == offsetof(VkGraphicsPipelineCreateInfo, pTessellationState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pViewportState) == offsetof(VkGraphicsPipelineCreateInfo, pViewportState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pRasterizationState) == offsetof(VkGraphicsPipelineCreateInfo, pRasterizationState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pMultisampleState) == offsetof(VkGraphicsPipelineCreateInfo, pMultisampleState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pDepthStencilState) == offsetof(VkGraphicsPipelineCreateInfo, pDepthStencilState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pColorBlendState) == offsetof(VkGraphicsPipelineCreateInfo, pColorBlendState));
    static_assert(offsetof(graphics_pipeline_create_info_t, pDynamicState) == offsetof(VkGraphicsPipelineCreateInfo, pDynamicState));
    static_assert(offsetof(graphics_pipeline_create_info_t, layout) == offsetof(VkGraphicsPipelineCreateInfo, layout));
    static_assert(offsetof(graphics_pipeline_create_info_t, renderPass) == offsetof(VkGraphicsPipelineCreateInfo, renderPass));
    static_assert(offsetof(graphics_pipeline_create_info_t, subpass) == offsetof(VkGraphicsPipelineCreateInfo, subpass));
    static_assert(offsetof(graphics_pipeline_create_info_t, basePipelineHandle) == offsetof(VkGraphicsPipelineCreateInfo, basePipelineHandle));
    static_assert(offsetof(graphics_pipeline_create_info_t, basePipelineIndex) == offsetof(VkGraphicsPipelineCreateInfo, basePipelineIndex));

    // VkCommandPoolCreateInfo checks.
    static_assert(std::is_standard_layout_v<command_pool_create_info_t>, "command_pool_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(command_pool_create_info_t) == sizeof(VkCommandPoolCreateInfo), "command_pool_create_info_t layout diverged from VkCommandPoolCreateInfo.");
    static_assert(offsetof(command_pool_create_info_t, sType) == offsetof(VkCommandPoolCreateInfo, sType));
    static_assert(offsetof(command_pool_create_info_t, pNext) == offsetof(VkCommandPoolCreateInfo, pNext));
    static_assert(offsetof(command_pool_create_info_t, flags) == offsetof(VkCommandPoolCreateInfo, flags));
    static_assert(offsetof(command_pool_create_info_t, queueFamilyIndex) == offsetof(VkCommandPoolCreateInfo, queueFamilyIndex));

    // VkCommandBufferAllocateInfo checks.
    static_assert(std::is_standard_layout_v<command_buffer_allocate_info_t>, "command_buffer_allocate_info_t must be standard-layout for native interop.");
    static_assert(sizeof(command_buffer_allocate_info_t) == sizeof(VkCommandBufferAllocateInfo), "command_buffer_allocate_info_t layout diverged from VkCommandBufferAllocateInfo.");
    static_assert(offsetof(command_buffer_allocate_info_t, sType) == offsetof(VkCommandBufferAllocateInfo, sType));
    static_assert(offsetof(command_buffer_allocate_info_t, pNext) == offsetof(VkCommandBufferAllocateInfo, pNext));
    static_assert(offsetof(command_buffer_allocate_info_t, commandPool) == offsetof(VkCommandBufferAllocateInfo, commandPool));
    static_assert(offsetof(command_buffer_allocate_info_t, level) == offsetof(VkCommandBufferAllocateInfo, level));
    static_assert(offsetof(command_buffer_allocate_info_t, commandBufferCount) == offsetof(VkCommandBufferAllocateInfo, commandBufferCount));

    // VkCommandBufferBeginInfo checks.
    static_assert(std::is_standard_layout_v<command_buffer_begin_info_t>, "command_buffer_begin_info_t must be standard-layout for native interop.");
    static_assert(sizeof(command_buffer_begin_info_t) == sizeof(VkCommandBufferBeginInfo), "command_buffer_begin_info_t layout diverged from VkCommandBufferBeginInfo.");
    static_assert(offsetof(command_buffer_begin_info_t, sType) == offsetof(VkCommandBufferBeginInfo, sType));
    static_assert(offsetof(command_buffer_begin_info_t, pNext) == offsetof(VkCommandBufferBeginInfo, pNext));
    static_assert(offsetof(command_buffer_begin_info_t, flags) == offsetof(VkCommandBufferBeginInfo, flags));
    static_assert(offsetof(command_buffer_begin_info_t, pInheritanceInfo) == offsetof(VkCommandBufferBeginInfo, pInheritanceInfo));

    // VkRenderingAttachmentInfo checks.
    static_assert(std::is_standard_layout_v<rendering_attachment_info_t>, "rendering_attachment_info_t must be standard-layout for native interop.");
    static_assert(sizeof(rendering_attachment_info_t) == sizeof(VkRenderingAttachmentInfo), "rendering_attachment_info_t layout diverged from VkRenderingAttachmentInfo.");
    static_assert(offsetof(rendering_attachment_info_t, sType) == offsetof(VkRenderingAttachmentInfo, sType));
    static_assert(offsetof(rendering_attachment_info_t, pNext) == offsetof(VkRenderingAttachmentInfo, pNext));
    static_assert(offsetof(rendering_attachment_info_t, imageView) == offsetof(VkRenderingAttachmentInfo, imageView));
    static_assert(offsetof(rendering_attachment_info_t, imageLayout) == offsetof(VkRenderingAttachmentInfo, imageLayout));
    static_assert(offsetof(rendering_attachment_info_t, resolveMode) == offsetof(VkRenderingAttachmentInfo, resolveMode));
    static_assert(offsetof(rendering_attachment_info_t, resolveImageView) == offsetof(VkRenderingAttachmentInfo, resolveImageView));
    static_assert(offsetof(rendering_attachment_info_t, resolveImageLayout) == offsetof(VkRenderingAttachmentInfo, resolveImageLayout));
    static_assert(offsetof(rendering_attachment_info_t, loadOp) == offsetof(VkRenderingAttachmentInfo, loadOp));
    static_assert(offsetof(rendering_attachment_info_t, storeOp) == offsetof(VkRenderingAttachmentInfo, storeOp));
    static_assert(offsetof(rendering_attachment_info_t, clearValue) == offsetof(VkRenderingAttachmentInfo, clearValue));

    // VkRenderingInfo checks.
    static_assert(std::is_standard_layout_v<rendering_info_t>, "rendering_info_t must be standard-layout for native interop.");
    static_assert(sizeof(rendering_info_t) == sizeof(VkRenderingInfo), "rendering_info_t layout diverged from VkRenderingInfo.");
    static_assert(offsetof(rendering_info_t, sType) == offsetof(VkRenderingInfo, sType));
    static_assert(offsetof(rendering_info_t, pNext) == offsetof(VkRenderingInfo, pNext));
    static_assert(offsetof(rendering_info_t, flags) == offsetof(VkRenderingInfo, flags));
    static_assert(offsetof(rendering_info_t, renderArea) == offsetof(VkRenderingInfo, renderArea));
    static_assert(offsetof(rendering_info_t, layerCount) == offsetof(VkRenderingInfo, layerCount));
    static_assert(offsetof(rendering_info_t, viewMask) == offsetof(VkRenderingInfo, viewMask));
    static_assert(offsetof(rendering_info_t, colorAttachmentCount) == offsetof(VkRenderingInfo, colorAttachmentCount));
    static_assert(offsetof(rendering_info_t, pColorAttachments) == offsetof(VkRenderingInfo, pColorAttachments));
    static_assert(offsetof(rendering_info_t, pDepthAttachment) == offsetof(VkRenderingInfo, pDepthAttachment));
    static_assert(offsetof(rendering_info_t, pStencilAttachment) == offsetof(VkRenderingInfo, pStencilAttachment));

    // VkSemaphoreCreateInfo checks.
    static_assert(std::is_standard_layout_v<semaphore_create_info_t>, "semaphore_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(semaphore_create_info_t) == sizeof(VkSemaphoreCreateInfo), "semaphore_create_info_t layout diverged from VkSemaphoreCreateInfo.");
    static_assert(offsetof(semaphore_create_info_t, sType) == offsetof(VkSemaphoreCreateInfo, sType));
    static_assert(offsetof(semaphore_create_info_t, pNext) == offsetof(VkSemaphoreCreateInfo, pNext));
    static_assert(offsetof(semaphore_create_info_t, flags) == offsetof(VkSemaphoreCreateInfo, flags));

    // VkFenceCreateInfo checks.
    static_assert(std::is_standard_layout_v<fence_create_info_t>, "fence_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(fence_create_info_t) == sizeof(VkFenceCreateInfo), "fence_create_info_t layout diverged from VkFenceCreateInfo.");
    static_assert(offsetof(fence_create_info_t, sType) == offsetof(VkFenceCreateInfo, sType));
    static_assert(offsetof(fence_create_info_t, pNext) == offsetof(VkFenceCreateInfo, pNext));
    static_assert(offsetof(fence_create_info_t, flags) == offsetof(VkFenceCreateInfo, flags));

    // VkSubmitInfo checks.
    static_assert(std::is_standard_layout_v<submit_info_t>, "submit_info_t must be standard-layout for native interop.");
    static_assert(sizeof(submit_info_t) == sizeof(VkSubmitInfo), "submit_info_t layout diverged from VkSubmitInfo.");
    static_assert(offsetof(submit_info_t, sType) == offsetof(VkSubmitInfo, sType));
    static_assert(offsetof(submit_info_t, pNext) == offsetof(VkSubmitInfo, pNext));
    static_assert(offsetof(submit_info_t, waitSemaphoreCount) == offsetof(VkSubmitInfo, waitSemaphoreCount));
    static_assert(offsetof(submit_info_t, pWaitSemaphores) == offsetof(VkSubmitInfo, pWaitSemaphores));
    static_assert(offsetof(submit_info_t, pWaitDstStageMask) == offsetof(VkSubmitInfo, pWaitDstStageMask));
    static_assert(offsetof(submit_info_t, commandBufferCount) == offsetof(VkSubmitInfo, commandBufferCount));
    static_assert(offsetof(submit_info_t, pCommandBuffers) == offsetof(VkSubmitInfo, pCommandBuffers));
    static_assert(offsetof(submit_info_t, signalSemaphoreCount) == offsetof(VkSubmitInfo, signalSemaphoreCount));
    static_assert(offsetof(submit_info_t, pSignalSemaphores) == offsetof(VkSubmitInfo, pSignalSemaphores));

    // VkPresentInfoKHR checks.
    static_assert(std::is_standard_layout_v<present_info_t>, "present_info_t must be standard-layout for native interop.");
    static_assert(sizeof(present_info_t) == sizeof(VkPresentInfoKHR), "present_info_t layout diverged from VkPresentInfoKHR.");
    static_assert(offsetof(present_info_t, sType) == offsetof(VkPresentInfoKHR, sType));
    static_assert(offsetof(present_info_t, pNext) == offsetof(VkPresentInfoKHR, pNext));
    static_assert(offsetof(present_info_t, waitSemaphoreCount) == offsetof(VkPresentInfoKHR, waitSemaphoreCount));
    static_assert(offsetof(present_info_t, pWaitSemaphores) == offsetof(VkPresentInfoKHR, pWaitSemaphores));
    static_assert(offsetof(present_info_t, swapchainCount) == offsetof(VkPresentInfoKHR, swapchainCount));
    static_assert(offsetof(present_info_t, pSwapchains) == offsetof(VkPresentInfoKHR, pSwapchains));
    static_assert(offsetof(present_info_t, pImageIndices) == offsetof(VkPresentInfoKHR, pImageIndices));
    static_assert(offsetof(present_info_t, pResults) == offsetof(VkPresentInfoKHR, pResults));

    // VkBufferCreateInfo checks.
    static_assert(std::is_standard_layout_v<buffer_create_info_t>, "buffer_create_info_t must be standard-layout for native interop.");
    static_assert(sizeof(buffer_create_info_t) == sizeof(VkBufferCreateInfo), "buffer_create_info_t layout diverged from VkBufferCreateInfo.");
    static_assert(offsetof(buffer_create_info_t, sType) == offsetof(VkBufferCreateInfo, sType));
    static_assert(offsetof(buffer_create_info_t, pNext) == offsetof(VkBufferCreateInfo, pNext));
    static_assert(offsetof(buffer_create_info_t, flags) == offsetof(VkBufferCreateInfo, flags));
    static_assert(offsetof(buffer_create_info_t, size) == offsetof(VkBufferCreateInfo, size));
    static_assert(offsetof(buffer_create_info_t, usage) == offsetof(VkBufferCreateInfo, usage));
    static_assert(offsetof(buffer_create_info_t, sharingMode) == offsetof(VkBufferCreateInfo, sharingMode));
    static_assert(offsetof(buffer_create_info_t, queueFamilyIndexCount) == offsetof(VkBufferCreateInfo, queueFamilyIndexCount));
    static_assert(offsetof(buffer_create_info_t, pQueueFamilyIndices) == offsetof(VkBufferCreateInfo, pQueueFamilyIndices));

    // VkMemoryRequirements checks.
    static_assert(std::is_standard_layout_v<memory_requirements_t>, "memory_requirements_t must be standard-layout for native interop.");
    static_assert(sizeof(memory_requirements_t) == sizeof(VkMemoryRequirements), "memory_requirements_t layout diverged from VkMemoryRequirements.");
    static_assert(offsetof(memory_requirements_t, size) == offsetof(VkMemoryRequirements, size));
    static_assert(offsetof(memory_requirements_t, alignment) == offsetof(VkMemoryRequirements, alignment));
    static_assert(offsetof(memory_requirements_t, memoryTypeBits) == offsetof(VkMemoryRequirements, memoryTypeBits));

    // VkMemoryAllocateInfo checks.
    static_assert(std::is_standard_layout_v<memory_allocate_info_t>, "memory_allocate_info_t must be standard-layout for native interop.");
    static_assert(sizeof(memory_allocate_info_t) == sizeof(VkMemoryAllocateInfo), "memory_allocate_info_t layout diverged from VkMemoryAllocateInfo.");
    static_assert(offsetof(memory_allocate_info_t, sType) == offsetof(VkMemoryAllocateInfo, sType));
    static_assert(offsetof(memory_allocate_info_t, pNext) == offsetof(VkMemoryAllocateInfo, pNext));
    static_assert(offsetof(memory_allocate_info_t, allocationSize) == offsetof(VkMemoryAllocateInfo, allocationSize));
    static_assert(offsetof(memory_allocate_info_t, memoryTypeIndex) == offsetof(VkMemoryAllocateInfo, memoryTypeIndex));


}
