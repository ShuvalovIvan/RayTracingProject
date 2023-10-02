#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer_base.h"
#include "vertex.h"

namespace VulkanImpl
{

class VertexBuffer : public BufferBase {
public:
    VertexBuffer(const Device &device) : BufferBase(device) {}

    ~VertexBuffer() {
        vkDestroyBuffer(_device.device(), _index_buffer, nullptr);
        vkFreeMemory(_device.device(), _index_buffer_memory, nullptr);

        vkDestroyBuffer(_device.device(), _vertex_buffer, nullptr);
        vkFreeMemory(_device.device(), _vertex_buffer_memory, nullptr);
    }

    VkBuffer vertex_buffer() const {
        return _vertex_buffer;
    }

    VkBuffer index_buffer() const {
        return _index_buffer;
    }

    void init(VkCommandPool command_pool)
    {
        create_vertex_buffer(command_pool);
        create_index_buffer(command_pool);
    }

    int indices_size() const {
        return _indices.size();
    }

private:
    VertexBuffer(const VertexBuffer &) = delete;
    VertexBuffer &operator=(const VertexBuffer &) = delete;

    void create_vertex_buffer(VkCommandPool command_pool)
    {
        VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(_device.device(), stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertex_buffer, _vertex_buffer_memory);

        copyBuffer(stagingBuffer, _vertex_buffer, bufferSize, command_pool);

        vkDestroyBuffer(_device.device(), stagingBuffer, nullptr);
        vkFreeMemory(_device.device(), stagingBufferMemory, nullptr);
    }

    void create_index_buffer(VkCommandPool command_pool)
    {
        VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _indices.data(), (size_t)bufferSize);
        vkUnmapMemory(_device.device(), stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _index_buffer, _index_buffer_memory);

        copyBuffer(stagingBuffer, _index_buffer, bufferSize, command_pool);

        vkDestroyBuffer(_device.device(), stagingBuffer, nullptr);
        vkFreeMemory(_device.device(), stagingBufferMemory, nullptr);
    }

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

    const std::vector<Vertex> _vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

    const std::vector<uint16_t> _indices = {
        0, 1, 2, 2, 3, 0};

    VkBuffer _vertex_buffer;
    VkDeviceMemory _vertex_buffer_memory;
    VkBuffer _index_buffer;
    VkDeviceMemory _index_buffer_memory;
};

} // namespace
