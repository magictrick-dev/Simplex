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

    // ---------------------------------------------------------------------------------------------
    // Using statements.
    //
    // Cleans up the template syntax and normalizes it to a friendlier to type
    // variation.
    // ---------------------------------------------------------------------------------------------

    using application_info_t                    = vk_struct_base<VkApplicationInfo>;
    using instance_create_info_t                = vk_struct_base<VkInstanceCreateInfo>;

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


}
