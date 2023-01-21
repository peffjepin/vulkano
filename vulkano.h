#ifndef VULKANO_H
#define VULKANO_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

//
// **************************************************************************************
//
//                              PREPROCESSOR OPTIONS
//
// **************************************************************************************
//

// available flags:

// VULKANO_DISABLE_VALIDATION_LAYERS
// VULKANO_ENABLE_DEFAULT_VALIDATION_LAYERS
// VULKANO_ENABLE_DEFAULT_GRAPHICS_EXTENSIONS
// VULKANO_INTEGRATE_SDL
// VULKANO_LOG_ERRORS

#ifndef VULKANO_CONCURRENT_FRAMES
#define VULKANO_CONCURRENT_FRAMES 2
#endif

#ifndef VULKANO_LOG
#define VULKANO_LOG stdout
#endif

#ifndef VULKANO_TIMEOUT
#define VULKANO_TIMEOUT 5lu * 1000000000lu  // 5 seconds
#endif

//
// **************************************************************************************
//
//                              COMMON FORWARD DECLARATIONS
//
// **************************************************************************************
//

typedef int (*gpu_compare_function)(VkPhysicalDevice*, VkPhysicalDevice*);
typedef int (*surface_format_compare_function)(VkSurfaceFormatKHR*, VkSurfaceFormatKHR*);
typedef int (*present_mode_compare_function)(VkPresentModeKHR*, VkPresentModeKHR*);

// returns NULL on success, or an error message on an error
typedef const char* (*surface_creation_function)(VkInstance, VkSurfaceKHR*);
// tells the library how to query for window size
typedef void (*query_size_function)(uint32_t* width, uint32_t* height);

struct vulkano_data {
    uint32_t length;
    uint8_t* bytes;
};

//
// **************************************************************************************
//
//                              CONFIGURATION STRUCTURES
//
// **************************************************************************************
//

// Any Vk*CreateInfo structs listed in vulkano_config or within it's struct members
// will have their .sType field filled in by the library
//
struct vulkano_config {
    // required functions
    surface_creation_function surface_creation;
    query_size_function query_size;

    // All compare functions have default implementations
    gpu_compare_function gpu_compare;
    surface_format_compare_function format_compare;
    present_mode_compare_function present_compare;

    uint32_t validation_layers_count;
    const char** validation_layers;

    uint32_t instance_extensions_count;
    const char** instance_extensions;

    uint32_t gpu_extensions_count;
    const char** gpu_extensions;

    // if VkRenderPassCreateInfo.format == 0
    // the library will use the format chosen with `format_compare`
    // which is the same format that the swapchain is setup with by default
    uint32_t render_pass_count;
    struct VkRenderPassCreateInfo* render_passes;

    uint32_t descriptor_set_layout_count;
    struct VkDescriptorSetLayoutCreateInfo* descriptor_set_layouts;

    uint32_t pipeline_layout_count;
    struct vulkano_pipeline_layout_config* pipeline_layout_configs;

    uint32_t descriptor_pool_count;
    struct VkDescriptorPoolCreateInfo* descriptor_pools;

    uint32_t swapchain_image_count;
    uint32_t swapchain_render_pass_index;

    uint32_t pipeline_count;
    struct vulkano_graphics_pipeline_config* pipeline_configs;
};

struct vulkano_pipeline_layout_config {
    struct VkPipelineLayoutCreateInfo create_info;
    uint32_t descriptor_set_layout_count;
    uint32_t* descriptor_set_layout_indices;
};

struct vulkano_graphics_pipeline_config {
    struct vulkano_data vertex_shader;
    struct vulkano_data tessellation_control_shader;
    struct vulkano_data tessellation_evaluation_shader;
    struct vulkano_data geometry_shader;
    struct vulkano_data fragment_shader;
    struct VkPipelineVertexInputStateCreateInfo vertex_input;
    struct VkPipelineInputAssemblyStateCreateInfo input_assembly;
    struct VkPipelineTessellationStateCreateInfo tessellation;
    struct VkViewport viewport;
    struct VkRect2D scissor;
    struct VkPipelineRasterizationStateCreateInfo rasterization;
    struct VkPipelineMultisampleStateCreateInfo multisample;
    struct VkPipelineDepthStencilStateCreateInfo depth;
    struct VkPipelineColorBlendStateCreateInfo blend;
    struct VkPipelineDynamicStateCreateInfo dynamic;
    uint32_t pipeline_layout_index;
    uint32_t render_pass_index;
    uint32_t subpass;
    uint32_t base_pipeline_index;
};

//
// **************************************************************************************
//
//                                 RUNTIME STRUCTURES
//
// **************************************************************************************
//

#define VULKANO_MAX_SHADER_STAGES 5

struct vulkano_rendering_state {
    uint64_t frame_count;
    VkCommandBuffer command_buffers[VULKANO_CONCURRENT_FRAMES];
    VkSemaphore image_ready[VULKANO_CONCURRENT_FRAMES];
    VkSemaphore rendering_complete[VULKANO_CONCURRENT_FRAMES];
    VkFence frame_complete[VULKANO_CONCURRENT_FRAMES];
};

struct vulkano_swapchain_state {
    VkSwapchainKHR handle;
    query_size_function query_size;
    VkRenderPass render_pass;

    struct VkExtent2D extent;
    uint32_t image_count;
    VkImageView* image_views;
    VkFramebuffer* framebuffers;
};

struct vulkano_gpu {
    VkPhysicalDevice handle;
    struct VkPhysicalDeviceMemoryProperties memory_properties;
    struct VkPhysicalDeviceProperties properties;

    VkSurfaceFormatKHR selected_swapchain_format;
    VkPresentModeKHR selected_present_mode;

    uint32_t graphics_queue_family;
    VkQueue graphics_queue;
};

struct vulkano_graphics_pipeline {
    VkPipeline handle;
    uint32_t shader_modules_count;
    VkShaderModule shader_modules[VULKANO_MAX_SHADER_STAGES];
};

struct vulkano_resources {
    uint32_t render_pass_count;
    VkRenderPass* render_passes;

    uint32_t descriptor_set_layout_count;
    VkDescriptorSetLayout* descriptor_set_layouts;

    uint32_t pipeline_layout_count;
    VkPipelineLayout* pipeline_layouts;

    uint32_t graphics_pipeline_count;
    struct vulkano_graphics_pipeline* graphics_pipelines;

    uint32_t descriptor_pool_count;
    VkDescriptorPool* descriptor_pools;

    VkCommandPool command_pool;
};

struct vulkano {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;

    struct vulkano_gpu gpu;
    struct vulkano_resources resources;
    struct vulkano_swapchain_state swapchain;
    struct vulkano_rendering_state rendering;
};

enum vulkano_error_code {
    VULKANO_ERROR_CODE_OK = 0,
    VULKANO_ERROR_CODE_BAD_CONFIGURATION,
    VULKANO_ERROR_CODE_OUT_OF_MEMORY,
    VULKANO_ERROR_CODE_INSTANCE_CREATION_FAILED,
    VULKANO_ERROR_CODE_FAILED_ENUMERATION,
    VULKANO_ERROR_CODE_UNSUPPORTED_VALIDATION_LAYER,
    VULKANO_ERROR_CODE_UNSUPPORTED_INSTANCE_EXTENSION,
    VULKANO_ERROR_CODE_SURFACE_CREATION_FAILED,
    VULKANO_ERROR_CODE_NO_GPU_AVAILABLE,
    VULKANO_ERROR_CODE_NO_SUITABLE_GPU_AVAILABLE,
    VULKANO_ERROR_CODE_DEVICE_CREATION_FAILED,
    VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION,
    VULKANO_ERROR_CODE_FAILED_RESOURCE_ALLOCATION,
    VULKANO_ERROR_CODE_SWAPCHAIN_CONFIGURATION_FAILED,
    VULKANO_ERROR_CODE_FAILED_SWAPCHAIN_CREATION,
    VULKANO_ERROR_CODE_TIMEOUT,
    VULKANO_ERROR_CODE_FATAL_ERROR,
    VULKANO_ERROR_CODE_MEMORY_REQUIREMENTS_UNFULFILLED,
    VULKANO_ERROR_CODE_VALIDATION,
};

struct vulkano_error {
    enum vulkano_error_code code;
    VkResult result;
    char message[128];
};

#ifdef VULKANO_INTEGRATE_SDL
// title, width, height have defaults
struct sdl_config {
    char* title;
    int left;
    int top;
    int width;
    int height;
    uint32_t window_flags;
    uint32_t init_flags;
};

struct vulkano_sdl {
    struct vulkano vk;
    struct SDL_Window* sdl;
};
#endif

//
// **************************************************************************************
//
//                                 RUNTIME HELPERS
//
// **************************************************************************************
//

// Helpers to get an application off the ground. Should be expanded upon/reimplemented as
// application needs become clear/non-trivial;

struct vulkano vulkano_init(struct vulkano_config, struct vulkano_error*);
void vulkano_destroy(struct vulkano*);

#ifdef VULKANO_INTEGRATE_SDL
struct vulkano_sdl vulkano_sdl_init(
    struct vulkano_config vkcfg, struct sdl_config sdlcfg, struct vulkano_error* error
);
void vulkano_sdl_destroy(struct vulkano_sdl* vksdl);
#endif

#define VULKANO_WIDTH(vulkano) vulkano->swapchain.extent.width
#define VULKANO_HEIGHT(vulkano) vulkano->swapchain.extent.height
#define VULKANO_VIEWPORT(vulkano)                                                        \
    (struct VkViewport)                                                                  \
    {                                                                                    \
        .width = VULKANO_WIDTH(vulkano), .height = VULKANO_HEIGHT(vulkano),              \
        .maxDepth = 1.0f                                                                 \
    }
#define VULKANO_SCISSOR(vulkano)                                                         \
    (struct VkRect2D) { .extent = vulkano->swapchain.extent }
#define VULKANO_RENDER_PASS(vulkano, index) vulkano->resources.render_passes[index]
#define VULKANO_GRAPHICS_PIPELINE(vulkano, index)                                        \
    vulkano->resources.graphics_pipelines[index].handle

#define VULKANO_DATA_STACK_ARRAY(array)                                                  \
    (struct vulkano_data) { .length = sizeof(array), .bytes = array }

struct vulkano_buffer {
    VkBuffer handle;
    VkDeviceMemory memory;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memory_flags;
    uint32_t capacity;
};

struct vulkano_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkMemoryPropertyFlags memory_flags;
    VkImageLayout layout;
};

struct vulkano_frame {
    uint64_t number;
    VkCommandBuffer command_buffer;
    VkFramebuffer framebuffer;
    uint32_t image_index;
    float clear[4];
};

void vulkano_begin_frame(
    struct vulkano* vk, struct vulkano_frame* frame, struct vulkano_error* error
);
void vulkano_submit_frame(
    struct vulkano* vk, struct vulkano_frame* frame, struct vulkano_error* error
);

struct vulkano_buffer vulkano_create_buffer(
    struct vulkano* vk,
    uint32_t capacity,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags required_memory_properties,
    struct vulkano_error* error
);
void vulkano_destroy_buffer(struct vulkano* vk, struct vulkano_buffer* buffer);

void vulkano_copy_to_buffer(
    struct vulkano* vk,
    struct vulkano_buffer* buffer,
    struct vulkano_data data,
    struct vulkano_error* error
);

struct vulkano_image vulkano_create_image(
    struct vulkano* vk,
    struct VkImageCreateInfo info,
    VkMemoryPropertyFlags memory_flags,
    struct vulkano_error* error
);
void vulkano_destroy_image(struct vulkano* vk, struct vulkano_image* image);

void vulkano_copy_to_image(
    struct vulkano* vk,
    struct vulkano_image* image,
    struct vulkano_data data,
    uint32_t width,
    uint32_t height,
    struct vulkano_error* error
);

void vulkano_image_change_layout(
    struct vulkano* vk,
    struct vulkano_image* image,
    VkImageLayout layout,
    struct vulkano_error* error
);

VkCommandBuffer vulkano_acquire_single_use_command_buffer(
    struct vulkano* vk, struct vulkano_error* error
);
void vulkano_submit_single_use_command_buffer(
    struct vulkano* vk, VkCommandBuffer cmd, struct vulkano_error* error
);

