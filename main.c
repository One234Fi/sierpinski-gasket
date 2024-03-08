#include "vulkan.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

const uint32_t NUM_POINTS = 100;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};
const uint32_t NUM_VALIDATION_LAYERS = 
    sizeof(validationLayers) / sizeof(validationLayers[0]);
const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const uint32_t NUM_DEVICE_EXTENSIONS = 
    sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else 
    const bool enableValidationLayers = true;
#endif

Vertex vertices[100] = {};

void print_points(Vertex* points, uint32_t NUM_POINTS);

int generate_points(uint32_t numPoints, uint32_t width, uint32_t height, Vertex* vertices) {
    float triangle_vertices[3][2] = {
        {0.0f, 0.0f}, 
        {width / 2.0f, (float) height}, 
        {(float) width, 0.0f}
    };
    Vertex p = {
        .pos = {0.0f, 0.0f}, 
        .color ={1.0f, 0.0f, 0.0f}
    };
    int j;
    int rand();
    Vertex out [numPoints];
    memset(out, 0, sizeof(Vertex) * numPoints);
    for (uint32_t k = 0; k < numPoints; k++) {
        j = rand()%3;
        p.pos[0] = (p.pos[0] + triangle_vertices[j][0] - width) / 2 / width;
        p.pos[1] = (p.pos[1] + triangle_vertices[j][1] - height) / 2 / height;

        out[k] = p;
    }

    memcpy(vertices, out, sizeof(Vertex) * numPoints);
    return 1;
}

bool checkValidationLayerSupport(const char** validationLayers, uint32_t numLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);


    fprintf(stdout, "availableLayers: \n");
    for (uint32_t i = 0; i < layerCount; i++) {
        fprintf(stdout, "%s\n", availableLayers[i].layerName);
    }

    for (uint32_t i = 0; i < numLayers; i++) {
        bool layerFound = false;
        for (uint32_t j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

const char* const* getRequiredExtensions(uint32_t* count) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = 
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
 
    if (!enableValidationLayers) {
        *count = glfwExtensionCount;
        return glfwExtensions;
    }

    const char** extensions = malloc((glfwExtensionCount+1) * sizeof(const char*));
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions[i] = glfwExtensions[i];
    }

    extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *count = glfwExtensionCount+1;
    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger) {

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
        createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo->pfnUserCallback = debugCallback;
        createInfo->pUserData = NULL;
}

VkVertexInputBindingDescription getVertexBinding() {
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex),

        //input per-vertex, could also be per-instance if doing instance rendering
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return bindingDescription;
}

int getAttributeDescriptions(VkVertexInputAttributeDescription* dst, uint32_t* n) {
    if (!dst && !n) {
        fprintf(stderr, "Must pass a valid pointer\n");
        return false;
    }

    uint32_t attributeCount = 2;
    if (!dst) {
        *n = attributeCount;
        return true;
    }

    dst[0].binding = 0;
    dst[0].location = 0;
    dst[0].format = VK_FORMAT_R32G32_SFLOAT;
    dst[0].offset = offsetof(Vertex, pos);

    dst[1].binding = 0;
    dst[1].location = 1;
    dst[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    dst[1].offset = offsetof(Vertex, color);

    return true;
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    ctx* app = (ctx*) glfwGetWindowUserPointer(window);
    app->framebufferResized = true;
}

int initWindow(ctx* ctx) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    ctx->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
    glfwSetFramebufferSizeCallback(ctx->window, framebufferResizeCallback);
    glfwSetWindowUserPointer(ctx->window, ctx);
    if (!ctx->window) {
        fprintf(stderr, "ERROR: Couldn't create glfw window\n");
        return 0;
    }
    return 1;
}

int createInstance(ctx* ctx) {
    if (enableValidationLayers && !checkValidationLayerSupport(
                validationLayers, NUM_VALIDATION_LAYERS)) {
        fprintf(stderr, "Validation layers are enabled, but not available\n");
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Base",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t extensionCount = 0;
    const char* const* extensions;
    extensions = getRequiredExtensions(&extensionCount);
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = NUM_VALIDATION_LAYERS;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, NULL, &ctx->instance) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create vulkan instance\n");
        return false;
    }
    return true;
}

