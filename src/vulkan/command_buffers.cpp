#include "command_buffers.h"

namespace VulkanImpl
{

void CommandBuffers::reset_record_graphics_command_buffer(
    VkFramebuffer frame_buffer,
    VkExtent2D swap_chain_extent,
    std::map<PipelineType, std::unique_ptr<Pipeline>>& pipelines,
    const VertexBuffer &vertex_buffer,
    DescriptorsManager &descriptors,
    uint32_t current_frame,
    VkClearValue background,
    const RenderPass &render_pass)
{
    assert(swap_chain_extent.height > 10);
    assert(swap_chain_extent.width > 10);
    assert(current_frame < _command_buffers.size());
    if (vkResetCommandBuffer(_command_buffers[current_frame], /*VkCommandBufferResetFlagBits*/ 0) != VK_SUCCESS) {
        LOG_AND_THROW(std::runtime_error("failed to reset command buffer!"));
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(_command_buffers[current_frame], &beginInfo) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to begin recording command buffer!"));
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass.render_pass();
    renderPassInfo.framebuffer = frame_buffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swap_chain_extent;

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &background;

    vkCmdBeginRenderPass(_command_buffers[current_frame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PipelineType::Graphics]->pipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_command_buffers[current_frame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(_command_buffers[current_frame], 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertex_buffer.vertex_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(_command_buffers[current_frame], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(_command_buffers[current_frame], vertex_buffer.index_buffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelines[PipelineType::Graphics]->pipeline_layout(), 0, 1, &descriptors.descriptor(FrameIndex(current_frame)), 0, nullptr);

    vkCmdDrawIndexed(_command_buffers[current_frame], static_cast<uint32_t>(vertex_buffer.indices_size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(_command_buffers[current_frame]);

    if (vkEndCommandBuffer(_command_buffers[current_frame]) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to record command buffer!"));
    }
}

void CommandBuffers::reset_record_compute_command_buffer(
    std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
    DescriptorsManager &descriptors,
    uint32_t current_frame)
{
    assert(current_frame < _command_buffers.size());
    if (vkResetCommandBuffer(_command_buffers[current_frame], /*VkCommandBufferResetFlagBits*/ 0) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to reset command buffer!"));
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(_command_buffers[current_frame], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    auto& pipeline = pipelines[PipelineType::Compute];
    vkCmdBindPipeline(_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline());

    vkCmdBindDescriptorSets(_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline_layout(), 0, 1,
        &descriptors.descriptor(FrameIndex(current_frame)), 0, nullptr);

    vkCmdDispatch(_command_buffers[current_frame], 1 /*PARTICLE_COUNT / 256*/, 1, 1);

    if (vkEndCommandBuffer(_command_buffers[current_frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record compute command buffer!");
    }
}

}  // namespace
