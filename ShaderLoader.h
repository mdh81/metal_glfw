#pragma once

#include <filesystem>

class ShaderLoader {
  public:
    static std::string loadShader(const std::filesystem::path& path);
};
