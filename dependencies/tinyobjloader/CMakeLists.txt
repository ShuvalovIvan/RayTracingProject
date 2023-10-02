target_sources(${PROJECT_LIB}
    PRIVATE
        tiny_obj_loader.cc
    PUBLIC
        tiny_obj_loader.h
)

target_link_libraries(${PROJECT_LIB} ${Vulkan_LIBRARIES})