//
// **************************************************************************************
//
//                        INDIVIDUAL INITIALIZATION FUNCTIONS
//
// **************************************************************************************
//

void vulkano_create_instance(
    struct vulkano* vk,
    uint32_t validation_layers_count,
    const char** validation_layers,
    uint32_t extensions_count,
    const char** extensions,
    struct vulkano_error* error
);

void vulkano_select_gpu(
    struct vulkano* vk,
    gpu_compare_function gpucmp,
    present_mode_compare_function presentcmp,
    surface_format_compare_function fmtcmp,
    uint32_t extensions_count,
    const char** extensions,
    struct vulkano_error* error
);

void vulkano_create_device(
    struct vulkano* vk,
    uint32_t gpu_extensions_count,
    const char** gpu_extensions,
    struct vulkano_error* error
);

void vulkano_create_resources(
    struct vulkano* vk, struct vulkano_config* config, struct vulkano_error* error
);
void vulkano_destroy_resources(struct vulkano* vk);

void vulkano_configure_swapchain(
    struct vulkano* vk,
    query_size_function query_size,
    uint32_t image_count,
    uint32_t render_pass_index,
    struct vulkano_error* error
);
void vulkano_create_swapchain(struct vulkano* vk, struct vulkano_error* error);
void vulkano_destroy_swapchain(struct vulkano* vk);

void vulkano_create_rendering_state(struct vulkano* vk, struct vulkano_error* error);
void vulkano_destroy_rendering_state(struct vulkano* vk);

void vulkano_check_validation_layer_support(
    uint32_t count, const char** validation_layers, struct vulkano_error* error
);
void vulkano_check_instance_extension_support(
    uint32_t count, const char** extensions, struct vulkano_error* error
);
bool vulkano_gpu_supports_extensions(
    VkPhysicalDevice gpu, uint32_t count, const char** extensions
);

//
// **************************************************************************************
//
//                               UTILITY FUNCTIONS
//
// **************************************************************************************
//

struct VkSurfaceFormatKHR vulkano_select_surface_format(
    VkPhysicalDevice gpu,
    VkSurfaceKHR surface,
    surface_format_compare_function cmp,
    struct vulkano_error* error
);
VkPresentModeKHR vulkano_select_present_mode(
    VkPhysicalDevice gpu,
    VkSurfaceKHR surface,
    present_mode_compare_function cmp,
    struct vulkano_error* error
);
VkCommandPool vulkano_create_command_pool(
    struct vulkano* vk,
    VkCommandPoolCreateFlags flags,
    uint32_t queue_family,
    struct vulkano_error* error
);

const char* vkresult_to_string(VkResult result);
const char* present_mode_to_string(VkPresentModeKHR mode);
const char* color_format_to_string(VkFormat fmt);
const char* color_space_to_string(VkColorSpaceKHR space);

int default_gpu_compare(VkPhysicalDevice* gpu1, VkPhysicalDevice* gpu2);
int default_present_modes_compare(VkPresentModeKHR* mode1, VkPresentModeKHR* mode2);
int default_surface_format_compare(VkSurfaceFormatKHR* fmt1, VkSurfaceFormatKHR* fmt2);

//
// **************************************************************************************
//
//                               BEGIN IMPLEMENTATION
//
// **************************************************************************************
//

#ifdef VULKANO_IMPLEMENTATION

#define LOG(message) fprintf(VULKANO_LOG, "%s", message)
#define LOGF(fmt, ...) fprintf(VULKANO_LOG, fmt, __VA_ARGS__)

#define CLAMP(min, max, value) ((value < min) ? min : ((value > max) ? max : value))

typedef int (*compare_function)(const void*, const void*);

struct string_array {
    uint32_t count;
    const char** data;
};

static void
vulkano_write_error_message(struct vulkano_error* error, const char* message)
{
    snprintf(
        error->message,
        sizeof(error->message),
        "%s (%s)",
        message,
        vkresult_to_string(error->result)
    );
#ifdef VULKANO_LOG_ERRORS
    LOGF("VULKANO ERROR: %s\n", error->message);
#endif
}

#define IS_VULKAN_MEMORY_ERROR(result)                                                   \
    ((result) == VK_ERROR_OUT_OF_HOST_MEMORY || (result) == VK_ERROR_OUT_OF_DEVICE_MEMORY)

static void
vulkano_out_of_memory(struct vulkano_error* error, VkResult result)
{
    error->result = (result == VK_SUCCESS) ? VK_ERROR_OUT_OF_HOST_MEMORY : result;
    error->code = VULKANO_ERROR_CODE_OUT_OF_MEMORY;
    vulkano_write_error_message(error, "out of memory");
}

static void
vulkano_fatal_error(struct vulkano_error* error, VkResult result)
{
    error->code = VULKANO_ERROR_CODE_FATAL_ERROR;
    error->result = result;
    vulkano_write_error_message(error, "fatal error encountered");
}

struct string_array
combine_string_arrays_unique(
    struct string_array array1, struct string_array array2, struct vulkano_error* error
)
{
    uint32_t total_count = array1.count + array2.count;
    if (total_count == 0) return (struct string_array){.count = 0, .data = NULL};

    struct string_array combined = {
        .data = (const char**)malloc((array1.count + array2.count) * sizeof(const char*)),
    };
    if (!combined.data) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return (struct string_array){.count = 0, .data = NULL};
    }

    if (array1.data) {
        combined.count = array1.count;
        memcpy(combined.data, array1.data, sizeof(array1.data[0]) * array1.count);
        qsort(array1.data, array1.count, sizeof(const char*), (compare_function)strcmp);
    }
    if (array2.data)
        for (uint32_t i = 0; i < array2.count; i++) {
            if (!array1.data || !bsearch(
                                    array2.data[i],
                                    array1.data,
                                    array1.count,
                                    sizeof(array1.data[0]),
                                    (compare_function)strcmp
                                ))
                memcpy(
                    combined.data + combined.count++,
                    array2.data + i,
                    sizeof(array1.data[0])
                );
        }
    return combined;
}

#define DEFAULT0(value, default) (value) = ((value)) ? (value) : (default)

struct vulkano
vulkano_init(struct vulkano_config config, struct vulkano_error* error)
{
    struct vulkano vk = {0};
    const char* surface_error = NULL;

    if (!config.surface_creation) {
        error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
        vulkano_write_error_message(
            error, "vulkano_config.surface_creation must be given"
        );
        return vk;
    }
    if (!config.query_size) {
        error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
        vulkano_write_error_message(error, "vulkano_config.query_size must be given");
        return vk;
    }

    DEFAULT0(config.gpu_compare, default_gpu_compare);
    DEFAULT0(config.format_compare, default_surface_format_compare);
    DEFAULT0(config.present_compare, default_present_modes_compare);

    // various default configurations availble through preprocessor
    static const char* DEFAULT_INSTANCE_VALIDATION_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation"};
    static const char* DEFAULT_GRAPHICS_GPU_EXTENSIONS[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    static struct string_array LIBRARY_REQUIRED_GPU_EXTENSIONS = {
        .count = 0,
        .data = NULL,
    };
    static struct string_array LIBRARY_REQUIRED_INSTANCE_EXTENSIONS = {
        .count = 0,
        .data = NULL,
    };
    static struct string_array LIBRARY_REQUIRED_VALIDATION_LAYERS = {
        .count = 0,
        .data = NULL,
    };
#ifdef VULKANO_ENABLE_DEFAULT_VALIDATION_LAYERS
    LIBRARY_REQUIRED_VALIDATION_LAYERS.data = DEFAULT_INSTANCE_VALIDATION_LAYERS;
    LIBRARY_REQUIRED_VALIDATION_LAYERS.count =
        sizeof(DEFAULT_INSTANCE_VALIDATION_LAYERS) / sizeof(const char*);
#endif
#ifdef VULKANO_ENABLE_DEFAULT_GRAPHICS_EXTENSIONS
    LIBRARY_REQUIRED_GPU_EXTENSIONS.data = DEFAULT_GRAPHICS_GPU_EXTENSIONS;
    LIBRARY_REQUIRED_GPU_EXTENSIONS.count =
        sizeof(DEFAULT_GRAPHICS_GPU_EXTENSIONS) / sizeof(const char*);
#endif

    // combine defaults (if requested) and user requested configuration into
    // single arrays with duplicates stripped
    //
    struct string_array user_required_validation_layers = {
        .count = config.validation_layers_count,
        .data = config.validation_layers,
    };
    struct string_array user_required_instance_extensions = {
        .count = config.instance_extensions_count,
        .data = config.instance_extensions,
    };
    struct string_array user_required_gpu_extensions = {
        .count = config.gpu_extensions_count,
        .data = config.gpu_extensions,
    };
    struct string_array required_validation_layers = combine_string_arrays_unique(
        LIBRARY_REQUIRED_VALIDATION_LAYERS, user_required_validation_layers, error
    );
    struct string_array required_instance_extensions = combine_string_arrays_unique(
        LIBRARY_REQUIRED_INSTANCE_EXTENSIONS, user_required_instance_extensions, error
    );
    struct string_array required_gpu_extensions = combine_string_arrays_unique(
        LIBRARY_REQUIRED_GPU_EXTENSIONS, user_required_gpu_extensions, error
    );
    if (error->code) goto cleanup;

    LOG("INITIALIZING VULKAN\n\n");

    // create instance
    vulkano_create_instance(
        &vk,
        required_validation_layers.count,
        required_validation_layers.data,
        required_instance_extensions.count,
        required_instance_extensions.data,
        error
    );
    if (error->code) goto cleanup;

    // create surface
    // native surface creation not currently implemented
    surface_error = config.surface_creation(vk.instance, &vk.surface);
    if (surface_error) {
        error->code = VULKANO_ERROR_CODE_SURFACE_CREATION_FAILED;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, surface_error);
        goto cleanup;
    }

    // select a gpu
    vulkano_select_gpu(
        &vk,
        config.gpu_compare,
        config.present_compare,
        config.format_compare,
        required_gpu_extensions.count,
        required_gpu_extensions.data,
        error
    );
    if (error->code) goto cleanup;

    // create logical device with selected gpu
    vulkano_create_device(
        &vk, required_gpu_extensions.count, required_gpu_extensions.data, error
    );
    if (error->code) goto cleanup;

    vulkano_create_resources(&vk, &config, error);
    if (error->code) goto cleanup;

    // configure swapchain (happens only this one time)
    vulkano_configure_swapchain(
        &vk,
        config.query_size,
        config.swapchain_image_count,
        config.swapchain_render_pass_index,
        error
    );
    if (error->code) goto cleanup;

    // create swapchain (can be destroyed and recreated many times once configured)
    vulkano_create_swapchain(&vk, error);
    if (error->code) goto cleanup;

    // create syncronization primitives, command buffers, etc. See:
    // `struct vulkano_rendering_state`
    vulkano_create_rendering_state(&vk, error);
    if (error->code) goto cleanup;
cleanup:
    free(required_gpu_extensions.data);
    free(required_validation_layers.data);
    free(required_instance_extensions.data);
    if (!error->code) return vk;

    vulkano_destroy(&vk);
    return vk;
}

void
vulkano_destroy(struct vulkano* vk)
{
    if (vk->device) vkDeviceWaitIdle(vk->device);

    vulkano_destroy_swapchain(vk);
    vulkano_destroy_rendering_state(vk);
    vulkano_destroy_resources(vk);

    if (vk->device) vkDestroyDevice(vk->device, NULL);
    if (vk->surface) vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    if (vk->instance) vkDestroyInstance(vk->instance, NULL);

    *vk = (struct vulkano){0};
}

