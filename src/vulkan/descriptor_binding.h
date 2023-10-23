#pragma once

#include <vulkan/vulkan.h>

#include "common_objects.h"
#include "device.h"

namespace VulkanImpl
{

enum class Binding : uint32_t {
    COMMON_UBO = 0,
}

class DescriptorBinding
{
public:
    DescriptorBinding(const Device &device, Binding binding)
        : _device(device) {}

private:
    const Device &_device;
    const Binding _binding;
};

} // namespace
