#include "Renderer.h"
#include <print>

#include <Metal/MTLCommandBuffer.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <QuartzCore/CAMetalDrawable.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include "GLFW/glfw3.h"
#include "QuartzCore/QuartzCore.h"

namespace {
    void glfwErrorCallback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    auto constexpr WindowTitle {"Metal Renderer"};
}

Renderer::Renderer(unsigned short width, unsigned short height, void* metalLayer)
    : width(width)
    , height(height)
    , device(nullptr)
    , commandQueue(nullptr)
    , glfwWindow(nullptr)
    , metalLayer(static_cast<CA::MetalLayer*>(metalLayer)) {
}


void Renderer::initializeMetal() {
    device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        throw std::runtime_error("Metal is not supported. Exiting...");
    }
    commandQueue = device->newCommandQueue();
    if (!commandQueue) {
        throw std::runtime_error("Failed to create command queue. Exiting...");
    }
}

void Renderer::initializeWindow() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindow =
        glfwCreateWindow(width, height,
            WindowTitle, nullptr, nullptr);
    if (!glfwWindow) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    metalLayer->setDevice(device);
    metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
}

GLFWwindow* Renderer::initialize() {
    initializeMetal();
    initializeWindow();
    return glfwWindow;
}

void Renderer::run() const {

    auto const renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
    auto const colorAttachment = renderPassDescriptor->colorAttachments()->object(0);

    float constexpr backgroundColor[4] = {.45f, .55f, .6f, 1.f};

    while (!glfwWindowShouldClose(glfwWindow)) {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        metalLayer->setDrawableSize(CGSizeMake(width, height));

        auto const drawable = metalLayer->nextDrawable();

        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();

        colorAttachment->setClearColor(
            MTL::ClearColor(
                backgroundColor[0],backgroundColor[1], backgroundColor[2],backgroundColor[3]));
        colorAttachment->setTexture(drawable->texture());
        colorAttachment->setLoadAction(MTL::LoadActionClear);
        colorAttachment->setStoreAction(MTL::StoreActionStore);

        MTL::RenderCommandEncoder* renderEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
        renderEncoder->endEncoding();

        commandBuffer->presentDrawable(drawable);
        commandBuffer->commit();
}
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    renderPassDescriptor->release();
}
