target_sources(${PROJECT_LIB}
    PRIVATE
        vulkan/command_buffers.cpp
        vulkan/descriptors_manager.cpp
        vulkan/device.cpp
        vulkan/graphical_environment_vulkan.cpp
        vulkan/ray_tracing_pipeline.cpp
        vulkan/shader_loader.cpp
        vulkan/texture.cpp
        vulkan/validation.cpp
        vulkan/vertex_buffer.cpp
    PUBLIC
        graphical_environment.h
)

target_link_libraries(${PROJECT_LIB} ${Vulkan_LIBRARIES})
