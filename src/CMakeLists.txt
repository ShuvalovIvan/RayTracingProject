target_sources(${PROJECT_LIB}
    PRIVATE
        vulkan/device.cpp
        vulkan/graphical_environment_vulkan.cpp
        vulkan/ray_tracing_pipeline.cpp
        vulkan/shader_loader.cpp
    PUBLIC
        graphical_environment.h
)

target_link_libraries(${PROJECT_LIB} ${Vulkan_LIBRARIES})
