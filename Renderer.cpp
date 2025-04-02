#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "Renderer.h"
#include "ShaderLoader.h"
#include "ShaderTypes.h"
#include "Metal/MTLRenderPass.hpp"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLBuffer.hpp"
#include "Metal/MTLLibrary.hpp"
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

    NS::String* asAppleString(std::string const& cppString) {
        return NS::String::string(cppString.c_str(), NS::UTF8StringEncoding);
    }

    std::string asString(NS::Error const* error) {
        return error->localizedDescription()->utf8String();
    }

    std::array constexpr ClearColor {0.3f, 0.3f, 0.3f, 1.f};

    uint8_t constexpr NumberOfInstances {1};

    bool TakeScreenShot {false};

}

Renderer::Renderer(unsigned short const width, unsigned short const height, void* metalLayer)
    : width(width)
    , height(height)
    , device{nullptr}
    , commandQueue{nullptr}
    , renderPipelineState{nullptr}
    , renderPipelineDescriptor{nullptr}
    , vertexBuffer{nullptr}
    , indexBuffer(nullptr)
    , glfwWindow{nullptr}
    , metalLayer{static_cast<CA::MetalLayer*>(metalLayer)} {
}

Renderer::~Renderer() {
    if (renderPipelineDescriptor) renderPipelineDescriptor->release();
    if (renderPipelineState) renderPipelineState->release();
    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();
    if (commandQueue) commandQueue->release();
    if (device) device->release();
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
    glfwWindow = glfwCreateWindow(width, height, WindowTitle, nullptr, nullptr);
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

void Renderer::preparePipeline() {
    NS::AutoreleasePool* autoReleasePool = NS::AutoreleasePool::alloc()->init();
    NS::Error* error{};
    auto const shaderSource = ShaderLoader::loadShader("Shaders.metal");
    auto const shaderLibrary = device->newLibrary(asAppleString(shaderSource), nullptr, &error);
    if (!shaderLibrary) {
        throw std::runtime_error(std::format("Failed to create shader library. Encountered error {}",
            asString(error)));
    }

    auto const shaderFunction = shaderLibrary->newFunction(asAppleString("processVertex"));
    auto const fragmentFunction = shaderLibrary->newFunction(asAppleString("processFragment"));

    renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipelineDescriptor->setVertexFunction(shaderFunction);
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    renderPipelineState = device->newRenderPipelineState(renderPipelineDescriptor, &error);
    if (!renderPipelineState) {
        throw std::runtime_error(std::format("Failed to create render pipeline state."
                                 "Encountered error {} Exiting...", asString(error)));
    }
    autoReleasePool->release();
}

uint16_t Renderer::prepareData() {
    if (vertexBuffer) vertexBuffer->release();
    if (indexBuffer) indexBuffer->release();

    VertexPositionAndColor vertexData[4] {
        {{-1.f, -1.f, 1.f}, {1.f, 1.f, 1.f}},
        {{1.f, -1.f, 1.f}, {1.f, 1.f, 1.f}},
        {{1.f, 1.f, 1.f},{.93f, .47f, .3f}},
        {{-1.f, 1.f, 1.f},{.93f, .47f, .3f}},
    };

    std::array<uint16_t, 6> indices {
        0, 1, 2,
        0, 2, 3
    };

    vertexBuffer = device->newBuffer(vertexData, 4 * sizeof(VertexPositionAndColor), MTL::ResourceStorageModeShared);
    indexBuffer = device->newBuffer(indices.data(), 6 * sizeof(uint16_t), MTL::ResourceStorageModeShared);

    return indices.size();
}

void Renderer::run() {

    preparePipeline();

    auto const numIndices = prepareData();

    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            TakeScreenShot = true;
        }
    });

    auto const renderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
    auto const colorAttachment = renderPassDescriptor->colorAttachments()->object(0);

    while (!glfwWindowShouldClose(glfwWindow)) {
        NS::AutoreleasePool* autoReleasePool = NS::AutoreleasePool::alloc()->init();

        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(glfwWindow, &width, &height);
        metalLayer->setDrawableSize(CGSizeMake(width, height));

        auto const drawable = metalLayer->nextDrawable();
        if (!drawable) {
            throw std::runtime_error("Failed to create drawable. Exiting...");
        }

        auto const commandBuffer = commandQueue->commandBuffer();

        colorAttachment->setClearColor(
            MTL::ClearColor(
                ClearColor[0],ClearColor[1], ClearColor[2],ClearColor[3]));
        auto const texture = drawable->texture();
        colorAttachment->setTexture(texture);
        colorAttachment->setLoadAction(MTL::LoadActionClear);
        colorAttachment->setStoreAction(MTL::StoreActionStore);

        auto const commandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
        commandEncoder->setRenderPipelineState(renderPipelineState);
        commandEncoder->setVertexBuffer(vertexBuffer, 0, 0);
        commandEncoder->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle, numIndices, MTL::IndexTypeUInt16, indexBuffer,
            0, NumberOfInstances);
        commandEncoder->endEncoding();

        commandBuffer->presentDrawable(drawable);
        commandBuffer->commit();

        if (TakeScreenShot) {
            std::println("Taking screenshot...");
            commandBuffer->waitUntilCompleted();
            std::vector<char> rendereredContent;
            rendereredContent.resize(texture->width() * texture->height() * 4);
            MTL::Region region {0, 0, texture->width(), texture->height() };
            texture->getBytes(rendereredContent.data(), texture->width() * 4, rendereredContent.size(), region, 0, 0);
            for (auto i = 0; i < rendereredContent.size(); i+=4) {
                std::swap(rendereredContent[i+2], rendereredContent[i]);
            }
            stbi_write_png("image.png", static_cast<int>(texture->width()),
                static_cast<int>(texture->height()), 4, rendereredContent.data(), 0);
            TakeScreenShot = false;
        }
        autoReleasePool->release();
    }
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    renderPassDescriptor->release();
}