int setupDebugMessenger(ctx* ctx) {
    if (!enableValidationLayers) {
        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(&createInfo);

    if (CreateDebugUtilsMessengerEXT(ctx->instance, &createInfo, NULL, 
                &ctx->debugMessenger) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't setup debug messenger\n");
        return false;
    }

    return true;
}

int createSurface(ctx* ctx) {
    if (glfwCreateWindowSurface(ctx->instance, ctx->window, NULL, &ctx->surface)
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to create window surface\n");
        return false;
    }
    return true;
}

bool qfiComplete(qfi* indices) {
    return indices->hasGraphics && indices->hasPresent && indices->hasTransfer;
}

void findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, qfi* indices) {
    memset(indices, 0, sizeof(qfi));
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    memset(queueFamilies, 0, queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);
    
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        //graphics
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices->graphicsFamily = i;
            indices->hasGraphics = true;
        }

        //present
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices->presentFamily = i;
            indices->hasPresent = true;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && 
                !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices->transferFamily = i;
            indices->hasTransfer = true;
        }

        if (qfiComplete(indices)) {
            break;
        }
    }

    if (!qfiComplete(indices)) {
        fprintf(stderr, "ERROR: Couldn't find one or more required queue families\n");
    }
}

//queuset must be preallocd with the max number of queues
void determineDistinctQueues(qfi* indices, uint32_t* count, uint32_t* queueSet) {
    //figure out how many distinct queues there are
    uint32_t queues[] = {
        indices->presentFamily, 
        indices->graphicsFamily, 
        indices->transferFamily 
    };
    uint32_t queueFamilyCount = 1;
    queueSet[0] = queues[0];
    for (uint32_t i = 1; i < 3; i++) {
        bool inSet = false;
        for (uint32_t j = 0; j < i; j++) {
            if (queues[i] == queueSet[j]) {
                inSet = true;
                break;
            }
        }
        if (!inSet) {
            queueSet[queueFamilyCount] = queues[i];
            queueFamilyCount += 1;
        }
    }

    *count = queueFamilyCount;
}

bool graphicsCardSupportsExtensions(VkPhysicalDevice device, const char** extensions,
        uint32_t numExtensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties availableExtensions[extensionCount];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, 
            availableExtensions);
    
    for (uint32_t i = 0; i < numExtensions; i++) {
        bool extensionFound = false;
        for (uint32_t j = 0; j < extensionCount; j++) {
            if (strcmp(extensions[i], availableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        
        if (!extensionFound) {
            return false;
        }
    }

    return true;
}

bool swapchainAdequate(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

    uint32_t numFormats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, NULL);
    VkSurfaceFormatKHR formats[numFormats];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, formats);

    uint32_t numPresentModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, 
            &numPresentModes, NULL);
    VkPresentModeKHR presentModes[numPresentModes];
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, 
            &numPresentModes, presentModes);

    //having at least one format and at least one present mode is sufficient here 
    return numFormats > 0 && numPresentModes > 0;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    qfi indices;
    findQueueFamilies(device, surface, &indices);

    return qfiComplete(&indices) 
        && graphicsCardSupportsExtensions(device, deviceExtensions, NUM_DEVICE_EXTENSIONS)
        && swapchainAdequate(device, surface);
}

int pickPhysicalDevice(ctx* ctx) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "ERROR: Couldn't find GPUs with vulkan support\n");
        return false;
    }

    VkPhysicalDevice devices[deviceCount];
    memset(devices, 0, sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, devices);

    //look for a suitable discrete gpu
    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                && isDeviceSuitable(devices[i], ctx->surface)) {
            fprintf(stdout, "Found a suitable discrete GPU\n");
            physicalDevice = devices[i];
            break;
        } else if (isDeviceSuitable(devices[i], ctx->surface)) {
            fprintf(stdout, "Found a suitable non-discrete GPU\n");
            physicalDevice = devices[i];
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        fprintf(stdout, "Couldn't find a suitable GPU\n");
        return false;
    }

    ctx->physicalDevice = physicalDevice;
    return true;
}


