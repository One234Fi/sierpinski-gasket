#ifndef VULKAN_H
#define VULKAN_H

#include "math.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef VERT_SHADER
#define VERT_SHADER "./shaders/vert.spv"
#endif
#ifndef FRAG_SHADER
#define FRAG_SHADER "./shaders/frag.spv"
#endif

typedef struct ctx {
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFramebuffer* swapchainFramebuffers;
    uint32_t numSwapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool graphicsCommandPool;
    VkCommandPool transferCommandPool;
    
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    uint32_t MAX_FRAMES_IN_FLIGHT;

    uint32_t currentFrame;
    uint32_t framebufferResized;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
} ctx;

typedef struct qfi {
    uint32_t graphicsFamily;
    uint32_t hasGraphics;
    uint32_t presentFamily;
    uint32_t hasPresent;
    uint32_t transferFamily;
    uint32_t hasTransfer;
} qfi;

typedef struct Vertex {
    float pos[2];
    float color[3];
} Vertex;

int run();

#endif
