#include "command_buffers.h"

namespace VulkanImpl
{

void CommandBuffers::reset_record_graphics_command_buffer(
    VkFramebuffer frame_buffer,
    VkExtent2D swap_chain_extent,
    std::map<PipelineType, std::unique_ptr<Pipeline>>& pipelines,
    const VertexBuffer &vertex_buffer,
    DescriptorsManager &descriptors,
    FrameIndex current_frame,
    ImageIndex image_index,
    VkClearValue background,
    const RenderPass &render_pass)
{
    assert(swap_chain_extent.height > 10);
    assert(swap_chain_extent.width > 10);
    assert(static_cast<uint32_t>(current_frame) < _command_buffers.size());
    VkCommandBuffer& command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];

    if (vkResetCommandBuffer(command_buffer, /*VkCommandBufferResetFlagBits*/ 0) != VK_SUCCESS) {
        LOG_AND_THROW(std::runtime_error("failed to reset command buffer!"));
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS)
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

    vkCmdBeginRenderPass(command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[PipelineType::Graphics]->pipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swap_chain_extent.width;
    viewport.height = (float)swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertex_buffer.vertex_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, vertex_buffer.index_buffer(), 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelines[PipelineType::Graphics]->pipeline_layout(), 0, 1, &descriptors.descriptor(image_index), 0, nullptr);

    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(vertex_buffer.indices_size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to record command buffer!"));
    }
}

void CommandBuffers::reset_record_compute_command_buffer(
    std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
    DescriptorsManager &descriptors,
    FrameIndex current_frame,
    ImageIndex image_index)
{
    assert(static_cast<uint32_t>(current_frame) < _command_buffers.size());
    VkCommandBuffer &command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];

    if (vkResetCommandBuffer(command_buffer, /*VkCommandBufferResetFlagBits*/ 0) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to reset command buffer!"));
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffer, &beginInfo) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to begin recording compute command buffer!"));
    }
}

void CommandBuffers::prepare_to_trace_barrier(FrameIndex current_frame, VkImage image)
{
    VkCommandBuffer &command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];
    VkImageSubresourceRange access;
    access.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    access.baseMipLevel = 0;
    access.levelCount = 1;
    access.baseArrayLayer = 0;
    access.layerCount = 1;

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = access;

    VkPipelineStageFlags sourceStage, destinationStage;

    barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    vkCmdPipelineBarrier(
        command_buffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void CommandBuffers::dispatch_raytrace(
    std::map<PipelineType, std::unique_ptr<Pipeline>> &pipelines,
    DescriptorsManager &descriptors,
    FrameIndex current_frame, ImageIndex image_index)
{
    VkCommandBuffer &command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];

    auto &pipeline = pipelines[PipelineType::Compute];
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline());

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline_layout(), 0, 1,
                            &descriptors.descriptor(image_index), 0, nullptr);

    vkCmdDispatch(command_buffer, 1, 1, 1);
    // commandBuffer.dispatch(static_cast<uint32_t>(swapchainExtent.width / 8), static_cast<uint32_t>(swapchainExtent.height / 8), 1);
}

void CommandBuffers::prepare_to_present_barrier(FrameIndex current_frame, VkImage image)
{
    VkCommandBuffer &command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];
    VkImageSubresourceRange access;
    access.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    access.baseMipLevel = 0;
    access.levelCount = 1;
    access.baseArrayLayer = 0;
    access.layerCount = 1;

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = access;

    VkPipelineStageFlags sourceStage, destinationStage;

    barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    barrier.dstAccessMask = VK_ACCESS_NONE_KHR;
    destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    vkCmdPipelineBarrier(
        command_buffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void CommandBuffers::end_command_buffer(FrameIndex current_frame)
{
    VkCommandBuffer &command_buffer = _command_buffers[static_cast<uint32_t>(current_frame)];
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        LOG_AND_THROW(std::runtime_error("failed to record compute command buffer!"));
    }
}

}  // namespace