void
vulkano_create_instance(
    struct vulkano* vk,
    uint32_t validation_layers_count,
    const char** validation_layers,
    uint32_t extensions_count,
    const char** extensions,
    struct vulkano_error* error
)
{
#ifdef VULKANO_DISABLE_VALIDATION_LAYERS
    validation_layers.count = 0;
    validation_layers.data = NULL;
#endif

    LOG("required instance extensions:\n");
    for (uint32_t i = 0; i < extensions_count; i++) LOGF("  %s\n", extensions[i]);
    LOG("\n");

    LOG("required instance validation layers:\n");
    for (uint32_t i = 0; i < validation_layers_count; i++)
        LOGF("  %s\n", validation_layers[i]);
    LOG("\n");

    vulkano_check_validation_layer_support(
        validation_layers_count, validation_layers, error
    );
    if (error->code) return;

    vulkano_check_instance_extension_support(extensions_count, extensions, error);
    if (error->code) return;

    VkInstanceCreateInfo vk_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledLayerCount = validation_layers_count,
        .ppEnabledLayerNames = validation_layers,
        .enabledExtensionCount = extensions_count,
        .ppEnabledExtensionNames = extensions,
    };
    VkResult result;

    if ((result = vkCreateInstance(&vk_create_info, NULL, &vk->instance)) != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_INSTANCE_CREATION_FAILED;
        error->result = result;
        vulkano_write_error_message(error, "failed to create vulkan instance");
        return;
    }
}

const char*
gpu_name(VkPhysicalDevice gpu)
{
    static char buffer[256];
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(gpu, &properties);
    snprintf(buffer, sizeof(buffer), "%s", properties.deviceName);
    return buffer;
}

static bool
confirm_gpu_selection(
    VkSurfaceKHR surface,
    struct vulkano_gpu* gpu,
    uint32_t extension_count,
    const char** extensions
)
{
    // ensure all required gpu extensions are supported
    if (!vulkano_gpu_supports_extensions(gpu->handle, extension_count, extensions)) {
        LOGF(
            "  GPU: [%s] failed requirements check: does not support all required "
            "extensions\n",
            gpu_name(gpu->handle)
        );
        return false;
    }

    // enumerate queue family properties
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu->handle, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_family_properties[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(
        gpu->handle, &queue_family_count, queue_family_properties
    );

    // search for queue family that supports graphics & presentation to our surface
    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 presentation_supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            gpu->handle, i, surface, &presentation_supported
        );
        bool suitable_device =
            presentation_supported &&
            queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

        // if a suitable device is found fill selected queue family information
        // on the gpu struct and return true
        if (suitable_device) {
            gpu->graphics_queue_family = i;
            vkGetPhysicalDeviceMemoryProperties(gpu->handle, &gpu->memory_properties);
            vkGetPhysicalDeviceProperties(gpu->handle, &gpu->properties);
            return true;
        }
    }

    LOGF(
        "  GPU: [%s] failed requirements check: does not support all required "
        "queues\n",
        gpu_name(gpu->handle)
    );
    return false;
}

void
vulkano_select_gpu(
    struct vulkano* vk,
    gpu_compare_function gpucmp,
    present_mode_compare_function presentcmp,
    surface_format_compare_function fmtcmp,
    uint32_t extensions_count,
    const char** extensions,
    struct vulkano_error* error
)
{
    LOG("selecting gpu\n");
    for (uint32_t i = 0; i < extensions_count; i++)
        LOGF("  required extension: %s\n", extensions[i]);

    static const char* ENUMERATION_ERROR_MESSAGE = "failed to enumerate devices gpus";
    VkResult result;

    // start by enumerating all devices available to us
    uint32_t device_count;
    if ((result = vkEnumeratePhysicalDevices(vk->instance, &device_count, NULL)) !=
        VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }
    if (device_count == 0) {
        error->code = VULKANO_ERROR_CODE_NO_GPU_AVAILABLE;
        error->result = result;
        vulkano_write_error_message(error, "vulkan found 0 gpus on system");
        return;
    }
    VkPhysicalDevice devices[device_count];
    if ((result = vkEnumeratePhysicalDevices(vk->instance, &device_count, devices)) !=
        VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }

    // then sort them (this will be worst->best order) based on configured compare
    // function
    qsort(devices, device_count, sizeof(VkPhysicalDevice), (compare_function)gpucmp);
    LOG("  available gpus listed worst->best by configured ranking function:\n");
    for (uint32_t i = 0; i < device_count; i++) LOGF("    %s\n", gpu_name(devices[i]));

    // finally enumerate devices from best->worst and return the first device that
    // meets the requirements
    //
    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice gpu = devices[device_count - 1 - i];
        vk->gpu.handle = gpu;
        vk->gpu.selected_present_mode =
            vulkano_select_present_mode(gpu, vk->surface, presentcmp, error);
        if (error->code) return;
        vk->gpu.selected_swapchain_format =
            vulkano_select_surface_format(gpu, vk->surface, fmtcmp, error);
        if (error->code) return;

        if (confirm_gpu_selection(vk->surface, &vk->gpu, extensions_count, extensions)) {
            LOGF(
                "  configured present mode %s\n",
                present_mode_to_string(vk->gpu.selected_present_mode)
            );
            LOGF(
                "  configured surface format with color format %s\n",
                color_format_to_string(vk->gpu.selected_swapchain_format.format)
            );
            LOGF(
                "  configured surface format with color space %s\n",
                color_space_to_string(vk->gpu.selected_swapchain_format.colorSpace)
            );
            LOGF("selected gpu: %s\n\n", gpu_name(vk->gpu.handle));
            return;
        }
    }

    // set error if we made it through all devices without finding one which meets the
    // configured requirements
    error->code = VULKANO_ERROR_CODE_NO_SUITABLE_GPU_AVAILABLE;
    vulkano_write_error_message(
        error,
        "no gpu available which meets configured requirements, see logs for more info"
    );
}

void
vulkano_create_device(
    struct vulkano* vk,
    uint32_t gpu_extensions_count,
    const char** gpu_extensions,
    struct vulkano_error* error
)
{
    // TODO: support this in config
    struct VkPhysicalDeviceFeatures gpu_features = {0};

    float queue_priorities[] = {1.0};
    struct VkDeviceQueueCreateInfo queue_create_infos[] = {
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities,
        },
    };
    struct VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount =
            sizeof(queue_create_infos) / sizeof(queue_create_infos[0]),
        .pQueueCreateInfos = queue_create_infos,
        .enabledExtensionCount = gpu_extensions_count,
        .ppEnabledExtensionNames = gpu_extensions,
        .pEnabledFeatures = &gpu_features,
    };

    VkResult result = vkCreateDevice(vk->gpu.handle, &create_info, NULL, &vk->device);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_DEVICE_CREATION_FAILED;
        error->result = result;
        vulkano_write_error_message(error, "vulkan logical device creation failed");
        return;
    }

    vkGetDeviceQueue(
        vk->device, vk->gpu.graphics_queue_family, 0, &vk->gpu.graphics_queue
    );
}

VkCommandPool
vulkano_create_command_pool(
    struct vulkano* vk,
    VkCommandPoolCreateFlags flags,
    uint32_t queue_family,
    struct vulkano_error* error
)
{
    struct VkCommandPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queue_family,
    };

    VkCommandPool pool;
    VkResult result = vkCreateCommandPool(vk->device, &info, NULL, &pool);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return VK_NULL_HANDLE;
    }

    return pool;
}

void
vulkano_create_render_passes(
    struct vulkano* vk,
    uint32_t count,
    struct VkRenderPassCreateInfo infos[],
    struct vulkano_error* error
)
{
    if (!count) return;
    vk->resources.render_pass_count = count;
    vk->resources.render_passes = (VkRenderPass*)malloc(sizeof(VkRenderPass) * count);
    if (!vk->resources.render_passes) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return;
    }

    for (uint32_t i = 0; i < count; i++) {
        infos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        // if attachment format is undefined use swapchain format instead
        for (uint32_t attachment = 0; attachment < infos[i].attachmentCount;
             attachment++) {
            VkAttachmentDescription* desc =
                (VkAttachmentDescription*)infos[i].pAttachments + attachment;
            desc->format =
                (desc->format) ? desc->format : vk->gpu.selected_swapchain_format.format;
        }

        VkResult result = vkCreateRenderPass(
            vk->device, infos + i, NULL, vk->resources.render_passes + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create render pass");
            return;
        }
    }

    return;
}

void
vulkano_create_descriptor_set_layouts(
    struct vulkano* vk,
    uint32_t count,
    struct VkDescriptorSetLayoutCreateInfo infos[],
    struct vulkano_error* error
)
{
    if (!count) return;
    vk->resources.descriptor_set_layout_count = count;
    vk->resources.descriptor_set_layouts =
        (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * count);
    if (!vk->resources.descriptor_set_layouts) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return;
    }

    for (uint32_t i = 0; i < count; i++) {
        infos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        VkResult result = vkCreateDescriptorSetLayout(
            vk->device, infos + i, NULL, vk->resources.descriptor_set_layouts + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create descriptor set layout");
            return;
        }
    }
}

void
vulkano_create_pipeline_layouts(
    struct vulkano* vk,
    uint32_t count,
    struct vulkano_pipeline_layout_config configs[],
    struct vulkano_error* error
)
{
    if (!count) return;

    vk->resources.pipeline_layout_count = count;
    vk->resources.pipeline_layouts =
        (VkPipelineLayout*)malloc(sizeof(VkPipelineLayout) * count);
    if (!vk->resources.pipeline_layouts) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return;
    }

    for (uint32_t i = 0; i < count; i++) {
        configs[i].create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        configs[i].create_info.setLayoutCount = configs[i].descriptor_set_layout_count;
        VkResult result;

        if (configs[i].descriptor_set_layout_count) {
            // confirm that we some previously create descriptor set layouts
            //
            if (!vk->resources.descriptor_set_layouts) {
                error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
                error->result = VK_ERROR_UNKNOWN;
                vulkano_write_error_message(
                    error,
                    "pipeline layout indexing descriptor set layouts that don't exist"
                );
                return;
            }

            // set up some stack memory for the set layouts we need to add to the
            // create info
            VkDescriptorSetLayout set_layouts[configs[i].descriptor_set_layout_count];

            for (uint32_t j = 0; j < configs[i].descriptor_set_layout_count; j++) {
                uint32_t set_index = configs[i].descriptor_set_layout_indices[j];

                // check that index is in range for descriptor layouts we have already
                // created
                //
                if (set_index >= vk->resources.descriptor_set_layout_count) {
                    error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
                    error->result = VK_ERROR_UNKNOWN;
                    vulkano_write_error_message(
                        error,
                        "pipeline layout indexing descriptor set layout that's out "
                        "of "
                        "range"
                    );
                    return;
                }

                set_layouts[j] = vk->resources.descriptor_set_layouts[j];
            }

            // create the layout with the described descriptor set layouts
            //
            configs[i].create_info.pSetLayouts = set_layouts;
            result = vkCreatePipelineLayout(
                vk->device,
                &configs[i].create_info,
                NULL,
                vk->resources.pipeline_layouts + i
            );
        }
        else {
            // create the layout with no descriptor set layouts
            //
            result = vkCreatePipelineLayout(
                vk->device,
                &configs[i].create_info,
                NULL,
                vk->resources.pipeline_layouts + i
            );
        }
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create pipeline layout");
            return;
        }
    }
}

// a collection of resources that need to be filled out when processing
// `struct vulkano_graphics_pipeline_config`
//
struct graphics_pipeline_processing {
    struct VkGraphicsPipelineCreateInfo* info;
    struct vulkano_graphics_pipeline* pipeline;
    struct VkPipelineShaderStageCreateInfo* shader_stage_infos;
    struct VkPipelineViewportStateCreateInfo* viewport_state_info;
};

static void
vulkano_add_shader_to_pipeline(
    struct vulkano* vk,
    struct vulkano_data data,
    VkShaderStageFlagBits shader_stage_bit,
    struct graphics_pipeline_processing processing,
    struct vulkano_error* error
)
{
    if (!data.bytes) return;
    if (error->code) return;

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = data.length,
        .pCode = (const uint32_t*)data.bytes,
    };
    VkResult result = vkCreateShaderModule(
        vk->device,
        &create_info,
        NULL,
        processing.pipeline->shader_modules + processing.pipeline->shader_modules_count
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to create shader module");
        return;
    }

    processing.shader_stage_infos[processing.pipeline->shader_modules_count] =
        (struct VkPipelineShaderStageCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = shader_stage_bit,
            .module = processing.pipeline
                          ->shader_modules[processing.pipeline->shader_modules_count++],
            .pName = "main",
        };
}

