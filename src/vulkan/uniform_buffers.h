#pragma once

#include <vulkan/vulkan.h>

namespace VulkanImpl
{

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class UniformBuffers {
public:
    UniformBuffers(const Device &device, int size) : _device(device), _size(size) {}

    ~UniformBuffers() {
        for (size_t i = 0; i < _size; i++)
        {
            vkDestroyBuffer(_device.device(), _uniform_buffers[i], nullptr);
            vkFreeMemory(_device.device(), _uniform_buffers_memory[i], nullptr);
        }
    }

    void init() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        _uniform_buffers.resize(_size);
        _uniform_buffers_memory.resize(_size);
        _uniform_buffers_mapped.resize(_size);

        for (size_t i = 0; i < _size; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                _uniform_buffers[i], _uniform_buffers_memory[i]);

            vkMapMemory(_device.device(), _uniform_buffers_memory[i], 0, bufferSize, 0, &_uniform_buffers_mapped[i]);
        }
    }

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(_device.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create buffer!"));
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device.device(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_device.device(), &allocInfo, nullptr, &buffer_memory) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to allocate vertex buffer memory!"));
        }

        vkBindBufferMemory(_device.device(), buffer, buffer_memory, 0);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_device.physical_device(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        LOG_AND_THROW(std::runtime_error("failed to find suitable memory type!"));
    }

    const Device &_device;
    const int _size;

    std::vector<VkBuffer> _uniform_buffers;
    std::vector<VkDeviceMemory> _uniform_buffers_memory;
    std::vector<void *> _uniform_buffers_mapped;
};

} // namespace
