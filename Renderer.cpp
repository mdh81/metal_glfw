#include "Renderer.h"
#include "Metal/MTLRenderPass.hpp"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "QuartzCore/CAMetalDrawable.hpp"
#include "QuartzCore/CAMetalLayer.hpp"
#include "QuartzCore/QuartzCore.h"
#include "GLFW/glfw3.h"

#include <print>
#include <array>

namespace {
    void glfwErrorCallback(int const error, char const* description) {
        std::println(stderr, "GLFW Error {}: {}", error, description);
    }
    auto constexpr WindowTitle {"Metal Renderer"};
    std::array constexpr BackgroundColor {0.3f, 0.3f, 0.3f, 1.f};
}

Renderer::Renderer(unsigned short const width, unsigned short const height, void* metalLayer)
    : width(width)
    , height(height)
    , device(nullptr)
    , commandQueue(nullptr)
    , renderPipelineState(nullptr)
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

    while (!glfwWindowShouldClose(glfwWindow)) {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        metalLayer->setDrawableSize(CGSizeMake(width, height));

        auto const drawable = metalLayer->nextDrawable();

        auto const commandBuffer = commandQueue->commandBuffer();

        colorAttachment->setClearColor(
            MTL::ClearColor(
                BackgroundColor[0],BackgroundColor[1], BackgroundColor[2],BackgroundColor[3]));
        colorAttachment->setTexture(drawable->texture());
        colorAttachment->setLoadAction(MTL::LoadActionClear);
        colorAttachment->setStoreAction(MTL::StoreActionStore);

        auto const renderEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
        renderEncoder->endEncoding();

        commandBuffer->presentDrawable(drawable);
        commandBuffer->commit();
    }
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    renderPassDescriptor->release();
}
