#include "command_buffer.h"

namespace VulkanImpl
{

    void CommandBuffer::reset_record_command_buffer(
        VkRenderPass render_pass, VkFramebuffer frame_buffer,
        VkExtent2D swap_chain_extent, VkPipeline pipeline,
        VkBuffer vertex_buffer)
    {
        vkResetCommandBuffer(_command_buffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_command_buffer, &beginInfo) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to begin recording command buffer!"));
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = render_pass;
        renderPassInfo.framebuffer = frame_buffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swap_chain_extent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swap_chain_extent.width;
        viewport.height = (float)swap_chain_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swap_chain_extent;
        vkCmdSetScissor(_command_buffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(_command_buffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(_command_buffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(_command_buffer);

        if (vkEndCommandBuffer(_command_buffer) != VK_SUCCESS)
        {
            LOG_AND_THROW(std::runtime_error("failed to record command buffer!"));
        }
}

}  // namespace