static void
vulkano_process_pipeline_config(
    struct vulkano* vk,
    struct vulkano_graphics_pipeline_config* config,
    struct graphics_pipeline_processing processing,
    struct vulkano_error* error
)
{
    vulkano_add_shader_to_pipeline(
        vk, config->vertex_shader, VK_SHADER_STAGE_VERTEX_BIT, processing, error
    );
    vulkano_add_shader_to_pipeline(
        vk,
        config->tessellation_control_shader,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        processing,
        error
    );
    vulkano_add_shader_to_pipeline(
        vk,
        config->tessellation_evaluation_shader,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        processing,
        error
    );
    vulkano_add_shader_to_pipeline(
        vk, config->geometry_shader, VK_SHADER_STAGE_GEOMETRY_BIT, processing, error
    );
    vulkano_add_shader_to_pipeline(
        vk, config->fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT, processing, error
    );
    if (error->code) return;

    // fill in structure types
    config->vertex_input.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    config->input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config->tessellation.sType =
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    config->rasterization.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config->multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config->depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config->blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config->dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    // fill in defaults
    DEFAULT0(config->multisample.rasterizationSamples, VK_SAMPLE_COUNT_1_BIT);

    // validate index based resources
    if (!vk->resources.pipeline_layouts ||
        vk->resources.pipeline_layout_count <= config->pipeline_layout_index) {
        error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, "pipeline layout index out of range");
        return;
    }
    if (!vk->resources.render_passes ||
        vk->resources.render_pass_count <= config->render_pass_index) {
        error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, "render pass index out of range");
        return;
    }

    *processing.viewport_state_info = (struct VkPipelineViewportStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &config->viewport,
        .scissorCount = 1,
        .pScissors = &config->scissor,
    };

    // fill in pipeline create info
    processing.info->stageCount = processing.pipeline->shader_modules_count;
    processing.info->pStages = processing.shader_stage_infos;
    processing.info->pVertexInputState = &config->vertex_input;
    processing.info->pInputAssemblyState = &config->input_assembly;
    processing.info->pTessellationState = &config->tessellation;
    processing.info->pViewportState = processing.viewport_state_info;
    processing.info->pRasterizationState = &config->rasterization;
    processing.info->pMultisampleState = &config->multisample;
    processing.info->pDepthStencilState = &config->depth;
    processing.info->pColorBlendState = &config->blend;
    processing.info->pDynamicState = &config->dynamic;
    processing.info->layout =
        vk->resources.pipeline_layouts[config->pipeline_layout_index];
    processing.info->renderPass = vk->resources.render_passes[config->render_pass_index];
    processing.info->subpass = config->subpass;
    processing.info->basePipelineHandle = 0;
    processing.info->basePipelineIndex = config->base_pipeline_index;
}

void
vulkano_create_graphics_pipelines(
    struct vulkano* vk,
    uint32_t count,
    struct vulkano_graphics_pipeline_config* configs,
    struct vulkano_error* error
)
{
    if (!count) return;

    // set up heap memory for graphics pipelines
    vk->resources.graphics_pipeline_count = count;
    vk->resources.graphics_pipelines = (struct vulkano_graphics_pipeline*)calloc(
        count, sizeof(struct vulkano_graphics_pipeline)
    );
    if (!vk->resources.graphics_pipelines) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return;
    }

    // set up some stack memory for the structs we need to fill out
    struct VkGraphicsPipelineCreateInfo infos[count];
    struct VkPipelineShaderStageCreateInfo shader_stage_infos[count]
                                                             [VULKANO_MAX_SHADER_STAGES];
    struct VkPipelineViewportStateCreateInfo viewport_state_infos[count];
    VkPipeline handles[count];

    // fill out each create info struct
    for (uint32_t i = 0; i < count; i++) {
        infos[i] = (struct VkGraphicsPipelineCreateInfo){
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        };
        viewport_state_infos[i] = (struct VkPipelineViewportStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        };
        for (uint32_t j = 0; j < VULKANO_MAX_SHADER_STAGES; j++)
            shader_stage_infos[i][j] = (struct VkPipelineShaderStageCreateInfo){
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            };

        vulkano_process_pipeline_config(
            vk,
            configs + i,
            // pass in a collection of pointers to our stack memory above
            (struct graphics_pipeline_processing){
                .info = infos + i,
                .pipeline = vk->resources.graphics_pipelines + i,
                .shader_stage_infos = shader_stage_infos[i],
                .viewport_state_info = viewport_state_infos + i,
            },
            error
        );
        if (error->code) return;
    }

    // create pipelines
    VkResult result =
        vkCreateGraphicsPipelines(vk->device, NULL, count, infos, NULL, handles);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to create graphics pipelines");
        return;
    }

    // copy handles into the vulkano struct
    for (uint32_t i = 0; i < count; i++)
        vk->resources.graphics_pipelines[i].handle = handles[i];
}

void
vulkano_create_descriptor_pools(
    struct vulkano* vk,
    uint32_t count,
    VkDescriptorPoolCreateInfo infos[],
    struct vulkano_error* error
)
{
    vk->resources.descriptor_pool_count = count;
    vk->resources.descriptor_pools =
        (VkDescriptorPool*)malloc(sizeof(VkDescriptorPool) * count);
    if (!vk->resources.descriptor_pools) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        return;
    }

    for (uint32_t i = 0; i < count; i++) {
        infos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

        VkResult result = vkCreateDescriptorPool(
            vk->device, infos + i, NULL, vk->resources.descriptor_pools + i
        );
        if (result != VK_SUCCESS) {
            vulkano_out_of_memory(error, result);
            return;
        }
    }
}

void
vulkano_create_resources(
    struct vulkano* vk, struct vulkano_config* config, struct vulkano_error* error
)
{
    vk->resources.command_pool = vulkano_create_command_pool(
        vk,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        vk->gpu.graphics_queue_family,
        error
    );
    if (error->code) return;

    vulkano_create_render_passes(
        vk, config->render_pass_count, config->render_passes, error
    );
    if (error->code) return;

    vulkano_create_descriptor_set_layouts(
        vk, config->descriptor_set_layout_count, config->descriptor_set_layouts, error
    );
    if (error->code) return;

    vulkano_create_pipeline_layouts(
        vk, config->pipeline_layout_count, config->pipeline_layout_configs, error
    );
    if (error->code) return;

    vulkano_create_graphics_pipelines(
        vk, config->pipeline_count, config->pipeline_configs, error
    );
    if (error->code) return;

    vulkano_create_descriptor_pools(
        vk, config->descriptor_pool_count, config->descriptor_pools, error
    );
    if (error->code) return;
}

void
vulkano_destroy_resources(struct vulkano* vk)
{
    if (vk->resources.render_passes) {
        for (uint32_t i = 0; i < vk->resources.render_pass_count; i++)
            if (vk->resources.render_passes[i])
                vkDestroyRenderPass(vk->device, vk->resources.render_passes[i], NULL);
        free(vk->resources.render_passes);
    }

    if (vk->resources.descriptor_set_layouts) {
        for (uint32_t i = 0; i < vk->resources.descriptor_set_layout_count; i++) {
            if (vk->resources.descriptor_set_layouts[i]) {
                vkDestroyDescriptorSetLayout(
                    vk->device, vk->resources.descriptor_set_layouts[i], NULL
                );
            }
        }
        free(vk->resources.descriptor_set_layouts);
    }

    if (vk->resources.pipeline_layouts) {
        for (uint32_t i = 0; i < vk->resources.pipeline_layout_count; i++) {
            if (vk->resources.pipeline_layouts[i]) {
                vkDestroyPipelineLayout(
                    vk->device, vk->resources.pipeline_layouts[i], NULL
                );
            }
        }
        free(vk->resources.pipeline_layouts);
    }

    if (vk->resources.graphics_pipelines) {
        for (uint32_t i = 0; i < vk->resources.graphics_pipeline_count; i++) {
            for (uint32_t shader = 0;
                 shader < vk->resources.graphics_pipelines[i].shader_modules_count;
                 shader++) {
                if (vk->resources.graphics_pipelines[i].shader_modules[shader]) {
                    vkDestroyShaderModule(
                        vk->device,
                        vk->resources.graphics_pipelines[i].shader_modules[shader],
                        NULL
                    );
                }
            }
            if (vk->resources.graphics_pipelines[i].handle) {
                vkDestroyPipeline(
                    vk->device, vk->resources.graphics_pipelines[i].handle, NULL
                );
            }
        }
        free(vk->resources.graphics_pipelines);
    }

    if (vk->resources.descriptor_pools) {
        for (uint32_t i = 0; i < vk->resources.descriptor_pool_count; i++) {
            if (vk->resources.descriptor_pools[i])
                vkDestroyDescriptorPool(
                    vk->device, vk->resources.descriptor_pools[i], NULL
                );
        }
        free(vk->resources.descriptor_pools);
    }

    if (vk->resources.command_pool)
        vkDestroyCommandPool(vk->device, vk->resources.command_pool, NULL);

    vk->resources = (struct vulkano_resources){0};
}

void
vulkano_configure_swapchain(
    struct vulkano* vk,
    query_size_function query_size,
    uint32_t image_count,
    uint32_t render_pass_index,
    struct vulkano_error* error
)
{
    assert(
        vk->resources.render_passes &&
        "render passes must be created before this function call"
    );

    // query surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vk->gpu.handle, vk->surface, &capabilities
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to query surface capabilities");
        return;
    }

    // fill out initial swapchain configuration
    vk->swapchain.query_size = query_size;

    // validate render pass index
    if (render_pass_index >= vk->resources.render_pass_count) {
        error->code = VULKANO_ERROR_CODE_BAD_CONFIGURATION;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(
            error, "render_pass_index out of range during swapchain configuration"
        );
        return;
    }
    vk->swapchain.render_pass = vk->resources.render_passes[render_pass_index];

    // validate image count
    if (image_count < capabilities.minImageCount ||
        image_count > capabilities.maxImageCount) {
        error->code = VULKANO_ERROR_CODE_SWAPCHAIN_CONFIGURATION_FAILED;
        error->result = VK_ERROR_UNKNOWN;
        LOGF(
            "min_image_count: %u max_image_count %u\n",
            capabilities.minImageCount,
            capabilities.maxImageCount
        );
        vulkano_write_error_message(error, "swapchain image count not supported");
        return;
    }
    vk->swapchain.image_count = image_count;

    return;
}

void
vulkano_create_swapchain(struct vulkano* vk, struct vulkano_error* error)
{
    // query surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vk->gpu.handle, vk->surface, &capabilities
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to query surface capabilities");
        return;
    }

    // select swapchain extent
    struct VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == 0xFFFFFFFF) {
        uint32_t width, height;
        vk->swapchain.query_size(&width, &height);
        extent.width = CLAMP(
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width, width
        );
        extent.height = CLAMP(
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height, height
        );
    }
    vk->swapchain.extent = extent;
    LOGF(
        "creating swapchain with extent (%u, %u)\n",
        vk->swapchain.extent.width,
        vk->swapchain.extent.height
    );

    // create swapchain
    struct VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk->surface,
        .minImageCount = vk->swapchain.image_count,
        .imageFormat = vk->gpu.selected_swapchain_format.format,
        .imageColorSpace = vk->gpu.selected_swapchain_format.colorSpace,
        .imageExtent = vk->swapchain.extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = vk->gpu.selected_present_mode,
        .clipped = VK_TRUE,
    };
    result =
        vkCreateSwapchainKHR(vk->device, &swapchain_info, NULL, &vk->swapchain.handle);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_SWAPCHAIN_CREATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to create vulkan swapchain");
        return;
    }

    // enumerate images
    VkImage images[vk->swapchain.image_count];
    result = vkGetSwapchainImagesKHR(
        vk->device, vk->swapchain.handle, &vk->swapchain.image_count, images
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to enumerate swapchain images");
        vulkano_destroy_swapchain(vk);
        return;
    }

    // allocate memory for framebuffer and image view handles
    vk->swapchain.framebuffers =
        (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * vk->swapchain.image_count);
    vk->swapchain.image_views =
        (VkImageView*)malloc(sizeof(VkImageView) * vk->swapchain.image_count);
    if (!vk->swapchain.framebuffers || !vk->swapchain.image_views) {
        vulkano_out_of_memory(error, VK_ERROR_OUT_OF_HOST_MEMORY);
        vulkano_destroy_swapchain(vk);
        return;
    }

    // create swapchain image views and framebuffers from the enuemrated images
    for (uint32_t i = 0; i < vk->swapchain.image_count; i++) {
        // TODO: support additional options here in configuration
        VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = vk->gpu.selected_swapchain_format.format,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };
        result = vkCreateImageView(
            vk->device, &image_view_info, NULL, vk->swapchain.image_views + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(
                error, "failed to create image views for swapchain"
            );
            vulkano_destroy_swapchain(vk);
            return;
        }

        // TODO: support additional options here in configuration
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vk->swapchain.render_pass,
            .attachmentCount = 1,
            .pAttachments = vk->swapchain.image_views + i,
            .width = vk->swapchain.extent.width,
            .height = vk->swapchain.extent.height,
            .layers = 1,
        };
        result = vkCreateFramebuffer(
            vk->device, &framebuffer_info, NULL, vk->swapchain.framebuffers + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(
                error, "failed to create framebuffers for swapchain"
            );
            vulkano_destroy_swapchain(vk);
            return;
        }
    }
}

