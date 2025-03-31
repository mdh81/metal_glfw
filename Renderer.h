#pragma once
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLCommandQueue.hpp"

class GLFWwindow;

namespace CA {
    class MetalLayer;
}

class Renderer {
public:
    Renderer(unsigned short width, unsigned short height, void*);
    GLFWwindow* initialize();
    void run() const;
private:
    void initializeMetal();
    void initializeWindow();

    unsigned short width, height;
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    GLFWwindow* glfwWindow;
    CA::MetalLayer* metalLayer;
};
