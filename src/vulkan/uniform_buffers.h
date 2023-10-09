#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "buffer_base.h"

namespace VulkanImpl
{

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class UniformBuffers : public BufferBase {
public:
    UniformBuffers(Device &device, int size) : BufferBase(device), _size(size) {}

    ~UniformBuffers() {
        for (size_t i = 0; i < _size; i++)
        {
            vkDestroyBuffer(_device.device(), _uniform_buffers[i], nullptr);
            vkFreeMemory(_device.device(), _uniform_buffers_memory[i], nullptr);
        }
    }

    VkBuffer uniform_buffer(int frame_index) const {
        assert(frame_index < _uniform_buffers.size());
        return _uniform_buffers[frame_index];
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

            if (vkMapMemory(_device.device(), _uniform_buffers_memory[i], 0, bufferSize, 0, &_uniform_buffers_mapped[i]) != VK_SUCCESS) {
                LOG_AND_THROW(std::runtime_error("failed to map memory"));
            }
        }
    }

    void copy_to_uniform_buffers_for_image(uint32_t currentImage, const UniformBufferObject& ubo)
    {
        assert(currentImage < _size);
        memcpy(_uniform_buffers_mapped[currentImage], &ubo, sizeof(ubo));
    }

private:
    UniformBuffers(const UniformBuffers &) = delete;
    UniformBuffers &operator=(const UniformBuffers &) = delete;

    const int _size;

    std::vector<VkBuffer> _uniform_buffers;
    std::vector<VkDeviceMemory> _uniform_buffers_memory;
    std::vector<void *> _uniform_buffers_mapped;
};

} // namespace
