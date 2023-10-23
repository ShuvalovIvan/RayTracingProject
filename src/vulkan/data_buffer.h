#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "buffer_base.h"
#include "vertex.h"

namespace VulkanImpl
{

template<typename T, size_t MAX>
class DataBuffer : public BufferBase {
public:
    DataBuffer(Device &device) : BufferBase(device) {
        _data.count = 0;
    }

    ~DataBuffer() {
        vkDestroyBuffer(_device.device(), _data_buffer, nullptr);
        vkFreeMemory(_device.device(), _data_buffer_memory, nullptr);
    }

    void init(VkCommandPool command_pool)
    {
        VkDeviceSize bufferSize = sizeof(Data);

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(_device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, static_cast<void*>(&_data), (size_t)bufferSize);
        vkUnmapMemory(_device.device(), stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _data_buffer, _data_buffer_memory);

        copyBuffer(stagingBuffer, _data_buffer, bufferSize, command_pool);

        vkDestroyBuffer(_device.device(), stagingBuffer, nullptr);
        vkFreeMemory(_device.device(), stagingBufferMemory, nullptr);
    }

    void append(T obj) {
        if (_data.count < MAX) {
            _data.objects[_data.count] = obj;
            ++_data.count;
        }
    }

private:
    struct Data {
        alignas(4)  int count;
        alignas(16) T objects[MAX];
    };

    DataBuffer(const DataBuffer &) = delete;
    DataBuffer &operator=(const DataBuffer &) = delete;

    VkBuffer _data_buffer;
    VkDeviceMemory _data_buffer_memory;

    Data _data;
};

} // namespace
