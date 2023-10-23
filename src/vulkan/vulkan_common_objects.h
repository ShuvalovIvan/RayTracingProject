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
};

enum class BindingType {
    Buffer,
    Image,
    Acceleration
};

enum class BindingsMaxCount : uint32_t {};

enum class ImageIndex : uint32_t {};

enum class FrameIndex : uint32_t {};

} // namespace
