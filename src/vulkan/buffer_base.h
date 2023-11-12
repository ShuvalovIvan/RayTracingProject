#pragma once

#include <vulkan/vulkan.h>

#include "device.h"

namespace VulkanImpl
{

class BufferBase {
protected:
    BufferBase(Device &device) : _device(device) {}

    ~BufferBase() {}

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

        if (vkAllocateMemory(_device.device(), &allocInfo, nullptr, &buffer_memory) != VK_SUCCESS ||
            vkBindBufferMemory(_device.device(), buffer, buffer_memory, 0) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to allocate vertex buffer memory!"));
        }
    }

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool command_pool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = command_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(_device.device(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool command_pool, VkQueue queue)
    {
        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
            LOG_AND_THROW(std::runtime_error("vkEndCommandBuffer failed"));
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
        VkFence fence;
        if (vkCreateFence(_device.device(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
            LOG_AND_THROW(std::runtime_error("failed to create synchronization object!"));
        }

        if (VK_SUCCESS != vkQueueSubmit(queue, 1, &submitInfo, fence))
        {
            LOG_AND_THROW(std::runtime_error("vkQueueSubmit failed"));
        }
        // Wait for the fence to signal that command buffer has finished executing
        if (vkWaitForFences(_device.device(), 1, &fence, VK_TRUE, 100000) != VK_SUCCESS){
            std::cerr << "Fence wait failed" << std::endl;
        }
        vkDestroyFence(_device.device(), fence, nullptr);
        vkQueueWaitIdle(queue);  // Queue could be compute.

        vkFreeCommandBuffers(_device.device(), command_pool, 1, &commandBuffer);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool command_pool)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(command_pool);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer, command_pool, _device.graphics_queue());
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

    Device &_device;

private:
    BufferBase(const BufferBase&) = delete;
    BufferBase &operator=(const BufferBase&) = delete;
};

} // namespace
