#include "ShaderLoader.h"
#include <fstream>

std::string ShaderLoader::loadShader(std::filesystem::path const& path)
{
    if (!exists(path)) {
        throw std::runtime_error(std::format("Shader {} does not exist", path.string()));
    }
    std::ifstream shaderFile(path);
    if (!shaderFile) {
          throw std::runtime_error(std::format("Failed to open file {}", path.string()));
    }
    shaderFile.seekg(0, std::ios::end);
    auto size = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);
    std::string shaderCode;
    shaderCode.resize(size);
    shaderFile.read(shaderCode.data(), size);
    return shaderCode;
}