int createLogicalDevice(ctx* ctx) {
    qfi indices;
    findQueueFamilies(ctx->physicalDevice, ctx->surface, &indices);

    if (!qfiComplete(&indices)) {
        fprintf(stderr, "ERROR: missing a required queue for logical device creation\n");
        return false;
    }

    uint32_t queueSet[3] = {};
    uint32_t queueFamilyCount;
    determineDistinctQueues(&indices, &queueFamilyCount, queueSet);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[queueFamilyCount];
    memset(queueCreateInfos, 0, sizeof(VkDeviceQueueCreateInfo) * queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = queueSet[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    //optional features
    VkPhysicalDeviceFeatures deviceFeatures = {
        .fillModeNonSolid = VK_TRUE,
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = queueFamilyCount,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = NUM_DEVICE_EXTENSIONS,
        .ppEnabledExtensionNames = deviceExtensions,
    };
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = NUM_VALIDATION_LAYERS;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(ctx->physicalDevice, &createInfo, NULL, &ctx->logicalDevice)
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create logical device\n");
        return false;
    }

    vkGetDeviceQueue(ctx->logicalDevice, indices.graphicsFamily, 0, &ctx->graphicsQueue);
    vkGetDeviceQueue(ctx->logicalDevice, indices.presentFamily, 0, &ctx->presentQueue);
    vkGetDeviceQueue(ctx->logicalDevice, indices.transferFamily, 0, &ctx->transferQueue);

    return true;
}

VkSurfaceFormatKHR pickSwapchainSurfaceFormat(VkSurfaceFormatKHR* formats, 
        uint32_t numFormats) {
    for (uint32_t i = 0; i < numFormats; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }

    return formats[0];
}
VkPresentModeKHR  pickSwapchainPresentMode(VkPresentModeKHR* presentModes,
        uint32_t numPresentModes) {
    for (uint32_t i = 0; i < numPresentModes; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentModes[i];
        }
    }

    // always available, but not preferred
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D pickSwapchainExtent(VkSurfaceCapabilitiesKHR capabilities, 
        GLFWwindow* window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } 
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        .width = (uint32_t) width,
        .height = (uint32_t) height,
    };

    if (actualExtent.width < capabilities.minImageExtent.width) {
        actualExtent.width = capabilities.minImageExtent.width;
    }
    if (actualExtent.width > capabilities.maxImageExtent.width) {
        actualExtent.width = capabilities.maxImageExtent.width;
    }
    if (actualExtent.height < capabilities.minImageExtent.height) {
        actualExtent.height = capabilities.minImageExtent.height;
    }
    if (actualExtent.height > capabilities.maxImageExtent.height) {
        actualExtent.height = capabilities.maxImageExtent.height;
    }

    return actualExtent;
}


int createSwapchain(ctx* ctx) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physicalDevice, ctx->surface, 
            &capabilities);

    uint32_t numFormats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, 
            &numFormats, NULL);
    VkSurfaceFormatKHR formats[numFormats];
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physicalDevice, ctx->surface, 
            &numFormats, formats);

    uint32_t numPresentModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, ctx->surface, 
            &numPresentModes, NULL);
    VkPresentModeKHR presentModes[numPresentModes];
    vkGetPhysicalDeviceSurfacePresentModesKHR(ctx->physicalDevice, ctx->surface, 
            &numPresentModes, presentModes);

    VkSurfaceFormatKHR surfaceFormat = pickSwapchainSurfaceFormat(formats, 
            numFormats);
    VkPresentModeKHR presentMode = pickSwapchainPresentMode(presentModes, 
            numPresentModes);
    VkExtent2D extent = pickSwapchainExtent(capabilities, ctx->window);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ctx->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    qfi indices;
    findQueueFamilies(ctx->physicalDevice, ctx->surface, &indices);
    uint32_t queueFamilyIndices[3] = {};
    uint32_t queueFamilyCount = 0;
    determineDistinctQueues(&indices, &queueFamilyCount, queueFamilyIndices);
    if (queueFamilyCount > 1) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueFamilyCount;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    if (vkCreateSwapchainKHR(ctx->logicalDevice, &createInfo, NULL, &ctx->swapchain)
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create swapchain\n");
        return false;
    }

    vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, 
            &ctx->numSwapchainImages, NULL);
    ctx->swapchainImages = malloc(sizeof(VkImage) * ctx->numSwapchainImages);
    if (vkGetSwapchainImagesKHR(ctx->logicalDevice, ctx->swapchain, 
            &ctx->numSwapchainImages, ctx->swapchainImages) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't load swapchain images\n");
        return false;
    }

    ctx->swapchainImageFormat = surfaceFormat.format;
    ctx->swapchainExtent = extent;
    
    return true;
}


