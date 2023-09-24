#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "vertex.h"

namespace VulkanImpl
{

class VertexBuffer {
public:
    VertexBuffer(const Device& device) : _device(device) {}

    ~VertexBuffer() {
        vkDestroyBuffer(_device.device(), _vertex_buffer, nullptr);
        vkFreeMemory(_device.device(), _vertex_buffer_memory, nullptr);
    }

    void init() {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(vertices[0]) * vertices.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(_device.device(), &bufferInfo, nullptr, &_vertex_buffer) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to create vertex buffer!"));
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device.device(), _vertex_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(_device.device(), &allocInfo, nullptr, &_vertex_buffer_memory) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to allocate vertex buffer memory!"));
        }

        vkBindBufferMemory(_device.device(), _vertex_buffer, _vertex_buffer_memory, 0);

        void *data;
        vkMapMemory(_device.device(), _vertex_buffer_memory, 0, bufferInfo.size, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferInfo.size);
        vkUnmapMemory(_device.device(), _vertex_buffer_memory);
    }

private:
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

    const Device& _device;

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

    VkBuffer _vertex_buffer;
    VkDeviceMemory _vertex_buffer_memory;
};

} // namespace