void
vulkano_destroy_swapchain(struct vulkano* vk)
{
    // cleanup image views
    if (vk->swapchain.image_views) {
        for (uint32_t i = 0; i < vk->swapchain.image_count; i++) {
            if (vk->swapchain.image_views[i])
                vkDestroyImageView(vk->device, vk->swapchain.image_views[i], NULL);
        }
        free(vk->swapchain.image_views);
        vk->swapchain.image_views = NULL;
    }
    // cleanup framebuffers
    if (vk->swapchain.framebuffers) {
        for (uint32_t i = 0; i < vk->swapchain.image_count; i++) {
            if (vk->swapchain.framebuffers[i])
                vkDestroyFramebuffer(vk->device, vk->swapchain.framebuffers[i], NULL);
        }
        free(vk->swapchain.framebuffers);
        vk->swapchain.framebuffers = NULL;
    }

    // cleanup swapchain
    if (vk->swapchain.handle)
        vkDestroySwapchainKHR(vk->device, vk->swapchain.handle, NULL);
    vk->swapchain.handle = VK_NULL_HANDLE;
}

void
vulkano_create_rendering_state(struct vulkano* vk, struct vulkano_error* error)
{
    assert(vk->device);
    assert(vk->gpu.handle);

    struct VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk->resources.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    struct VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    struct VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (uint32_t i = 0; i < VULKANO_CONCURRENT_FRAMES; i++) {
        VkResult result = vkAllocateCommandBuffers(
            vk->device, &allocate_info, vk->rendering.command_buffers + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_ALLOCATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to allocate command buffer");
            return;
        }

        result = vkCreateSemaphore(
            vk->device, &semaphore_info, NULL, vk->rendering.image_ready + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create vulkan semaphore");
            return;
        }

        result = vkCreateSemaphore(
            vk->device, &semaphore_info, NULL, vk->rendering.rendering_complete + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create vulkan semaphore");
            return;
        }

        result = vkCreateFence(
            vk->device, &fence_info, NULL, vk->rendering.frame_complete + i
        );
        if (result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(error, "failed to create vulkan fence");
            return;
        }
    }
}

void
vulkano_destroy_rendering_state(struct vulkano* vk)
{
    for (uint32_t i = 0; i < VULKANO_CONCURRENT_FRAMES; i++) {
        if (vk->rendering.image_ready[i])
            vkDestroySemaphore(vk->device, vk->rendering.image_ready[i], NULL);
        if (vk->rendering.rendering_complete[i])
            vkDestroySemaphore(vk->device, vk->rendering.rendering_complete[i], NULL);
        if (vk->rendering.frame_complete[i])
            vkDestroyFence(vk->device, vk->rendering.frame_complete[i], NULL);
    }
    vk->rendering = (struct vulkano_rendering_state){0};
}

VkCommandBuffer
vulkano_acquire_single_use_command_buffer(struct vulkano* vk, struct vulkano_error* error)
{
    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo allocation_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk->resources.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkResult result = vkAllocateCommandBuffers(vk->device, &allocation_info, &cmd);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return cmd;
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    result = vkBeginCommandBuffer(cmd, &begin_info);
    if (result != VK_SUCCESS) {
        vkFreeCommandBuffers(vk->device, vk->resources.command_pool, 1, &cmd);
        vulkano_out_of_memory(error, result);
        return cmd;
    }

    return cmd;
}

void
vulkano_submit_single_use_command_buffer(
    struct vulkano* vk, VkCommandBuffer cmd, struct vulkano_error* error
)
{
    VkResult result;

    static const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    VkFence fence = VK_NULL_HANDLE;

    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    result = vkCreateFence(vk->device, &fence_info, NULL, &fence);
    if (result != VK_SUCCESS) goto cleanup;

    result = vkEndCommandBuffer(cmd);
    if (result != VK_SUCCESS) goto cleanup;

    result = vkQueueSubmit(vk->gpu.graphics_queue, 1, &submit, fence);
    if (result != VK_SUCCESS) goto cleanup;

    result = vkWaitForFences(vk->device, 1, &fence, VK_TRUE, VULKANO_TIMEOUT);
    if (result != VK_SUCCESS) goto cleanup;

cleanup:
    vkFreeCommandBuffers(vk->device, vk->resources.command_pool, 1, &cmd);
    if (fence) vkDestroyFence(vk->device, fence, NULL);

    if (result == VK_SUCCESS) return;

    if (result == VK_TIMEOUT) {
        error->code = VULKANO_ERROR_CODE_TIMEOUT;
        error->result = result;
        vulkano_write_error_message(
            error, "timeout exceeded submitting single use command buffer"
        );
        return;
    }

    if (IS_VULKAN_MEMORY_ERROR(result)) {
        vulkano_out_of_memory(error, result);
        return;
    }

    vulkano_fatal_error(error, result);
}

void
vulkano_begin_frame(
    struct vulkano* vk, struct vulkano_frame* frame, struct vulkano_error* error
)
{
    uint64_t frame_index = vk->rendering.frame_count % VULKANO_CONCURRENT_FRAMES;

    frame->command_buffer = vk->rendering.command_buffers[frame_index];
    frame->number = vk->rendering.frame_count;

    VkResult result = vkWaitForFences(
        vk->device,
        1,
        vk->rendering.frame_complete + frame_index,
        VK_TRUE,
        VULKANO_TIMEOUT
    );
    if (result == VK_TIMEOUT) {
        error->code = VULKANO_ERROR_CODE_TIMEOUT;
        error->result = result;
        vulkano_write_error_message(error, "timeout waiting for frame complete fence");
        return;
    }
    if (IS_VULKAN_MEMORY_ERROR(result)) {
        vulkano_out_of_memory(error, result);
        return;
    }
    if (result != VK_SUCCESS) {
        vulkano_fatal_error(error, result);
        return;
    }

    result = vkResetFences(vk->device, 1, vk->rendering.frame_complete + frame_index);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }
    result = vkResetCommandBuffer(frame->command_buffer, 0);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }

acquire_image:
    result = vkAcquireNextImageKHR(
        vk->device,
        vk->swapchain.handle,
        VULKANO_TIMEOUT,
        vk->rendering.image_ready[frame_index],
        VK_NULL_HANDLE,
        &frame->image_index
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vulkano_destroy_swapchain(vk);
        vulkano_create_swapchain(vk, error);
        if (error->code) return;
        vkDestroySemaphore(vk->device, vk->rendering.image_ready[frame_index], NULL);
        struct VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VkResult semaphore_result = vkCreateSemaphore(
            vk->device, &semaphore_info, NULL, vk->rendering.image_ready + frame_index
        );
        if (semaphore_result != VK_SUCCESS) {
            error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
            error->result = result;
            vulkano_write_error_message(
                error, "failed recreating semaphore after swapchain recreation"
            );
            return;
        }
        goto acquire_image;
    }
    if (result == VK_TIMEOUT) {
        error->code = VULKANO_ERROR_CODE_TIMEOUT;
        error->result = result;
        vulkano_write_error_message(error, "timeout trying to acquire swapchain image");
        return;
    }
    if (IS_VULKAN_MEMORY_ERROR(result)) {
        vulkano_out_of_memory(error, result);
        return;
    }
    if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
        // TODO: handle this case
    }
    if (result != VK_SUCCESS) {
        vulkano_fatal_error(error, result);
        return;
    }

    frame->framebuffer = vk->swapchain.framebuffers[frame->image_index];

    struct VkCommandBufferBeginInfo command_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    result = vkBeginCommandBuffer(frame->command_buffer, &command_begin_info);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }

    VkClearValue clear = {
        frame->clear[0], frame->clear[1], frame->clear[2], frame->clear[3]};
    struct VkRenderPassBeginInfo render_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = VULKANO_RENDER_PASS(vk, 0),
        .framebuffer = frame->framebuffer,
        .renderArea.extent = vk->swapchain.extent,
        .clearValueCount = 1,
        .pClearValues = &clear,
    };
    vkCmdBeginRenderPass(
        frame->command_buffer, &render_begin_info, VK_SUBPASS_CONTENTS_INLINE
    );
}

void
vulkano_submit_frame(
    struct vulkano* vk, struct vulkano_frame* frame, struct vulkano_error* error
)
{
    uint64_t frame_index = vk->rendering.frame_count++ % 2;

    vkCmdEndRenderPass(frame->command_buffer);
    VkResult result = vkEndCommandBuffer(frame->command_buffer);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }

    VkPipelineStageFlags wait_mask[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    struct VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = vk->rendering.image_ready + frame_index,
        .pWaitDstStageMask = wait_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = vk->rendering.rendering_complete + frame_index,
    };

    result = vkQueueSubmit(
        vk->gpu.graphics_queue, 1, &submit_info, vk->rendering.frame_complete[frame_index]
    );
    if (IS_VULKAN_MEMORY_ERROR(result)) {
        vulkano_out_of_memory(error, result);
        return;
    }
    if (result != VK_SUCCESS) {
        vulkano_fatal_error(error, result);
        return;
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = vk->rendering.rendering_complete + frame_index,
        .swapchainCount = 1,
        .pSwapchains = &vk->swapchain.handle,
        .pImageIndices = &frame->image_index,
    };
    result = vkQueuePresentKHR(vk->gpu.graphics_queue, &present_info);
    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // next frame will recreate the swapchain,
        // this is not yet an error state
        return;
    }
    if (IS_VULKAN_MEMORY_ERROR(result)) {
        vulkano_out_of_memory(error, result);
        return;
    }
    if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
        // FIXME: handle this case
    }

    error->code = VULKANO_ERROR_CODE_FATAL_ERROR;
    error->result = result;
    vulkano_write_error_message(error, "failed to present image");
}

static uint32_t
select_memory_type(
    struct vulkano_gpu gpu,
    uint32_t required_memory_type_bits,
    VkMemoryPropertyFlags required_memory_property_flags,
    struct vulkano_error* error
)
{
    for (uint32_t i = 0; i < gpu.memory_properties.memoryTypeCount; i++) {
        uint32_t memory_type_check = (1 << i) & required_memory_type_bits;
        uint32_t memory_properties_check =
            required_memory_property_flags ==
            (gpu.memory_properties.memoryTypes[i].propertyFlags &
             required_memory_property_flags);

        if (memory_type_check && memory_properties_check) {
            return i;
        }
    }

    error->code = VULKANO_ERROR_CODE_MEMORY_REQUIREMENTS_UNFULFILLED;
    error->result = VK_ERROR_UNKNOWN;
    vulkano_write_error_message(
        error, "unable to find memory fitting the given requriements"
    );
    return 0;
}

