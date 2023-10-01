target_sources(${PROJECT_LIB}
    PUBLIC
        stb_image.h
)

target_link_libraries(${PROJECT_LIB} ${Vulkan_LIBRARIES})
