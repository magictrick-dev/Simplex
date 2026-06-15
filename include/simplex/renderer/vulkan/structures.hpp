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

    /// @brief VkInstanceCreateInfo mixin.
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

    /// @brief VkPhysicalDeviceProperties mixin (physical_device_properties).
    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkPhysicalDeviceProperties>
    {

        uint32_t                            apiVersion                                      {         };
        uint32_t                            driverVersion                                   {         };
        uint32_t                            vendorID                                        {         };
        uint32_t                            deviceID                                        {         };
        VkPhysicalDeviceType                deviceType                                      {         };
        char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]    {         };
        uint8_t                             pipelineCacheUUID[VK_UUID_SIZE]                 {         };
        VkPhysicalDeviceLimits              limits                                          {         };
        VkPhysicalDeviceSparseProperties    sparseProperties                                {         };

        inline uint32_t                                 get_api_version() const         { return this->apiVersion;          }
        inline uint32_t                                 get_driver_version() const      { return this->driverVersion;       }
        inline uint32_t                                 get_vendor_id() const           { return this->vendorID;            }
        inline uint32_t                                 get_device_id() const           { return this->deviceID;            }
        inline VkPhysicalDeviceType                     get_device_type() const         { return this->deviceType;          }
        inline const char*                              get_device_name() const         { return this->deviceName;          }
        inline const VkPhysicalDeviceLimits&            get_limits() const              { return this->limits;              }
        inline const VkPhysicalDeviceSparseProperties&  get_sparse_properties() const   { return this->sparseProperties;    }

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
        VkConformanceVersion                conformanceVersion                                      {         };
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

    using application_info      = vk_struct_base<VkApplicationInfo>;
    using instance_create_info  = vk_struct_base<VkInstanceCreateInfo>;

    using physical_device_properties    = vk_struct_base<VkPhysicalDeviceProperties>;
    using physical_device_10_properties = vk_struct_base<VkPhysicalDeviceProperties2>;
    using physical_device_11_properties = vk_struct_base<VkPhysicalDeviceVulkan11Properties>;
    using physical_device_12_properties = vk_struct_base<VkPhysicalDeviceVulkan12Properties>;
    using physical_device_13_properties = vk_struct_base<VkPhysicalDeviceVulkan13Properties>;
    using physical_device_14_properties = vk_struct_base<VkPhysicalDeviceVulkan14Properties>;

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

    // Physical device property checks. The version-specific structs are large, so beyond sType/pNext
    // these probe the first and last data members: any reorder, retype, or SDK bump shifts sizeof or
    // the trailing offset and trips a guard.

    // VkPhysicalDeviceProperties checks.
    static_assert(std::is_standard_layout_v<physical_device_properties>, "physical_device_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_properties) == sizeof(VkPhysicalDeviceProperties), "physical_device_properties layout diverged from VkPhysicalDeviceProperties.");
    static_assert(offsetof(physical_device_properties, apiVersion) == offsetof(VkPhysicalDeviceProperties, apiVersion));
    static_assert(offsetof(physical_device_properties, deviceType) == offsetof(VkPhysicalDeviceProperties, deviceType));
    static_assert(offsetof(physical_device_properties, deviceName) == offsetof(VkPhysicalDeviceProperties, deviceName));
    static_assert(offsetof(physical_device_properties, pipelineCacheUUID) == offsetof(VkPhysicalDeviceProperties, pipelineCacheUUID));
    static_assert(offsetof(physical_device_properties, limits) == offsetof(VkPhysicalDeviceProperties, limits));
    static_assert(offsetof(physical_device_properties, sparseProperties) == offsetof(VkPhysicalDeviceProperties, sparseProperties));

    // VkPhysicalDeviceProperties2 checks.
    static_assert(std::is_standard_layout_v<physical_device_10_properties>, "physical_device_10_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_10_properties) == sizeof(VkPhysicalDeviceProperties2), "physical_device_10_properties layout diverged from VkPhysicalDeviceProperties2.");
    static_assert(offsetof(physical_device_10_properties, sType) == offsetof(VkPhysicalDeviceProperties2, sType));
    static_assert(offsetof(physical_device_10_properties, pNext) == offsetof(VkPhysicalDeviceProperties2, pNext));
    static_assert(offsetof(physical_device_10_properties, properties) == offsetof(VkPhysicalDeviceProperties2, properties));

    // VkPhysicalDeviceVulkan11Properties checks.
    static_assert(std::is_standard_layout_v<physical_device_11_properties>, "physical_device_11_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_11_properties) == sizeof(VkPhysicalDeviceVulkan11Properties), "physical_device_11_properties layout diverged from VkPhysicalDeviceVulkan11Properties.");
    static_assert(offsetof(physical_device_11_properties, sType) == offsetof(VkPhysicalDeviceVulkan11Properties, sType));
    static_assert(offsetof(physical_device_11_properties, pNext) == offsetof(VkPhysicalDeviceVulkan11Properties, pNext));
    static_assert(offsetof(physical_device_11_properties, deviceUUID) == offsetof(VkPhysicalDeviceVulkan11Properties, deviceUUID));
    static_assert(offsetof(physical_device_11_properties, maxMemoryAllocationSize) == offsetof(VkPhysicalDeviceVulkan11Properties, maxMemoryAllocationSize));

    // VkPhysicalDeviceVulkan12Properties checks.
    static_assert(std::is_standard_layout_v<physical_device_12_properties>, "physical_device_12_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_12_properties) == sizeof(VkPhysicalDeviceVulkan12Properties), "physical_device_12_properties layout diverged from VkPhysicalDeviceVulkan12Properties.");
    static_assert(offsetof(physical_device_12_properties, sType) == offsetof(VkPhysicalDeviceVulkan12Properties, sType));
    static_assert(offsetof(physical_device_12_properties, pNext) == offsetof(VkPhysicalDeviceVulkan12Properties, pNext));
    static_assert(offsetof(physical_device_12_properties, driverID) == offsetof(VkPhysicalDeviceVulkan12Properties, driverID));
    static_assert(offsetof(physical_device_12_properties, framebufferIntegerColorSampleCounts) == offsetof(VkPhysicalDeviceVulkan12Properties, framebufferIntegerColorSampleCounts));

    // VkPhysicalDeviceVulkan13Properties checks.
    static_assert(std::is_standard_layout_v<physical_device_13_properties>, "physical_device_13_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_13_properties) == sizeof(VkPhysicalDeviceVulkan13Properties), "physical_device_13_properties layout diverged from VkPhysicalDeviceVulkan13Properties.");
    static_assert(offsetof(physical_device_13_properties, sType) == offsetof(VkPhysicalDeviceVulkan13Properties, sType));
    static_assert(offsetof(physical_device_13_properties, pNext) == offsetof(VkPhysicalDeviceVulkan13Properties, pNext));
    static_assert(offsetof(physical_device_13_properties, minSubgroupSize) == offsetof(VkPhysicalDeviceVulkan13Properties, minSubgroupSize));
    static_assert(offsetof(physical_device_13_properties, maxBufferSize) == offsetof(VkPhysicalDeviceVulkan13Properties, maxBufferSize));

    // VkPhysicalDeviceVulkan14Properties checks.
    static_assert(std::is_standard_layout_v<physical_device_14_properties>, "physical_device_14_properties must be standard-layout for native interop.");
    static_assert(sizeof(physical_device_14_properties) == sizeof(VkPhysicalDeviceVulkan14Properties), "physical_device_14_properties layout diverged from VkPhysicalDeviceVulkan14Properties.");
    static_assert(offsetof(physical_device_14_properties, sType) == offsetof(VkPhysicalDeviceVulkan14Properties, sType));
    static_assert(offsetof(physical_device_14_properties, pNext) == offsetof(VkPhysicalDeviceVulkan14Properties, pNext));
    static_assert(offsetof(physical_device_14_properties, lineSubPixelPrecisionBits) == offsetof(VkPhysicalDeviceVulkan14Properties, lineSubPixelPrecisionBits));
    static_assert(offsetof(physical_device_14_properties, identicalMemoryTypeRequirements) == offsetof(VkPhysicalDeviceVulkan14Properties, identicalMemoryTypeRequirements));


}