struct vulkano_buffer
vulkano_create_buffer(
    struct vulkano* vk,
    uint32_t capacity,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags required_memory_properties,
    struct vulkano_error* error
)
{
    struct vulkano_buffer buffer = {
        .usage = usage,
        .capacity = capacity,
    };

    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = capacity,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkResult result = vkCreateBuffer(vk->device, &create_info, NULL, &buffer.handle);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_CREATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to create buffer");
        return buffer;
    }

    struct VkMemoryRequirements requirements = {0};
    vkGetBufferMemoryRequirements(vk->device, buffer.handle, &requirements);

    uint32_t memory_type_index = select_memory_type(
        vk->gpu, requirements.memoryTypeBits, required_memory_properties, error
    );
    if (error->code) {
        vkDestroyBuffer(vk->device, buffer.handle, NULL);
        return buffer;
    }
    buffer.memory_flags =
        vk->gpu.memory_properties.memoryTypes[memory_type_index].propertyFlags;

    struct VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memory_type_index,
    };
    result = vkAllocateMemory(vk->device, &allocate_info, NULL, &buffer.memory);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_ALLOCATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to allocate new buffer memory");
        vkDestroyBuffer(vk->device, buffer.handle, NULL);
        return buffer;
    }

    result = vkBindBufferMemory(vk->device, buffer.handle, buffer.memory, 0);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_RESOURCE_ALLOCATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to bind allocated memory to buffer");
        vkDestroyBuffer(vk->device, buffer.handle, NULL);
        vkFreeMemory(vk->device, buffer.memory, NULL);
        return buffer;
    }

    return buffer;
}

void
vulkano_destroy_buffer(struct vulkano* vk, struct vulkano_buffer* buffer)
{
    if (!buffer) return;
    if (buffer->handle) vkDestroyBuffer(vk->device, buffer->handle, NULL);
    if (buffer->memory) vkFreeMemory(vk->device, buffer->memory, NULL);
    *buffer = (struct vulkano_buffer){0};
}

static void
vulkano_copy_to_buffer_host_coherent(
    struct vulkano* vk,
    struct vulkano_buffer* buffer,
    struct vulkano_data data,
    struct vulkano_error* error
)
{
    void* memory;
    VkResult result =
        vkMapMemory(vk->device, buffer->memory, 0, VK_WHOLE_SIZE, 0, &memory);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }
    memcpy(memory, data.bytes, data.length);
    vkUnmapMemory(vk->device, buffer->memory);
}

static void
vulkano_copy_to_buffer_host_visible(
    struct vulkano* vk,
    struct vulkano_buffer* buffer,
    struct vulkano_data data,
    struct vulkano_error* error
)
{
    void* memory;
    VkResult result =
        vkMapMemory(vk->device, buffer->memory, 0, VK_WHOLE_SIZE, 0, &memory);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }
    memcpy(memory, data.bytes, data.length);

    struct VkMappedMemoryRange range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = buffer->memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };
    VkResult flush_result = vkFlushMappedMemoryRanges(vk->device, 1, &range);
    VkResult invalidate_result = vkInvalidateMappedMemoryRanges(vk->device, 1, &range);
    vkUnmapMemory(vk->device, buffer->memory);

    result = (flush_result != VK_SUCCESS) ? flush_result : invalidate_result;
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return;
    }
}

static void
vulkano_copy_to_buffer_device_local(
    struct vulkano* vk,
    struct vulkano_buffer* buffer,
    struct vulkano_data data,
    struct vulkano_error* error
)
{
    if (!(buffer->usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)) {
        error->code = VULKANO_ERROR_CODE_VALIDATION;
        vulkano_write_error_message(
            error,
            "trying to copy to a buffer without VK_BUFFER_USAGE_TRANSFER_DST_BIT in "
            "usage"
        );
        return;
    }

    struct vulkano_buffer transfer_buffer = {0};
    VkCommandBuffer cmd = VK_NULL_HANDLE;

    transfer_buffer = vulkano_create_buffer(
        vk,
        data.length,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        error
    );
    if (error->code) return;

    vulkano_copy_to_buffer(vk, &transfer_buffer, data, error);
    if (error->code) goto cleanup;

    cmd = vulkano_acquire_single_use_command_buffer(vk, error);
    if (error->code) goto cleanup;

    vkCmdCopyBuffer(
        cmd,
        transfer_buffer.handle,
        buffer->handle,
        1,
        (struct VkBufferCopy[]){{.size = data.length}}
    );

    vulkano_submit_single_use_command_buffer(vk, cmd, error);
    if (error->code) goto cleanup;

cleanup:
    vulkano_destroy_buffer(vk, &transfer_buffer);
}

void
vulkano_copy_to_buffer(
    struct vulkano* vk,
    struct vulkano_buffer* buffer,
    struct vulkano_data data,
    struct vulkano_error* error
)
{
    if (buffer->capacity < data.length) {
        error->code = VULKANO_ERROR_CODE_VALIDATION;
        vulkano_write_error_message(error, "overflowing copy operation requested");
        return;
    }

    if (buffer->memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        vulkano_copy_to_buffer_host_coherent(vk, buffer, data, error);
        return;
    }
    else if (buffer->memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vulkano_copy_to_buffer_host_visible(vk, buffer, data, error);
        return;
    }
    else {
        vulkano_copy_to_buffer_device_local(vk, buffer, data, error);
        return;
    }
}

struct vulkano_image
vulkano_create_image(
    struct vulkano* vk,
    struct VkImageCreateInfo info,
    VkMemoryPropertyFlags memory_flags,
    struct vulkano_error* error
)
{
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    DEFAULT0(info.imageType, VK_IMAGE_TYPE_2D);
    DEFAULT0(info.extent.depth, 1);
    DEFAULT0(info.mipLevels, 1);
    DEFAULT0(info.arrayLayers, 1);
    DEFAULT0(info.format, vk->gpu.selected_swapchain_format.format);
    DEFAULT0(info.samples, VK_SAMPLE_COUNT_1_BIT);

    struct vulkano_image image = {.layout = info.initialLayout};

    VkResult result = vkCreateImage(vk->device, &info, NULL, &image.handle);
    if (result != VK_SUCCESS) {
        vulkano_out_of_memory(error, result);
        return image;
    }

    struct VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vk->device, image.handle, &requirements);
    uint32_t memory_type =
        select_memory_type(vk->gpu, requirements.memoryTypeBits, memory_flags, error);
    if (error->code) {
        vulkano_destroy_image(vk, &image);
        return image;
    }

    VkMemoryAllocateInfo allocation_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memory_type,
    };
    result = vkAllocateMemory(vk->device, &allocation_info, NULL, &image.memory);
    if (result != VK_SUCCESS) {
        vulkano_destroy_image(vk, &image);
        vulkano_out_of_memory(error, result);
        return image;
    }

    result = vkBindImageMemory(vk->device, image.handle, image.memory, 0);
    if (result != VK_SUCCESS) {
        vulkano_destroy_image(vk, &image);
        vulkano_out_of_memory(error, result);
        return image;
    }

    return image;
}

void
vulkano_destroy_image(struct vulkano* vk, struct vulkano_image* image)
{
    if (!image) return;
    if (image->memory) vkFreeMemory(vk->device, image->memory, NULL);
    if (image->handle) vkDestroyImage(vk->device, image->handle, NULL);
    *image = (struct vulkano_image){0};
}

void
vulkano_image_change_layout(
    struct vulkano* vk,
    struct vulkano_image* image,
    VkImageLayout layout,
    struct vulkano_error* error
)
{
    struct VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = image->layout,
        .newLayout = layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->handle,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.levelCount = 1,
        .subresourceRange.layerCount = 1,
    };

    VkPipelineStageFlags dest_stage_flags;
    VkPipelineStageFlags source_stage_flags;

    if (image->layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dest_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (image->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dest_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        error->code = VULKANO_ERROR_CODE_VALIDATION;
        vulkano_write_error_message(error, "unimplemented layout transition");
        return;
    }

    image->layout = layout;

    VkCommandBuffer cmd = vulkano_acquire_single_use_command_buffer(vk, error);
    if (error->code) return;

    vkCmdPipelineBarrier(
        cmd, source_stage_flags, dest_stage_flags, 0, 0, NULL, 0, NULL, 1, &barrier
    );

    vulkano_submit_single_use_command_buffer(vk, cmd, error);
    if (error->code) return;
}

void
vulkano_copy_to_image(
    struct vulkano* vk,
    struct vulkano_image* image,
    struct vulkano_data data,
    uint32_t width,
    uint32_t height,
    struct vulkano_error* error
)
{
    struct vulkano_buffer transfer_buffer = {0};
    VkCommandBuffer cmd;

    // TODO: store some data on vulkano_image struct to fill out this struct more
    // generally
    struct VkBufferImageCopy copy_info = {
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.layerCount = 1,
        .imageExtent = {width, height, 1},
    };

    transfer_buffer = vulkano_create_buffer(
        vk,
        data.length,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        error
    );
    if (error->code) goto cleanup;

    cmd = vulkano_acquire_single_use_command_buffer(vk, error);
    if (error->code) goto cleanup;

    vulkano_copy_to_buffer(vk, &transfer_buffer, data, error);
    if (error->code) goto cleanup;

    vkCmdCopyBufferToImage(
        cmd, transfer_buffer.handle, image->handle, image->layout, 1, &copy_info
    );

    vulkano_submit_single_use_command_buffer(vk, cmd, error);
    if (error->code) goto cleanup;

cleanup:
    vulkano_destroy_buffer(vk, &transfer_buffer);
}

struct VkSurfaceFormatKHR
vulkano_select_surface_format(
    VkPhysicalDevice gpu,
    VkSurfaceKHR surface,
    surface_format_compare_function cmp,
    struct vulkano_error* error
)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result =
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to query surface capabilities");
        return (struct VkSurfaceFormatKHR){.format = VK_FORMAT_UNDEFINED};
    }

    static const char* ENUMERATE_SURFACE_FORMATS_ERROR_MESSAGE =
        "failed to enumerate vulkan surface formats";
    uint32_t formats_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formats_count, NULL);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATE_SURFACE_FORMATS_ERROR_MESSAGE);
        return (struct VkSurfaceFormatKHR){.format = VK_FORMAT_UNDEFINED};
    }
    struct VkSurfaceFormatKHR surface_formats[formats_count];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        gpu, surface, &formats_count, surface_formats
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATE_SURFACE_FORMATS_ERROR_MESSAGE);
        return (struct VkSurfaceFormatKHR){.format = VK_FORMAT_UNDEFINED};
    }
    qsort(  // sorted worst->best
        surface_formats,
        formats_count,
        sizeof(struct VkSurfaceFormatKHR),
        (compare_function)cmp
    );
    struct VkSurfaceFormatKHR surface_format = surface_formats[formats_count - 1];
    return surface_format;
}

VkPresentModeKHR
vulkano_select_present_mode(
    VkPhysicalDevice gpu,
    VkSurfaceKHR surface,
    present_mode_compare_function cmp,
    struct vulkano_error* error
)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result =
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilities);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, "failed to query surface capabilities");
        return (VkPresentModeKHR)0;
    }

    static const char* ENUMERATE_PRESENT_MODES_ERROR_MESSAGE =
        "failed to enumerate vulkan surface present modes";
    uint32_t present_modes_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        gpu, surface, &present_modes_count, NULL
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATE_PRESENT_MODES_ERROR_MESSAGE);
        return (VkPresentModeKHR)0;
    }
    VkPresentModeKHR present_modes[present_modes_count];
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        gpu, surface, &present_modes_count, present_modes
    );
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATE_PRESENT_MODES_ERROR_MESSAGE);
        return (VkPresentModeKHR)0;
    }
    qsort(  // sorted worst->best
        present_modes,
        present_modes_count,
        sizeof(VkPresentModeKHR),
        (compare_function)cmp
    );
    VkPresentModeKHR present_mode = present_modes[present_modes_count - 1];
    return present_mode;
}

static int
compare_layer_properties_name(VkLayerProperties* prop1, VkLayerProperties* prop2)
{
    return strcmp(prop1->layerName, prop2->layerName);
}

