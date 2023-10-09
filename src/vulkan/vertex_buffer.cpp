#include "vertex_buffer.h"

namespace VulkanImpl
{

// static
const std::vector<Vertex> VertexBuffer::_vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

// static
const std::vector<uint16_t> VertexBuffer::_indices = {
    0, 1, 2, 2, 3, 0};
}
