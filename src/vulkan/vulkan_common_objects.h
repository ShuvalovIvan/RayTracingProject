#pragma once

#include <vulkan/vulkan.h>

#include "common_objects.h"
#include "device.h"

namespace VulkanImpl
{

enum class BindingSequence : uint32_t
{
    COMMON_UBO = 0,
    TEXTURE_IMAGE_SAMPLER,
    FRAME_IMAGE,
};

enum class BindingType {
    Buffer,
    Image,
    SwapChainImage,
    Acceleration
};

enum class BindingKey {
    PrimaryTexture,
    CommonUBO,
    FrameImage
};

enum class BindingsMaxCount : uint32_t {};

enum class ImageIndex : uint32_t {};

enum class FrameIndex : uint32_t {};

} // namespace