int createImageViews(ctx* ctx) {
    ctx->swapchainImageViews = malloc(sizeof(VkImageView) * ctx->numSwapchainImages);
    for (uint32_t i = 0; i < ctx->numSwapchainImages; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = ctx->swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = ctx->swapchainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        if (vkCreateImageView(ctx->logicalDevice, &createInfo, NULL, 
                    &ctx->swapchainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: Couldn't create one or more swapchain image views\n");
            return false;
        }
    }
    return true;
}

int createRenderPass(ctx* ctx) {
    VkAttachmentDescription colorAttachment = {
        .format = ctx->swapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, //framebuffer color data before render 
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE, //framebuffer color data after render
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    //subpass(es)
    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    //subpass dependencies (control execution order)
    //
    //this makes sure that image transitions during the renderpass happen
    //at the right time. There are implicit subpasses at the beginning and end
    //of the renderpass, this dependency makes sure that the explicit subpass 
    //is done in between the pre and post renderpass image transitions
    //
    //this synchronization could also be done in the drawFrame function with semaphores
    //
    //TODO: do some more reading on subpass dependencies and vulkan synchronization
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    //renderpass
    VkRenderPassCreateInfo renderpassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(ctx->logicalDevice, &renderpassInfo, NULL, 
                &ctx->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create render pass\n");
        return false;
    }

    return true;
}

int createShader(VkDevice device, char* path, VkShaderModule* shader) {
    FILE* file = fopen(path, "rb+");

    if (file == NULL) {
        fprintf(stderr, "ERROR: Couldn't open shader file\n");
        return false;
    }

    int fd = fileno(file);
    struct stat fileStats;
    fstat(fd, &fileStats);
    
    char buffer[fileStats.st_size];
    read(fd, buffer, fileStats.st_size);
    fclose(file);

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fileStats.st_size,
        .pCode = (uint32_t*) buffer,
    };

    if (vkCreateShaderModule(device, &createInfo, NULL, shader) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create shader module\n");
        return false;
    }

    return true;
}

int createGraphicsPipeline(ctx* ctx) {
    //Programmable stages
    VkShaderModule vertexShader, fragmentShader;
    if (!createShader(ctx->logicalDevice, VERT_SHADER, &vertexShader)) {
        fprintf(stderr, "Vertex shader couldn't be loaded\n");
        return false;
    }
    if (!createShader(ctx->logicalDevice, FRAG_SHADER, &fragmentShader)) {
        fprintf(stderr, "Fragment shader couldn't be loaded\n");
        vkDestroyShaderModule(ctx->logicalDevice, vertexShader, NULL);
        return false;
    }
    fprintf(stdout, "shaders created\n");
    
    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShader,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShader,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderStageInfo,
        fragmentShaderStageInfo,
    };

    VkVertexInputBindingDescription bindingDescription = getVertexBinding();

    uint32_t numAttributeDescriptions;
    getAttributeDescriptions(NULL, &numAttributeDescriptions);
    VkVertexInputAttributeDescription attributeDescriptions[numAttributeDescriptions];
    getAttributeDescriptions(attributeDescriptions, &numAttributeDescriptions);

    fprintf(stdout, "prog stages done\n");

    //Fixed function stages
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = numAttributeDescriptions,
        .pVertexAttributeDescriptions = attributeDescriptions,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    fprintf(stdout, "input assembly\n");

    //Dynamic viewport set up, defers specifing viewport and scissor till draw 
    //time. If done as a proper fixed-function, a new pipeline would need to be 
    //created everytime the viewport or scissor changed.
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    uint32_t numDynamicStates = sizeof(dynamicStates) / sizeof(VkDynamicState);
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = numDynamicStates,
        .pDynamicStates = dynamicStates,
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    fprintf(stdout, "viewport\n");

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE, 
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                        | VK_COLOR_COMPONENT_G_BIT 
                        | VK_COLOR_COMPONENT_B_BIT 
                        | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f,
    };

    //pipeline layout is used to define uniform values (push constants) in shaders,
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL, 
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };
    if (vkCreatePipelineLayout(ctx->logicalDevice, &pipelineLayoutInfo, NULL,
                &ctx->pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create pipeline layout\n");
        vkDestroyShaderModule(ctx->logicalDevice, vertexShader, NULL);
        vkDestroyShaderModule(ctx->logicalDevice, fragmentShader, NULL);
        return false;
    }
    fprintf(stdout, "created layout\n");

    //Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = ctx->pipelineLayout,
        .renderPass = ctx->renderPass,
        .subpass = 0, //this is the subpass index, not a count

        //used for derived pipelines (disabled here)
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    if (vkCreateGraphicsPipelines(ctx->logicalDevice, VK_NULL_HANDLE, 1, 
                &pipelineInfo, NULL, &ctx->graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create the graphics pipeline\n");
        vkDestroyShaderModule(ctx->logicalDevice, vertexShader, NULL);
        vkDestroyShaderModule(ctx->logicalDevice, fragmentShader, NULL);
        return false;
    }
    fprintf(stdout, "created graphics pipelines\n");

    vkDestroyShaderModule(ctx->logicalDevice, vertexShader, NULL);
    vkDestroyShaderModule(ctx->logicalDevice, fragmentShader, NULL);
    return true;
}

int createFramebuffers(ctx* ctx) {
    ctx->swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * 
            ctx->numSwapchainImages);

    //attach each framebuffer to its corresponding image view
    for (uint32_t i = 0; i < ctx->numSwapchainImages; i++) {
        VkImageView attachments[] = {
            ctx->swapchainImageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = ctx->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = ctx->swapchainExtent.width,
            .height = ctx->swapchainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(ctx->logicalDevice, &framebufferInfo, NULL, 
                    &ctx->swapchainFramebuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "ERROR: Couldn't create a framebuffer\n");
            return false;
        }
    }
    
    return true;
}

