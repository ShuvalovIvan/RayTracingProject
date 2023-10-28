#pragma once

#include <vulkan/vulkan.h>
#include <map>

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
        for (const auto& key_val : _uniform_buffers) {
            BindingKey key = key_val.first;
            for (size_t i = 0; i < _size; i++)
            {
                vkDestroyBuffer(_device.device(), _uniform_buffers[key][i], nullptr);
                vkFreeMemory(_device.device(), _uniform_buffers_memory[key][i], nullptr);
            }
        }
    }

    VkBuffer uniform_buffer(FrameIndex frame_index, BindingKey key) const
    {
        auto it = _uniform_buffers.find(key);
        assert(it != _uniform_buffers.end());
        assert(static_cast<size_t>(frame_index) < (*it).second.size());
        return (*it).second[static_cast<size_t>(frame_index)];
    }

    size_t uniform_buffer_size(FrameIndex frame_index, BindingKey key) const
    {
        auto it = _uniform_buffer_sizes.find(key);
        assert(it != _uniform_buffer_sizes.end());
        assert(static_cast<size_t>(frame_index) < (*it).second.size());
        return (*it).second[static_cast<size_t>(frame_index)];
    }

    template<typename T>
    void add(BindingKey key) {
        VkDeviceSize bufferSize = sizeof(T);

        _uniform_buffers[key].resize(_size);
        _uniform_buffers_memory[key].resize(_size);
        _uniform_buffers_mapped[key].resize(_size);
        _uniform_buffer_sizes[key].resize(_size);

        for (size_t i = 0; i < _size; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         _uniform_buffers[key][i], _uniform_buffers_memory[key][i]);

            if (vkMapMemory(_device.device(), _uniform_buffers_memory[key][i], 0, bufferSize, 0, &_uniform_buffers_mapped[key][i]) != VK_SUCCESS)
            {
                LOG_AND_THROW(std::runtime_error("failed to map memory"));
            }
            _uniform_buffer_sizes[key][i] = bufferSize;
        }
    }

    template <typename T>
    void copy_to_uniform_buffers_for_frame(FrameIndex currentFrame, BindingKey key, const T &ubo)
    {
        assert(static_cast<size_t>(currentFrame) < _size);
        memcpy(_uniform_buffers_mapped[key][static_cast<size_t>(currentFrame)], &ubo, sizeof(ubo));
    }

private:
    UniformBuffers(const UniformBuffers &) = delete;
    UniformBuffers &operator=(const UniformBuffers &) = delete;

    const int _size;

    std::map<BindingKey, std::vector<VkBuffer>> _uniform_buffers;
    std::map<BindingKey, std::vector<VkDeviceMemory>> _uniform_buffers_memory;
    std::map<BindingKey, std::vector<void *>> _uniform_buffers_mapped;
    std::map<BindingKey, std::vector<size_t>> _uniform_buffer_sizes;
};

} // namespace