void
vulkano_check_validation_layer_support(
    uint32_t count, const char** layers, struct vulkano_error* error
)
{
    static const char* ENUMERATION_ERROR_MESSAGE =
        "failed to enumerate supported vulkan validation layers";
    VkResult result;

    // enumerate supported validation layers
    uint32_t available_layers_count;
    result = vkEnumerateInstanceLayerProperties(&available_layers_count, NULL);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }

    VkLayerProperties available_layers[available_layers_count];
    result =
        vkEnumerateInstanceLayerProperties(&available_layers_count, available_layers);
    if (result != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }

    // sort for bsearch
    qsort(
        available_layers,
        available_layers_count,
        sizeof(VkLayerProperties),
        (compare_function)compare_layer_properties_name
    );

    // check if all requested layers are available
    unsigned unsupported = 0;
    for (uint32_t i = 0; i < count; i++) {
        VkLayerProperties requested = {0};
        snprintf(requested.layerName, sizeof(requested.layerName), "%s", layers[i]);
        bool layer_is_supported = (bool)bsearch(
            &requested,
            available_layers,
            available_layers_count,
            sizeof(VkLayerProperties),
            (compare_function)compare_layer_properties_name
        );
        if (!layer_is_supported) {
            unsupported++;
            LOGF("ERROR: unsupported vulkan validation layer: %s\n", layers[i]);
        }
    }

    // error if any layers were not supported
    if (unsupported) {
        error->code = VULKANO_ERROR_CODE_UNSUPPORTED_VALIDATION_LAYER;
        error->result = VK_ERROR_LAYER_NOT_PRESENT;
        vulkano_write_error_message(error, "unsupported validation layers");
        return;
    }
}

static uint64_t
score_gpu(VkPhysicalDevice gpu)
{
    uint64_t score;

    // score gpus in tiers where Discrete > Integrated > Virtual reguardless of other
    // properties
    {
        static const uint64_t DISCRETE = (uint64_t)1 << 63;
        static const uint64_t INTEGRATED = (uint64_t)1 << 62;
        static const uint64_t VIRTUAL = (uint64_t)1 << 61;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(gpu, &properties);

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score = DISCRETE;
        else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            score = INTEGRATED;
        else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            score = VIRTUAL;
        else
            score = 0;
    }

    // score gpus within the same tier based on vram
    {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);

        for (size_t i = 0; i < memory_properties.memoryHeapCount; i++) {
            VkMemoryHeap heap = memory_properties.memoryHeaps[i];
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                score += heap.size;
                break;
            }
        }
    }

    return score;
}

int
default_gpu_compare(VkPhysicalDevice* gpu1, VkPhysicalDevice* gpu2)
{
    uint64_t score1 = score_gpu(*gpu1);
    uint64_t score2 = score_gpu(*gpu2);
    if (score1 == score2) return 0;
    if (score1 < score2) return -1;
    return 1;
}

static int
compare_extension_names(
    const VkExtensionProperties* ext1, const VkExtensionProperties* ext2
)
{
    return strcmp(ext1->extensionName, ext2->extensionName);
}

void
vulkano_check_instance_extension_support(
    uint32_t count, const char** extensions, struct vulkano_error* error
)
{
    static const char* ENUMERATION_ERROR_MESSAGE =
        "failed to enumerate supported vulkan instance extensions";
    VkResult result;

    // enumerate supported instance extensions
    uint32_t supported_count;
    if ((result = vkEnumerateInstanceExtensionProperties(NULL, &supported_count, NULL)) !=
        VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }
    VkExtensionProperties supported_extensions[supported_count];
    if ((result = vkEnumerateInstanceExtensionProperties(
             NULL, &supported_count, supported_extensions
         )) != VK_SUCCESS) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = result;
        vulkano_write_error_message(error, ENUMERATION_ERROR_MESSAGE);
        return;
    }

    // sort for bsearch
    qsort(
        supported_extensions,
        supported_count,
        sizeof(VkExtensionProperties),
        (compare_function)compare_extension_names
    );

    // check if each requested extension is supported
    unsigned unsupported = 0;
    for (uint32_t i = 0; i < count; i++) {
        VkExtensionProperties required_extension = {0};
        snprintf(
            required_extension.extensionName,
            sizeof(required_extension.extensionName),
            "%s",
            extensions[i]
        );
        bool extension_is_supported = (bool)bsearch(
            &required_extension,
            supported_extensions,
            supported_count,
            sizeof(VkLayerProperties),
            (compare_function)compare_extension_names
        );
        if (!extension_is_supported) {
            unsupported++;
            LOGF("unsupported instance extension: %s\n", extensions[i]);
        }
    }

    // error if any extensions are not supported
    if (unsupported) {
        error->code = VULKANO_ERROR_CODE_UNSUPPORTED_INSTANCE_EXTENSION;
        error->result = VK_ERROR_EXTENSION_NOT_PRESENT;
        vulkano_write_error_message(error, "unsupported instance extensions");
        return;
    }
}

bool
vulkano_gpu_supports_extensions(
    VkPhysicalDevice gpu, uint32_t count, const char** extensions
)
{
    VkResult result;

    // enumerate gpu supported extensions
    uint32_t supported_extensions_count;
    result = vkEnumerateDeviceExtensionProperties(
        gpu, NULL, &supported_extensions_count, NULL
    );
    if (result != VK_SUCCESS) {
        LOG("unable to enumerate gpu supported extensions\n");
        return false;
    }
    VkExtensionProperties supported_extensions[supported_extensions_count];
    result = vkEnumerateDeviceExtensionProperties(
        gpu, NULL, &supported_extensions_count, supported_extensions
    );
    if (result != VK_SUCCESS) {
        LOG("unable to enumerate gpu supported extensions\n");
        return false;
    }

    // sort for searching
    qsort(
        supported_extensions,
        supported_extensions_count,
        sizeof(VkExtensionProperties),
        (compare_function)compare_extension_names
    );

    // check that all required extensions are available
    bool supported = true;
    for (size_t i = 0; i < count; i++) {
        VkExtensionProperties required_extension = {0};
        snprintf(
            required_extension.extensionName,
            sizeof(required_extension.extensionName),
            "%s",
            extensions[i]
        );
        if (!bsearch(
                &required_extension,
                supported_extensions,
                supported_extensions_count,
                sizeof(VkExtensionProperties),
                (compare_function)compare_extension_names
            )) {
            supported = false;
            LOGF("unsupported gpu extension: %s\n", extensions[i]);
        }
    }

    return supported;
}

static uint32_t
score_present_mode(VkPresentModeKHR mode)
{
    switch (mode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return 1;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return 4;
        case VK_PRESENT_MODE_FIFO_KHR:
            return 3;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return 2;
        default:
            return 0;
    }
}

int
default_present_modes_compare(VkPresentModeKHR* mode1, VkPresentModeKHR* mode2)
{
    uint32_t score1 = score_present_mode(*mode1);
    uint32_t score2 = score_present_mode(*mode2);
    if (score1 > score2) return 1;
    if (score1 == score2) return 0;
    return -1;
}

static uint32_t
score_surface_format(VkSurfaceFormatKHR format)
{
    uint32_t STANDARD_COLOR_SPACE_BIT = (uint32_t)1 << 30;
    uint32_t STANDARD_COLOR_FORMAT_BIT = (uint32_t)1 << 29;

    uint32_t score = 0;
    if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        score &= STANDARD_COLOR_SPACE_BIT;
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB) score &= STANDARD_COLOR_FORMAT_BIT;

    return score;
}

int
default_surface_format_compare(VkSurfaceFormatKHR* fmt1, VkSurfaceFormatKHR* fmt2)
{
    uint32_t score1 = score_surface_format(*fmt1);
    uint32_t score2 = score_surface_format(*fmt2);
    if (score1 > score2) return 1;
    if (score1 == score2) return 0;
    return -1;
}

const char*
vkresult_to_string(VkResult result)
{
    switch (result) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED:
            return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR:
            return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR:
            return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR:
            return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR:
            return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR:
            return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
            return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
            return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
#endif
        case VK_RESULT_MAX_ENUM:
            return "VK_RESULT_MAX_ENUM";
    }

    return "Unrecognized VkResult";
}

const char*
present_mode_to_string(VkPresentModeKHR mode)
{
    switch (mode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return "VK_PRESENT_MODE_IMMEDIATE_KHR";
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return "VK_PRESENT_MODE_MAILBOX_KHR";
        case VK_PRESENT_MODE_FIFO_KHR:
            return "VK_PRESENT_MODE_FIFO_KHR";
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
            return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
        case VK_PRESENT_MODE_MAX_ENUM_KHR:
            return "VK_PRESENT_MODE_MAX_ENUM_KHR";
    }
    return "Unrecognized present mode";
}