int createCommandPools(ctx* ctx) {
    qfi queueFamilyIndices;
    findQueueFamilies(ctx->physicalDevice, ctx->surface, &queueFamilyIndices);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily,
    };
    
    if (vkCreateCommandPool(ctx->logicalDevice, &poolInfo, NULL, 
                &ctx->graphicsCommandPool) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create the graphics command pool\n");
        return false;
    }

    VkCommandPoolCreateInfo poolInfo2 = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.transferFamily,
    };
    if (vkCreateCommandPool(ctx->logicalDevice, &poolInfo2, NULL, 
                &ctx->transferCommandPool) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create the transfer command pool\n");
        return false;
    }

    return true;
}

int findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,
        VkPhysicalDevice device, uint32_t* out) {

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if (typeFilter & (i << i) && (memProps.memoryTypes[i].propertyFlags 
                    & properties) == properties) {
            *out = i;
            return true;
        }
    }

    fprintf(stderr, "ERROR: couldn't find a suitable memory type\n");
    return false;
}

int createBuffer(ctx* ctx, VkDeviceSize size, VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size, 
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(ctx->logicalDevice, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't create buffer\n");
        return false;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(ctx->logicalDevice, *buffer, &memReqs);

    uint32_t memType;
    if (!findMemoryType(memReqs.memoryTypeBits, properties, ctx->physicalDevice, 
                &memType)) {
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memType,
    };

    if (vkAllocateMemory(ctx->logicalDevice, &allocInfo, NULL, bufferMemory)
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't allocate vertex buffer memory\n");
        return false;
    }
    vkBindBufferMemory(ctx->logicalDevice, *buffer, *bufferMemory, 0);

    return true;
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, 
        ctx* ctx) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = ctx->transferCommandPool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(ctx->logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };

        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    //could also use a fence to wait instead, (better for batches of commands)
    vkQueueSubmit(ctx->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx->transferQueue);
    vkFreeCommandBuffers(ctx->logicalDevice, ctx->transferCommandPool, 1, 
            &commandBuffer);
}

