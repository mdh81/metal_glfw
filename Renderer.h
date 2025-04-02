#pragma once
#include <Metal/MTLRenderPipeline.hpp>

class GLFWwindow;

namespace CA {
    class MetalLayer;
}

namespace MTL {
    class Device;
    class CommandQueue;
    class RenderPipelineState;
    class Buffer;
    class RenderPassDescriptor;
}

class Renderer {
public:
    Renderer(unsigned short width, unsigned short height, void*);
    ~Renderer();
    GLFWwindow* initialize();
    void run();
private:
    void initializeMetal();
    void initializeWindow();
    void preparePipeline();
    uint16_t prepareData();

    unsigned short width, height;
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::RenderPipelineState* renderPipelineState;
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor;
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    GLFWwindow* glfwWindow;
    CA::MetalLayer* metalLayer;
};
