#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace VulkanImpl
{

    class ModelLoader
    {
    public:
        ModelLoader(const std::string& model_path, const std::string& texture_path,
            int width, int height)
            : _model_path(model_path), _texture_path(texture_path), _width(width), _texture_path(texture_path) {}

        void load() {

        }

    private:
        ModelLoader(const ModelLoader &) = delete;
        ModelLoader &operator=(const ModelLoader &) = delete;

        const std::string _model_path;
        const std::string _texture_path;
        int _width;
        int _height;
    };

} // namespace