int createVertexBuffer(ctx* ctx) {
    print_points(vertices, NUM_POINTS);

    VkDeviceSize bufferSize = sizeof(Vertex) * NUM_POINTS;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    if (!createBuffer(
            ctx, 
            bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer, 
            &stagingBufferMemory)) {
        fprintf(stderr, "ERROR: Failed to create staging buffer\n");
        return false;
    }

    void* data;
    vkMapMemory(ctx->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, &vertices, (uint32_t) bufferSize);
    printf("\n\ncopied points:\n\n");
    print_points(data, NUM_POINTS);
    vkUnmapMemory(ctx->logicalDevice, stagingBufferMemory);

    if (!createBuffer(
            ctx,
            bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            &ctx->vertexBuffer,
            &ctx->vertexBufferMemory)) {
        fprintf(stderr, "ERROR: Failed to create vertex buffer\n");
        return false;
    }

    copyBuffer(stagingBuffer, ctx->vertexBuffer, bufferSize, ctx);
    vkDestroyBuffer(ctx->logicalDevice, stagingBuffer, NULL);
    vkFreeMemory(ctx->logicalDevice, stagingBufferMemory, NULL);
    return true;
}

int createCommandBuffers(ctx* ctx) {
    ctx->commandBuffers = malloc(sizeof(VkCommandBuffer) * ctx->MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = ctx->graphicsCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = ctx->MAX_FRAMES_IN_FLIGHT,
    };

    if (vkAllocateCommandBuffers(ctx->logicalDevice, &allocInfo, ctx->commandBuffers) 
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't allocate command buffers\n");
        return false;
    }

    return true;
}

int recordCommandBuffer(ctx* ctx, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't begin recording command buffer %d\n", 
                imageIndex);
        return false;
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f,}}};
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = ctx->renderPass,
        .framebuffer = ctx->swapchainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = ctx->swapchainExtent,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                ctx->graphicsPipeline);

        //this is done here because the graphics pipeline specifies dynamic viewport
        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float) ctx->swapchainExtent.width,
            .height = (float) ctx->swapchainExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        //also dynamic scissor
        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = ctx->swapchainExtent,
        };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { ctx->vertexBuffer };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(commandBuffer, NUM_POINTS, 1, 0, 0);
        //vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't record command buffer %d\n", imageIndex);
        return false;
    }

    return true;
}