const char*
color_format_to_string(VkFormat fmt)
{
    switch (fmt) {
        case VK_FORMAT_UNDEFINED:
            return "VK_FORMAT_UNDEFINED";
        case VK_FORMAT_R4G4_UNORM_PACK8:
            return "VK_FORMAT_R4G4_UNORM_PACK8";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return "VK_FORMAT_R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return "VK_FORMAT_B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
        case VK_FORMAT_R8_UNORM:
            return "VK_FORMAT_R8_UNORM";
        case VK_FORMAT_R8_SNORM:
            return "VK_FORMAT_R8_SNORM";
        case VK_FORMAT_R8_USCALED:
            return "VK_FORMAT_R8_USCALED";
        case VK_FORMAT_R8_SSCALED:
            return "VK_FORMAT_R8_SSCALED";
        case VK_FORMAT_R8_UINT:
            return "VK_FORMAT_R8_UINT";
        case VK_FORMAT_R8_SINT:
            return "VK_FORMAT_R8_SINT";
        case VK_FORMAT_R8_SRGB:
            return "VK_FORMAT_R8_SRGB";
        case VK_FORMAT_R8G8_UNORM:
            return "VK_FORMAT_R8G8_UNORM";
        case VK_FORMAT_R8G8_SNORM:
            return "VK_FORMAT_R8G8_SNORM";
        case VK_FORMAT_R8G8_USCALED:
            return "VK_FORMAT_R8G8_USCALED";
        case VK_FORMAT_R8G8_SSCALED:
            return "VK_FORMAT_R8G8_SSCALED";
        case VK_FORMAT_R8G8_UINT:
            return "VK_FORMAT_R8G8_UINT";
        case VK_FORMAT_R8G8_SINT:
            return "VK_FORMAT_R8G8_SINT";
        case VK_FORMAT_R8G8_SRGB:
            return "VK_FORMAT_R8G8_SRGB";
        case VK_FORMAT_R8G8B8_UNORM:
            return "VK_FORMAT_R8G8B8_UNORM";
        case VK_FORMAT_R8G8B8_SNORM:
            return "VK_FORMAT_R8G8B8_SNORM";
        case VK_FORMAT_R8G8B8_USCALED:
            return "VK_FORMAT_R8G8B8_USCALED";
        case VK_FORMAT_R8G8B8_SSCALED:
            return "VK_FORMAT_R8G8B8_SSCALED";
        case VK_FORMAT_R8G8B8_UINT:
            return "VK_FORMAT_R8G8B8_UINT";
        case VK_FORMAT_R8G8B8_SINT:
            return "VK_FORMAT_R8G8B8_SINT";
        case VK_FORMAT_R8G8B8_SRGB:
            return "VK_FORMAT_R8G8B8_SRGB";
        case VK_FORMAT_B8G8R8_UNORM:
            return "VK_FORMAT_B8G8R8_UNORM";
        case VK_FORMAT_B8G8R8_SNORM:
            return "VK_FORMAT_B8G8R8_SNORM";
        case VK_FORMAT_B8G8R8_USCALED:
            return "VK_FORMAT_B8G8R8_USCALED";
        case VK_FORMAT_B8G8R8_SSCALED:
            return "VK_FORMAT_B8G8R8_SSCALED";
        case VK_FORMAT_B8G8R8_UINT:
            return "VK_FORMAT_B8G8R8_UINT";
        case VK_FORMAT_B8G8R8_SINT:
            return "VK_FORMAT_B8G8R8_SINT";
        case VK_FORMAT_B8G8R8_SRGB:
            return "VK_FORMAT_B8G8R8_SRGB";
        case VK_FORMAT_R8G8B8A8_UNORM:
            return "VK_FORMAT_R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM:
            return "VK_FORMAT_R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED:
            return "VK_FORMAT_R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return "VK_FORMAT_R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT:
            return "VK_FORMAT_R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT:
            return "VK_FORMAT_R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB:
            return "VK_FORMAT_R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM:
            return "VK_FORMAT_B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM:
            return "VK_FORMAT_B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED:
            return "VK_FORMAT_B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED:
            return "VK_FORMAT_B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT:
            return "VK_FORMAT_B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT:
            return "VK_FORMAT_B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB:
            return "VK_FORMAT_B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16_UNORM:
            return "VK_FORMAT_R16_UNORM";
        case VK_FORMAT_R16_SNORM:
            return "VK_FORMAT_R16_SNORM";
        case VK_FORMAT_R16_USCALED:
            return "VK_FORMAT_R16_USCALED";
        case VK_FORMAT_R16_SSCALED:
            return "VK_FORMAT_R16_SSCALED";
        case VK_FORMAT_R16_UINT:
            return "VK_FORMAT_R16_UINT";
        case VK_FORMAT_R16_SINT:
            return "VK_FORMAT_R16_SINT";
        case VK_FORMAT_R16_SFLOAT:
            return "VK_FORMAT_R16_SFLOAT";
        case VK_FORMAT_R16G16_UNORM:
            return "VK_FORMAT_R16G16_UNORM";
        case VK_FORMAT_R16G16_SNORM:
            return "VK_FORMAT_R16G16_SNORM";
        case VK_FORMAT_R16G16_USCALED:
            return "VK_FORMAT_R16G16_USCALED";
        case VK_FORMAT_R16G16_SSCALED:
            return "VK_FORMAT_R16G16_SSCALED";
        case VK_FORMAT_R16G16_UINT:
            return "VK_FORMAT_R16G16_UINT";
        case VK_FORMAT_R16G16_SINT:
            return "VK_FORMAT_R16G16_SINT";
        case VK_FORMAT_R16G16_SFLOAT:
            return "VK_FORMAT_R16G16_SFLOAT";
        case VK_FORMAT_R16G16B16_UNORM:
            return "VK_FORMAT_R16G16B16_UNORM";
        case VK_FORMAT_R16G16B16_SNORM:
            return "VK_FORMAT_R16G16B16_SNORM";
        case VK_FORMAT_R16G16B16_USCALED:
            return "VK_FORMAT_R16G16B16_USCALED";
        case VK_FORMAT_R16G16B16_SSCALED:
            return "VK_FORMAT_R16G16B16_SSCALED";
        case VK_FORMAT_R16G16B16_UINT:
            return "VK_FORMAT_R16G16B16_UINT";
        case VK_FORMAT_R16G16B16_SINT:
            return "VK_FORMAT_R16G16B16_SINT";
        case VK_FORMAT_R16G16B16_SFLOAT:
            return "VK_FORMAT_R16G16B16_SFLOAT";
        case VK_FORMAT_R16G16B16A16_UNORM:
            return "VK_FORMAT_R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM:
            return "VK_FORMAT_R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED:
            return "VK_FORMAT_R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return "VK_FORMAT_R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT:
            return "VK_FORMAT_R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT:
            return "VK_FORMAT_R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return "VK_FORMAT_R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32_UINT:
            return "VK_FORMAT_R32_UINT";
        case VK_FORMAT_R32_SINT:
            return "VK_FORMAT_R32_SINT";
        case VK_FORMAT_R32_SFLOAT:
            return "VK_FORMAT_R32_SFLOAT";
        case VK_FORMAT_R32G32_UINT:
            return "VK_FORMAT_R32G32_UINT";
        case VK_FORMAT_R32G32_SINT:
            return "VK_FORMAT_R32G32_SINT";
        case VK_FORMAT_R32G32_SFLOAT:
            return "VK_FORMAT_R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_UINT:
            return "VK_FORMAT_R32G32B32_UINT";
        case VK_FORMAT_R32G32B32_SINT:
            return "VK_FORMAT_R32G32B32_SINT";
        case VK_FORMAT_R32G32B32_SFLOAT:
            return "VK_FORMAT_R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT:
            return "VK_FORMAT_R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT:
            return "VK_FORMAT_R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return "VK_FORMAT_R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64_UINT:
            return "VK_FORMAT_R64_UINT";
        case VK_FORMAT_R64_SINT:
            return "VK_FORMAT_R64_SINT";
        case VK_FORMAT_R64_SFLOAT:
            return "VK_FORMAT_R64_SFLOAT";
        case VK_FORMAT_R64G64_UINT:
            return "VK_FORMAT_R64G64_UINT";
        case VK_FORMAT_R64G64_SINT:
            return "VK_FORMAT_R64G64_SINT";
        case VK_FORMAT_R64G64_SFLOAT:
            return "VK_FORMAT_R64G64_SFLOAT";
        case VK_FORMAT_R64G64B64_UINT:
            return "VK_FORMAT_R64G64B64_UINT";
        case VK_FORMAT_R64G64B64_SINT:
            return "VK_FORMAT_R64G64B64_SINT";
        case VK_FORMAT_R64G64B64_SFLOAT:
            return "VK_FORMAT_R64G64B64_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT:
            return "VK_FORMAT_R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT:
            return "VK_FORMAT_R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return "VK_FORMAT_R64G64B64A64_SFLOAT";
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
        case VK_FORMAT_D16_UNORM:
            return "VK_FORMAT_D16_UNORM";
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return "VK_FORMAT_X8_D24_UNORM_PACK32";
        case VK_FORMAT_D32_SFLOAT:
            return "VK_FORMAT_D32_SFLOAT";
        case VK_FORMAT_S8_UINT:
            return "VK_FORMAT_S8_UINT";
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return "VK_FORMAT_D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return "VK_FORMAT_D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return "VK_FORMAT_D32_SFLOAT_S8_UINT";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
        case VK_FORMAT_BC2_UNORM_BLOCK:
            return "VK_FORMAT_BC2_UNORM_BLOCK";
        case VK_FORMAT_BC2_SRGB_BLOCK:
            return "VK_FORMAT_BC2_SRGB_BLOCK";
        case VK_FORMAT_BC3_UNORM_BLOCK:
            return "VK_FORMAT_BC3_UNORM_BLOCK";
        case VK_FORMAT_BC3_SRGB_BLOCK:
            return "VK_FORMAT_BC3_SRGB_BLOCK";
        case VK_FORMAT_BC4_UNORM_BLOCK:
            return "VK_FORMAT_BC4_UNORM_BLOCK";
        case VK_FORMAT_BC4_SNORM_BLOCK:
            return "VK_FORMAT_BC4_SNORM_BLOCK";
        case VK_FORMAT_BC5_UNORM_BLOCK:
            return "VK_FORMAT_BC5_UNORM_BLOCK";
        case VK_FORMAT_BC5_SNORM_BLOCK:
            return "VK_FORMAT_BC5_SNORM_BLOCK";
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
        case VK_FORMAT_BC7_UNORM_BLOCK:
            return "VK_FORMAT_BC7_UNORM_BLOCK";
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return "VK_FORMAT_BC7_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
        case VK_FORMAT_G8B8G8R8_422_UNORM:
            return "VK_FORMAT_G8B8G8R8_422_UNORM";
        case VK_FORMAT_B8G8R8G8_422_UNORM:
            return "VK_FORMAT_B8G8R8G8_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
            return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
            return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
            return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
            return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
            return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
        case VK_FORMAT_R10X6_UNORM_PACK16:
            return "VK_FORMAT_R10X6_UNORM_PACK16";
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
            return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
            return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
            return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_R12X4_UNORM_PACK16:
            return "VK_FORMAT_R12X4_UNORM_PACK16";
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
            return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
            return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
            return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
            return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16B16G16R16_422_UNORM:
            return "VK_FORMAT_G16B16G16R16_422_UNORM";
        case VK_FORMAT_B16G16R16G16_422_UNORM:
            return "VK_FORMAT_B16G16R16G16_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
            return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
            return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
            return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
            return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
            return "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
            return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
            return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM";
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            return "VK_FORMAT_A4R4G4B4_UNORM_PACK16";
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return "VK_FORMAT_A4B4G4R4_UNORM_PACK16";
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK";
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
            return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
            return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
            return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
            return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
            return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
            return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_R16G16_S10_5_NV:
            return "VK_FORMAT_R16G16_S10_5_NV";
        case VK_FORMAT_MAX_ENUM:
            return "VK_FORMAT_MAX_ENUM";
    }
    return "Unrecognized VkFormat";
}

const char*
color_space_to_string(VkColorSpaceKHR space)
{
    switch (space) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
            return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
            return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
            return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
            return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
            return "VK_COLOR_SPACE_DOLBYVISION_EXT";
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
            return "VK_COLOR_SPACE_HDR10_HLG_EXT";
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
            return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
            return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD";
        case VK_COLOR_SPACE_MAX_ENUM_KHR:
            return "VK_COLOR_SPACE_MAX_ENUM_KHR";
    }
    return "Unrecognized VkColorSpaceKHR";
}

#ifdef VULKANO_INTEGRATE_SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

SDL_Window* vulkano_sdl_window;

const char*
create_surface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (!SDL_Vulkan_CreateSurface(vulkano_sdl_window, instance, surface)) {
        return SDL_GetError();
    }
    return NULL;
}

void
query_size(uint32_t* width, uint32_t* height)
{
    int w, h;
    SDL_Vulkan_GetDrawableSize(vulkano_sdl_window, &w, &h);
    w = (w < 0) ? 720 : w;
    h = (h < 0) ? 480 : h;
    *width = (uint32_t)w;
    *height = (uint32_t)h;
}

struct vulkano_sdl
vulkano_sdl_init(
    struct vulkano_config vkcfg, struct sdl_config sdlcfg, struct vulkano_error* error
)
{
    DEFAULT0(sdlcfg.title, "vulkano sdl window");
    DEFAULT0(sdlcfg.width, 720);
    DEFAULT0(sdlcfg.height, 480);

    vkcfg.surface_creation = create_surface;
    vkcfg.query_size = query_size;

    struct vulkano_sdl vksdl = {0};

    // initialize sdl
    if (SDL_Init(sdlcfg.init_flags | SDL_INIT_VIDEO)) {
        error->code = VULKANO_ERROR_CODE_SURFACE_CREATION_FAILED;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, SDL_GetError());
        return vksdl;
    }
    vulkano_sdl_window = SDL_CreateWindow(
        sdlcfg.title,
        sdlcfg.left,
        sdlcfg.top,
        sdlcfg.width,
        sdlcfg.height,
        sdlcfg.window_flags | SDL_WINDOW_VULKAN
    );
    if (!vulkano_sdl_window) {
        error->code = VULKANO_ERROR_CODE_SURFACE_CREATION_FAILED;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, SDL_GetError());
        SDL_Quit();
        return vksdl;
    }
    vksdl.sdl = vulkano_sdl_window;

    // enumerate sdl required instance extensions and combine them with user required
    // instance extensions
    struct string_array user_instance_extensions = {
        .count = vkcfg.instance_extensions_count,
        .data = vkcfg.instance_extensions,
    };
    struct string_array sdl_required_extensions = {0};
    struct string_array combined_instance_extensions = {0};

    if (!SDL_Vulkan_GetInstanceExtensions(
            vksdl.sdl, &sdl_required_extensions.count, NULL
        )) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, SDL_GetError());
        SDL_DestroyWindow(vksdl.sdl);
        SDL_Quit();
        return vksdl;
    }
    const char* extensions[sdl_required_extensions.count];
    sdl_required_extensions.data = extensions;
    if (!SDL_Vulkan_GetInstanceExtensions(
            vksdl.sdl, &sdl_required_extensions.count, sdl_required_extensions.data
        )) {
        error->code = VULKANO_ERROR_CODE_FAILED_ENUMERATION;
        error->result = VK_ERROR_UNKNOWN;
        vulkano_write_error_message(error, SDL_GetError());
        goto cleanup;
    }

    combined_instance_extensions = combine_string_arrays_unique(
        user_instance_extensions, sdl_required_extensions, error
    );
    if (error->code) goto cleanup;
    vkcfg.instance_extensions = combined_instance_extensions.data;
    vkcfg.instance_extensions_count = combined_instance_extensions.count;

    // initialize vulkano
    vksdl.vk = vulkano_init(vkcfg, error);
    if (error->code) goto cleanup;

cleanup:
    free(combined_instance_extensions.data);
    if (error->code) vulkano_sdl_destroy(&vksdl);
    return vksdl;
}

void
vulkano_sdl_destroy(struct vulkano_sdl* vksdl)
{
    vulkano_destroy(&vksdl->vk);
    if (vksdl->sdl) SDL_DestroyWindow(vksdl->sdl);
    SDL_Quit();
}

#endif  // VULKANO_INTEGRATE_SDL

#endif  // VULKANO_IMPLEMENTATION

#endif  // VULKANO_H
