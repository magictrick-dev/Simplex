#pragma once
#include <vulkan/vulkan.h>

#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>


#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/queue_family.hpp>

namespace spx::vk
{

    class physical_device 
    {

        public:
            inline  physical_device() = default;
            inline ~physical_device() = default;

            inline 
            physical_device(VkInstance vulkan_instance, VkPhysicalDevice physical_device) 
                : device(physical_device)
            {


                // Gets the physical device properties.
                vkGetPhysicalDeviceProperties(this->device, &this->device_properties_1);
                vkGetPhysicalDeviceMemoryProperties(this->device, &this->memory_properties);

                this->driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
                this->device_properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                this->device_properties_2.pNext = &this->driver_properties;

                vkGetPhysicalDeviceProperties2(this->device, &this->device_properties_2);

                // Get the physical device features available.
                this->device_10_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                this->device_11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                this->device_12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                this->device_13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                this->device_14_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;

                // NOTE(Chris): We assume device is Vulkan 1.4 compatible here, but we will need to
                //              eventually write code that determine compatibility.
                this->device_10_features.pNext = &this->device_11_features;
                this->device_11_features.pNext = &this->device_12_features;
                this->device_12_features.pNext = &this->device_13_features;
                this->device_13_features.pNext = &this->device_14_features;
                this->device_14_features.pNext = NULL;

                vkGetPhysicalDeviceFeatures2(this->device, &this->device_10_features);

                // NOTE(Chris): Print what features are available for each struct here.
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "robustBufferAccess", this->device_10_features.features.robustBufferAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "fullDrawIndexUint32", this->device_10_features.features.fullDrawIndexUint32);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "imageCubeArray", this->device_10_features.features.imageCubeArray);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "independentBlend", this->device_10_features.features.independentBlend);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "geometryShader", this->device_10_features.features.geometryShader);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "tessellationShader", this->device_10_features.features.tessellationShader);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sampleRateShading", this->device_10_features.features.sampleRateShading);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "dualSrcBlend", this->device_10_features.features.dualSrcBlend);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "logicOp", this->device_10_features.features.logicOp);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "multiDrawIndirect", this->device_10_features.features.multiDrawIndirect);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "drawIndirectFirstInstance", this->device_10_features.features.drawIndirectFirstInstance);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "depthClamp", this->device_10_features.features.depthClamp);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "depthBiasClamp", this->device_10_features.features.depthBiasClamp);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "fillModeNonSolid", this->device_10_features.features.fillModeNonSolid);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "depthBounds", this->device_10_features.features.depthBounds);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "wideLines", this->device_10_features.features.wideLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "largePoints", this->device_10_features.features.largePoints);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "alphaToOne", this->device_10_features.features.alphaToOne);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "multiViewport", this->device_10_features.features.multiViewport);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "samplerAnisotropy", this->device_10_features.features.samplerAnisotropy);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "textureCompressionETC2", this->device_10_features.features.textureCompressionETC2);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "textureCompressionASTC_LDR", this->device_10_features.features.textureCompressionASTC_LDR);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "textureCompressionBC", this->device_10_features.features.textureCompressionBC);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "occlusionQueryPrecise", this->device_10_features.features.occlusionQueryPrecise);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "pipelineStatisticsQuery", this->device_10_features.features.pipelineStatisticsQuery);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "vertexPipelineStoresAndAtomics", this->device_10_features.features.vertexPipelineStoresAndAtomics);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "fragmentStoresAndAtomics", this->device_10_features.features.fragmentStoresAndAtomics);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderTessellationAndGeometryPointSize", this->device_10_features.features.shaderTessellationAndGeometryPointSize);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderImageGatherExtended", this->device_10_features.features.shaderImageGatherExtended);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageImageExtendedFormats", this->device_10_features.features.shaderStorageImageExtendedFormats);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageImageMultisample", this->device_10_features.features.shaderStorageImageMultisample);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageImageReadWithoutFormat", this->device_10_features.features.shaderStorageImageReadWithoutFormat);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageImageWriteWithoutFormat", this->device_10_features.features.shaderStorageImageWriteWithoutFormat);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderUniformBufferArrayDynamicIndexing", this->device_10_features.features.shaderUniformBufferArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderSampledImageArrayDynamicIndexing", this->device_10_features.features.shaderSampledImageArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageBufferArrayDynamicIndexing", this->device_10_features.features.shaderStorageBufferArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderStorageImageArrayDynamicIndexing", this->device_10_features.features.shaderStorageImageArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderClipDistance", this->device_10_features.features.shaderClipDistance);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderCullDistance", this->device_10_features.features.shaderCullDistance);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderFloat64", this->device_10_features.features.shaderFloat64);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderInt64", this->device_10_features.features.shaderInt64);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderInt16", this->device_10_features.features.shaderInt16);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderResourceResidency", this->device_10_features.features.shaderResourceResidency);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "shaderResourceMinLod", this->device_10_features.features.shaderResourceMinLod);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseBinding", this->device_10_features.features.sparseBinding);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidencyBuffer", this->device_10_features.features.sparseResidencyBuffer);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidencyImage2D", this->device_10_features.features.sparseResidencyImage2D);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidencyImage3D", this->device_10_features.features.sparseResidencyImage3D);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidency2Samples", this->device_10_features.features.sparseResidency2Samples);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidency4Samples", this->device_10_features.features.sparseResidency4Samples);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidency8Samples", this->device_10_features.features.sparseResidency8Samples);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidency16Samples", this->device_10_features.features.sparseResidency16Samples);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "sparseResidencyAliased", this->device_10_features.features.sparseResidencyAliased);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "variableMultisampleRate", this->device_10_features.features.variableMultisampleRate);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.0 Feature: {} : {}", "inheritedQueries", this->device_10_features.features.inheritedQueries);
                spx::logger::process_message_queue(); // Quickly dump the available features.

                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "storageBuffer16BitAccess", this->device_11_features.storageBuffer16BitAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "uniformAndStorageBuffer16BitAccess", this->device_11_features.uniformAndStorageBuffer16BitAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "storagePushConstant16", this->device_11_features.storagePushConstant16);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "storageInputOutput16", this->device_11_features.storageInputOutput16);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "multiview", this->device_11_features.multiview);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "multiviewGeometryShader", this->device_11_features.multiviewGeometryShader);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "multiviewTessellationShader", this->device_11_features.multiviewTessellationShader);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "variablePointersStorageBuffer", this->device_11_features.variablePointersStorageBuffer);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "variablePointers", this->device_11_features.variablePointers);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "protectedMemory", this->device_11_features.protectedMemory);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "samplerYcbcrConversion", this->device_11_features.samplerYcbcrConversion);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.1 Feature: {} : {}", "shaderDrawParameters", this->device_11_features.shaderDrawParameters);
                spx::logger::process_message_queue(); // Quickly dump the available features.

                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "samplerMirrorClampToEdge", this->device_12_features.samplerMirrorClampToEdge);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "drawIndirectCount", this->device_12_features.drawIndirectCount);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "storageBuffer8BitAccess", this->device_12_features.storageBuffer8BitAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "uniformAndStorageBuffer8BitAccess", this->device_12_features.uniformAndStorageBuffer8BitAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "storagePushConstant8", this->device_12_features.storagePushConstant8);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderBufferInt64Atomics", this->device_12_features.shaderBufferInt64Atomics);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderSharedInt64Atomics", this->device_12_features.shaderSharedInt64Atomics);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderFloat16", this->device_12_features.shaderFloat16);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderInt8", this->device_12_features.shaderInt8);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorIndexing", this->device_12_features.descriptorIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderInputAttachmentArrayDynamicIndexing", this->device_12_features.shaderInputAttachmentArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderUniformTexelBufferArrayDynamicIndexing", this->device_12_features.shaderUniformTexelBufferArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderStorageTexelBufferArrayDynamicIndexing", this->device_12_features.shaderStorageTexelBufferArrayDynamicIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderUniformBufferArrayNonUniformIndexing", this->device_12_features.shaderUniformBufferArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderSampledImageArrayNonUniformIndexing", this->device_12_features.shaderSampledImageArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderStorageBufferArrayNonUniformIndexing", this->device_12_features.shaderStorageBufferArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderStorageImageArrayNonUniformIndexing", this->device_12_features.shaderStorageImageArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderInputAttachmentArrayNonUniformIndexing", this->device_12_features.shaderInputAttachmentArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderUniformTexelBufferArrayNonUniformIndexing", this->device_12_features.shaderUniformTexelBufferArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderStorageTexelBufferArrayNonUniformIndexing", this->device_12_features.shaderStorageTexelBufferArrayNonUniformIndexing);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingUniformBufferUpdateAfterBind", this->device_12_features.descriptorBindingUniformBufferUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingSampledImageUpdateAfterBind", this->device_12_features.descriptorBindingSampledImageUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingStorageImageUpdateAfterBind", this->device_12_features.descriptorBindingStorageImageUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingStorageBufferUpdateAfterBind", this->device_12_features.descriptorBindingStorageBufferUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingUniformTexelBufferUpdateAfterBind", this->device_12_features.descriptorBindingUniformTexelBufferUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingStorageTexelBufferUpdateAfterBind", this->device_12_features.descriptorBindingStorageTexelBufferUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingUpdateUnusedWhilePending", this->device_12_features.descriptorBindingUpdateUnusedWhilePending);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingPartiallyBound", this->device_12_features.descriptorBindingPartiallyBound);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "descriptorBindingVariableDescriptorCount", this->device_12_features.descriptorBindingVariableDescriptorCount);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "runtimeDescriptorArray", this->device_12_features.runtimeDescriptorArray);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "samplerFilterMinmax", this->device_12_features.samplerFilterMinmax);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "scalarBlockLayout", this->device_12_features.scalarBlockLayout);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "imagelessFramebuffer", this->device_12_features.imagelessFramebuffer);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "uniformBufferStandardLayout", this->device_12_features.uniformBufferStandardLayout);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderSubgroupExtendedTypes", this->device_12_features.shaderSubgroupExtendedTypes);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "separateDepthStencilLayouts", this->device_12_features.separateDepthStencilLayouts);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "hostQueryReset", this->device_12_features.hostQueryReset);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "timelineSemaphore", this->device_12_features.timelineSemaphore);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "bufferDeviceAddress", this->device_12_features.bufferDeviceAddress);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "bufferDeviceAddressCaptureReplay", this->device_12_features.bufferDeviceAddressCaptureReplay);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "bufferDeviceAddressMultiDevice", this->device_12_features.bufferDeviceAddressMultiDevice);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "vulkanMemoryModel", this->device_12_features.vulkanMemoryModel);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "vulkanMemoryModelDeviceScope", this->device_12_features.vulkanMemoryModelDeviceScope);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "vulkanMemoryModelAvailabilityVisibilityChains", this->device_12_features.vulkanMemoryModelAvailabilityVisibilityChains);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderOutputViewportIndex", this->device_12_features.shaderOutputViewportIndex);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "shaderOutputLayer", this->device_12_features.shaderOutputLayer);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.2 Feature: {} : {}", "subgroupBroadcastDynamicId", this->device_12_features.subgroupBroadcastDynamicId);
                spx::logger::process_message_queue(); // Quickly dump the available features.

                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "robustImageAccess", this->device_13_features.robustImageAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "inlineUniformBlock", this->device_13_features.inlineUniformBlock);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "descriptorBindingInlineUniformBlockUpdateAfterBind", this->device_13_features.descriptorBindingInlineUniformBlockUpdateAfterBind);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "pipelineCreationCacheControl", this->device_13_features.pipelineCreationCacheControl);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "privateData", this->device_13_features.privateData);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "shaderDemoteToHelperInvocation", this->device_13_features.shaderDemoteToHelperInvocation);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "shaderTerminateInvocation", this->device_13_features.shaderTerminateInvocation);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "subgroupSizeControl", this->device_13_features.subgroupSizeControl);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "computeFullSubgroups", this->device_13_features.computeFullSubgroups);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "synchronization2", this->device_13_features.synchronization2);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "textureCompressionASTC_HDR", this->device_13_features.textureCompressionASTC_HDR);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "shaderZeroInitializeWorkgroupMemory", this->device_13_features.shaderZeroInitializeWorkgroupMemory);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "dynamicRendering", this->device_13_features.dynamicRendering);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "shaderIntegerDotProduct", this->device_13_features.shaderIntegerDotProduct);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.3 Feature: {} : {}", "maintenance4", this->device_13_features.maintenance4);
                spx::logger::process_message_queue(); // Quickly dump the available features.

                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "globalPriorityQuery", this->device_14_features.globalPriorityQuery);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "shaderSubgroupRotate", this->device_14_features.shaderSubgroupRotate);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "shaderSubgroupRotateClustered", this->device_14_features.shaderSubgroupRotateClustered);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "shaderFloatControls2", this->device_14_features.shaderFloatControls2);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "shaderExpectAssume", this->device_14_features.shaderExpectAssume);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "rectangularLines", this->device_14_features.rectangularLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "bresenhamLines", this->device_14_features.bresenhamLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "smoothLines", this->device_14_features.smoothLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "stippledRectangularLines", this->device_14_features.stippledRectangularLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "stippledBresenhamLines", this->device_14_features.stippledBresenhamLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "stippledSmoothLines", this->device_14_features.stippledSmoothLines);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "vertexAttributeInstanceRateDivisor", this->device_14_features.vertexAttributeInstanceRateDivisor);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "vertexAttributeInstanceRateZeroDivisor", this->device_14_features.vertexAttributeInstanceRateZeroDivisor);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "indexTypeUint8", this->device_14_features.indexTypeUint8);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "dynamicRenderingLocalRead", this->device_14_features.dynamicRenderingLocalRead);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "maintenance5", this->device_14_features.maintenance5);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "maintenance6", this->device_14_features.maintenance6);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "pipelineProtectedAccess", this->device_14_features.pipelineProtectedAccess);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "pipelineRobustness", this->device_14_features.pipelineRobustness);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "hostImageCopy", this->device_14_features.hostImageCopy);
                spx::logger::dispatch_diagnostic_log("Vulkan 1.4 Feature: {} : {}", "pushDescriptor", this->device_14_features.pushDescriptor);
                spx::logger::process_message_queue(); // Quickly dump the available features.

                // Gets the queue families and stores them in a neat structure.
                uint32_t queue_families_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, NULL);

                spx::dynamic_array<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, queue_family_properties.begin());

                for (size_t i = 0; i < queue_family_properties.size(); ++i)
                {
                    this->queue_families.emplace_back(i, queue_family_properties[i]);
                }

            }

            inline spx::string_view<char> 
            get_device_name() const
            {
                return this->device_properties_1.deviceName;
            }

            inline spx::string_view<char>
            get_driver_name() const
            {
                return this->driver_properties.driverName;
            }

            inline spx::string_view<char>
            get_driver_version() const
            {
                return this->driver_properties.driverInfo;
            }

            inline spx::dynamic_string<char>
            get_qualified_name() const
            {
                spx::dynamic_string<char> result;
                result += this->get_device_name();
                result += " ";
                result += this->get_driver_name();
                result += " ";
                result += this->get_driver_version();
                return std::move(result);
            }

            inline int64_t
            get_device_score() const
            {
                
                // NOTE(Chris): We will need some sort of mechanism to score the GPU
                //              on the system. The easiest way to score a device is just
                //              look at the device's memory. We will use that for now.
                int64_t device_score = 0;
                device_score += memory_properties.memoryHeaps[0].size;
                
                return device_score;
            }

            inline operator VkPhysicalDevice() 
            { 
                return this->device; 
            }

            inline uint32_t
            get_queue_family_index_with(VkQueueFlags flags) const
            {

                for (const auto& family : this->queue_families)
                {
                    if (family.has_flags(flags))
                    {
                        return family.index;
                    }
                }

                throw std::runtime_error("Failed to find queue family with required flags.");
                return 0;

            }



        public:
            static inline spx::array_view<spx::vk::physical_device>
            get_physical_devices(spx::vk::instance instance)
            {

                static bool initialized = false;
                static spx::dynamic_array<spx::vk::physical_device> physical_devices;
                if (initialized == true) return physical_devices;
                initialized = true;

                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(instance, &device_count, NULL);

                spx::dynamic_array<VkPhysicalDevice> devices(device_count);
                vkEnumeratePhysicalDevices(instance, &device_count, devices.begin());
                for (auto device : devices)
                {
                    physical_devices.emplace_back(instance, device);
                }

                return physical_devices;

            }

            static inline spx::vk::physical_device
            get_optimal_device(spx::vk::instance instance)
            {

                auto physical_devices = spx::vk::physical_device::get_physical_devices(instance);

                spx::vk::physical_device optimal_device;
                int64_t maximum_score = 0;
                for (auto &device : physical_devices)
                {
                    const int64_t device_score = device.get_device_score();
                    if (device_score > maximum_score)
                    {
                        maximum_score = device_score;
                        optimal_device = device;
                    }
                }

                return optimal_device;

            }

        public:
            VkPhysicalDevice device = NULL;
            VkPhysicalDeviceProperties          device_properties_1 = {};
            VkPhysicalDeviceProperties2         device_properties_2 = {};
            VkPhysicalDeviceDriverProperties    driver_properties = {};
            VkPhysicalDeviceMemoryProperties    memory_properties = {};

            VkPhysicalDeviceFeatures2           device_10_features = {};
            VkPhysicalDeviceVulkan11Features    device_11_features = {};
            VkPhysicalDeviceVulkan12Features    device_12_features = {};
            VkPhysicalDeviceVulkan13Features    device_13_features = {};
            VkPhysicalDeviceVulkan14Features    device_14_features = {};

            spx::dynamic_array<spx::vk::queue_family> queue_families;

    };

}