int createSyncObjects(ctx* ctx) {
    ctx->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * ctx->MAX_FRAMES_IN_FLIGHT);
    ctx->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * ctx->MAX_FRAMES_IN_FLIGHT);
    ctx->inFlightFences = malloc(sizeof(VkFence) * ctx->MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    //start in signaled state so that drawFrame doesn't deadlock on frame 0
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint32_t i = 0; i < ctx->MAX_FRAMES_IN_FLIGHT; i++) {
        if ((vkCreateSemaphore(ctx->logicalDevice, &semaphoreInfo, NULL, 
                &ctx->imageAvailableSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateSemaphore(ctx->logicalDevice, &semaphoreInfo, NULL, 
                &ctx->renderFinishedSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateFence(ctx->logicalDevice, &fenceInfo, NULL, 
                &ctx->inFlightFences[i]) != VK_SUCCESS)) {
            fprintf(stderr, "ERROR: Couldn't create a sync objects\n");
            return false;
        }
    }

    return true;
}

int initVulkan(ctx* ctx) {
    ctx->MAX_FRAMES_IN_FLIGHT = 2;
    ctx->currentFrame = 0;
    ctx->framebufferResized = false;
    if (!createInstance(ctx)) { return false; }
    if (!setupDebugMessenger(ctx)) { return false; }
    if (!createSurface(ctx)) { return false; }
    if (!pickPhysicalDevice(ctx)) { return false; }
    if (!createLogicalDevice(ctx)) { return false; }
    if (!createSwapchain(ctx)) { return false; }
    if (!createImageViews(ctx)) { return false; }
    if (!createRenderPass(ctx)) { return false; }
    if (!createGraphicsPipeline(ctx)) { return false; }
    if (!createFramebuffers(ctx)) { return false; }
    if (!createCommandPools(ctx)) { return false; }
    if (!createVertexBuffer(ctx)) { return false; }
    if (!createCommandBuffers(ctx)) { return false; }
    if (!createSyncObjects(ctx)) { return false; }
    return true;
}

int cleanupSwapchain(ctx* ctx) {
    for (uint32_t i = 0; i < ctx->numSwapchainImages; i++) {
            vkDestroyFramebuffer(ctx->logicalDevice, ctx->swapchainFramebuffers[i], NULL);
    }
    for (uint32_t i = 0; i < ctx->numSwapchainImages; i++) {
            vkDestroyImageView(ctx->logicalDevice, ctx->swapchainImageViews[i], NULL);
    }
        vkDestroySwapchainKHR(ctx->logicalDevice, ctx->swapchain, NULL);
    return true;
}

int recreateSwapchain(ctx* ctx) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(ctx->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(ctx->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(ctx->logicalDevice);

    cleanupSwapchain(ctx);
    
    if (!createSwapchain(ctx)) {
        fprintf(stderr, "ERROR: FAILED TO RECREATE SWAPCHAIN\n");
        return false;
    } 
    if (!createImageViews(ctx)) {
        fprintf(stderr, "ERROR: FAILED TO RECREATE IMAGE VIEWS\n");
        return false;
    }
    if (!createFramebuffers(ctx)) {
        fprintf(stderr, "ERROR: FAILED TO RECREATE FRAMEBUFFERS\n");
        return false;
    }

    return true;
}

int drawFrame(ctx* ctx) {
    //stall host until gpu has signalled that the render is finished
    vkWaitForFences(ctx->logicalDevice, 1, &ctx->inFlightFences[ctx->currentFrame], 
            VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(ctx->logicalDevice, ctx->swapchain, UINT64_MAX, 
            ctx->imageAvailableSemaphores[ctx->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain(ctx);
        return true;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "ERROR: Couldn't acquire swapchain image\n");
        return false;
    }

    //return fence to unsignaled state after recieving signal
    vkResetFences(ctx->logicalDevice, 1, &ctx->inFlightFences[ctx->currentFrame]);

    vkResetCommandBuffer(ctx->commandBuffers[ctx->currentFrame], 0);
    recordCommandBuffer(ctx, ctx->commandBuffers[ctx->currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = {
        ctx->imageAvailableSemaphores[ctx->currentFrame],
    };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    VkSemaphore signalSemaphores[] = {
        ctx->renderFinishedSemaphores[ctx->currentFrame],
    };
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &ctx->commandBuffers[ctx->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    if (vkQueueSubmit(ctx->graphicsQueue, 1, &submitInfo, ctx->inFlightFences[ctx->currentFrame]) 
            != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't submit draw command buffer %d\n", imageIndex);
        return false;
    }

    VkSwapchainKHR swapchains[] = { ctx->swapchain };
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
        .pResults = NULL,
    };

    res = vkQueuePresentKHR(ctx->presentQueue, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || ctx->framebufferResized) {
        ctx->framebufferResized = false;
        recreateSwapchain(ctx);
    } else if (res != VK_SUCCESS) {
        fprintf(stderr, "ERROR: Couldn't present swapchain image\n");
        return false;
    }

    //currentFrame should probably become a static variable in this func
    //instead of a ctx variable if its not used outside of here
    ctx->currentFrame = (ctx->currentFrame + 1) % ctx->MAX_FRAMES_IN_FLIGHT;

    return true;
}

int mainLoop(ctx* ctx) {
    while (!glfwWindowShouldClose(ctx->window)) {
        glfwPollEvents();
        if (!drawFrame(ctx)) {
            return false;
        }
    }
    vkDeviceWaitIdle(ctx->logicalDevice);
    return true;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = 
        (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, 
                "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

int cleanup(ctx* ctx) {
    cleanupSwapchain(ctx);
    for (uint32_t i = 0; i < ctx->MAX_FRAMES_IN_FLIGHT; i++) {
        if (ctx->imageAvailableSemaphores[i]) { vkDestroySemaphore(ctx->logicalDevice, ctx->imageAvailableSemaphores[i], NULL); }
        if (ctx->inFlightFences[i]) { vkDestroyFence(ctx->logicalDevice, ctx->inFlightFences[i], NULL); }
        if (ctx->renderFinishedSemaphores[i]) { vkDestroySemaphore(ctx->logicalDevice, ctx->renderFinishedSemaphores[i], NULL); }
    }
    if (ctx->vertexBuffer) { vkDestroyBuffer(ctx->logicalDevice, ctx->vertexBuffer, NULL); }
    if (ctx->vertexBufferMemory) { vkFreeMemory(ctx->logicalDevice, ctx->vertexBufferMemory, NULL); }
    if (ctx->graphicsCommandPool) { vkDestroyCommandPool(ctx->logicalDevice, ctx->graphicsCommandPool, NULL); }
    if (ctx->transferCommandPool) { vkDestroyCommandPool(ctx->logicalDevice, ctx->transferCommandPool, NULL); }
    if (ctx->graphicsPipeline) { vkDestroyPipeline(ctx->logicalDevice, ctx->graphicsPipeline, NULL); }
    if (ctx->pipelineLayout) { vkDestroyPipelineLayout(ctx->logicalDevice, ctx->pipelineLayout, NULL); }
    if (ctx->renderPass) { vkDestroyRenderPass(ctx->logicalDevice, ctx->renderPass, NULL); }
    if (ctx->logicalDevice) { vkDestroyDevice(ctx->logicalDevice, NULL); }
    if (enableValidationLayers) { DestroyDebugUtilsMessengerEXT(ctx->instance, ctx->debugMessenger, NULL); }
    if (ctx->surface) { vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL); }
    if (ctx->instance) { vkDestroyInstance(ctx->instance, NULL); }
    if (ctx->window) { glfwDestroyWindow(ctx->window); }
    glfwTerminate();

    if (ctx->swapchainImages) { free(ctx->swapchainImages); }
    if (ctx->swapchainImageViews) { free(ctx->swapchainImageViews); }
    if (ctx->swapchainFramebuffers) { free(ctx->swapchainFramebuffers); }
    if (ctx->commandBuffers) { free(ctx->commandBuffers); }
    if (ctx->imageAvailableSemaphores) { free(ctx->imageAvailableSemaphores); }
    if (ctx->renderFinishedSemaphores) { free(ctx->renderFinishedSemaphores); }
    if (ctx->inFlightFences) { free(ctx->inFlightFences); }
    free(ctx);
    return 1;
}

void print_points(Vertex* points, uint32_t NUM_POINTS) {
    Vertex* p = points;
    for (uint32_t i = 0; i < NUM_POINTS; i++) {
        fprintf(stdout, "%f, %f\n", p->pos[0], p->pos[1]);
        p++;
    }
}

int main() {
    ctx* app = malloc(sizeof(ctx));
    memset(app, 0, sizeof(ctx));
    uint32_t exit_code = EXIT_SUCCESS;

    generate_points(NUM_POINTS, WIDTH, HEIGHT, vertices);
    //print_points(vertices, NUM_POINTS);

    if (!initWindow(app)) {
        fprintf(stderr, "Problem with window initialization\n");
        exit_code = EXIT_FAILURE;
    }
    if (!exit_code && !initVulkan(app)) {
        fprintf(stderr, "Problem with vulkan initialization\n");
        exit_code = EXIT_FAILURE;
    }
    if (!exit_code && !mainLoop(app)) {
        fprintf(stderr, "Problem during the main loop\n");
        exit_code = EXIT_FAILURE;
    }
    if (!cleanup(app)) {
        fprintf(stderr, "Problem during cleanup\n");
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}

