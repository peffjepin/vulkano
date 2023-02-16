#include <errno.h>

#define VULKANO_IMPLEMENTATION
#define VULKANO_ENABLE_DEFAULT_VALIDATION_LAYERS
#define VULKANO_ENABLE_DEFAULT_GRAPHICS_EXTENSIONS
#define VULKANO_INTEGRATE_SDL

#include "../vulkano.h"

struct vec2 {
    float x;
    float y;
};

struct vec3 {
    float x;
    float y;
    float z;
};

struct vertex {
    struct vec2 position;
    struct vec3 color;
};

struct vertex_instanced {
    float       scale;
    struct vec2 offset;
};

struct vulkano_data read_file_content(const char* filepath);

int
main(void)
{
    struct vulkano_error error = {0};
    struct vulkano_sdl   vksdl = vulkano_sdl_create(
        (struct vulkano_config){0}, (struct sdl_config){.window_flags = SDL_WINDOW_RESIZABLE}, &error
    );

    if (error.code) {
        fprintf(stderr, "ERROR: vulkan failed to initialize (%s)\n", error.message);
        return error.code;
    }

    VkRenderPass render_pass = vulkano_create_render_pass(
        &vksdl.vk,
        (struct VkRenderPassCreateInfo){
            .attachmentCount = 2,
            .pAttachments =
                (struct VkAttachmentDescription[]){
                    {
                        .format = 0,  // match swapchain
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    },
                    {
                        .format = VULKANO_DEPTH_FORMAT,
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    },
                },
            .subpassCount = 1,
            .pSubpasses =
                (struct VkSubpassDescription[]){
                    {
                        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .colorAttachmentCount = 1,
                        .pColorAttachments =
                            (struct VkAttachmentReference[]){
                                {
                                    .attachment = 0,
                                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                },
                            },
                        .pDepthStencilAttachment =
                            (struct VkAttachmentReference[]){
                                {
                                    .attachment = 1,
                                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                },
                            },
                    },
                },
        },
        &error
    );

    vulkano_configure_swapchain(&vksdl.vk, render_pass, 3, &error);

    struct vulkano_data vertex_shader_data = read_file_content("shader.vert.spv");
    struct vulkano_data fragment_shader_data = read_file_content("shader.frag.spv");

    VkShaderModule   vertex_shader_module = vulkano_create_shader_module(&vksdl.vk, vertex_shader_data, &error);
    VkShaderModule   fragment_shader_module = vulkano_create_shader_module(&vksdl.vk, fragment_shader_data, &error);
    VkPipelineLayout pipeline_layout =
        vulkano_create_pipeline_layout(&vksdl.vk, (struct VkPipelineLayoutCreateInfo){0}, &error);
    VkPipeline pipeline = vulkano_create_graphics_pipeline(
        &vksdl.vk,
        (struct graphics_pipeline_create_info){
            .stage_count = 2,
            .stages =
                {
                    {
                        .stage = VK_SHADER_STAGE_VERTEX_BIT,
                        .module = vertex_shader_module,
                        .pName = "main",
                    },
                    {
                        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .module = fragment_shader_module,
                        .pName = "main",
                    },
                },
            .vertex_input_state =
                {
                    .vertexBindingDescriptionCount = 2,
                    .pVertexBindingDescriptions =
                        (struct VkVertexInputBindingDescription[]){
                            {
                                .binding = 0,
                                .stride = sizeof(struct vertex),
                                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                            },
                            {
                                .binding = 1,
                                .stride = sizeof(struct vertex_instanced),
                                .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
                            },
                        },
                    .vertexAttributeDescriptionCount = 4,
                    .pVertexAttributeDescriptions =
                        (struct VkVertexInputAttributeDescription[]){
                            {
                                .binding = 0,
                                .location = 0,
                                .format = VK_FORMAT_R32G32_SFLOAT,
                                .offset = offsetof(struct vertex, position),
                            },
                            {
                                .binding = 0,
                                .location = 1,
                                .format = VK_FORMAT_R32G32B32_SFLOAT,
                                .offset = offsetof(struct vertex, color),
                            },
                            {
                                .binding = 1,
                                .location = 2,
                                .format = VK_FORMAT_R32_SFLOAT,
                                .offset = offsetof(struct vertex_instanced, scale),
                            },
                            {
                                .binding = 1,
                                .location = 3,
                                .format = VK_FORMAT_R32G32_SFLOAT,
                                .offset = offsetof(struct vertex_instanced, offset),
                            },
                        },
                },
            .input_assembly_state =
                {
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                },
            .rasterization_state =
                {
                    .polygonMode = VK_POLYGON_MODE_FILL,
                    .cullMode = VK_CULL_MODE_BACK_BIT,
                    .frontFace = VK_FRONT_FACE_CLOCKWISE,
                    .lineWidth = 1,
                },
            .viewport_state =
                {
                    .viewportCount = 1,
                    .pViewports =
                        (struct VkViewport[]){
                            VULKANO_VIEWPORT(&vksdl.vk),
                        },
                    .scissorCount = 1,
                    .pScissors =
                        (struct VkRect2D[]){
                            VULKANO_SCISSOR(&vksdl.vk),
                        },
                },
            .color_blend_state =
                {
                    .attachmentCount = 1,
                    .pAttachments =
                        (struct VkPipelineColorBlendAttachmentState[]){
                            {
                                .blendEnable = VK_TRUE,
                                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                .colorBlendOp = VK_BLEND_OP_ADD,
                                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                .alphaBlendOp = VK_BLEND_OP_ADD,
                                .colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                  VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
                            },
                        },
                },
            .dynamic_state =
                {
                    .dynamicStateCount = 2,
                    .pDynamicStates =
                        (VkDynamicState[]){
                            VK_DYNAMIC_STATE_SCISSOR,
                            VK_DYNAMIC_STATE_VIEWPORT,
                        },
                },
            .render_pass = render_pass,
            .layout = pipeline_layout,
        },
        &error
    );

    struct vertex vertices[] = {
        {.position = {-1, -1}, .color = {0.0, 0.8, 0.8}},
        {.position = {1, -1}, .color = {0.8, 0.0, 0.8}},
        {.position = {1, 1}, .color = {0.8, 0.8, 0.0}},
        {.position = {-1, 1}, .color = {0.2, 0.2, 0.6}},
    };
    struct vertex_instanced instanced_attributes[] = {
        {.scale = 0.05, .offset = {-0.5, -0.5}},
        {.scale = 0.2, .offset = {0.5, 0.5}},
        {.scale = 0.33, .offset = {0.5, -0.5}},
        {.scale = 0.1, .offset = {-0.5, 0.5}},
    };
    uint16_t indices[] = {0, 1, 3, 3, 1, 2};

    struct vulkano_buffer vertex_buffer = vulkano_buffer_create(
        &vksdl.vk,
        (struct VkBufferCreateInfo){
            .size = sizeof vertices,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        },
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &error
    );
    vulkano_buffer_copy_to(
        &vksdl.vk,
        &vertex_buffer,
        (struct vulkano_data){
            .length = sizeof(vertices),
            .bytes = (uint8_t*)vertices,
        },
        &error
    );

    struct vulkano_buffer instance_attibutes_buffer = vulkano_buffer_create(
        &vksdl.vk,
        (struct VkBufferCreateInfo){
            .size = sizeof instanced_attributes,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        },
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &error
    );
    vulkano_buffer_copy_to(
        &vksdl.vk,
        &instance_attibutes_buffer,
        (struct vulkano_data){
            .length = sizeof(instanced_attributes),
            .bytes = (uint8_t*)instanced_attributes,
        },
        &error
    );

    struct vulkano_buffer index_buffer = vulkano_buffer_create(
        &vksdl.vk,
        (struct VkBufferCreateInfo){
            .size = sizeof indices,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        },
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &error
    );
    vulkano_buffer_copy_to(
        &vksdl.vk,
        &index_buffer,
        (struct vulkano_data){
            .length = sizeof(indices),
            .bytes = (uint8_t*)indices,
        },
        &error
    );

    if (error.code) goto cleanup;

    for (uint32_t i = 0; i < 1000; i++) {
        struct vulkano_frame frame = {.clear = {0.012, 0.01, 0.01, 0.0}};
        vulkano_frame_acquire(&vksdl.vk, &frame, &error);
        if (error.code) goto cleanup;

        vkCmdBindPipeline(frame.state.render_command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkViewport viewport = VULKANO_VIEWPORT(&vksdl.vk);
        vkCmdSetViewport(frame.state.render_command, 0, 1, &viewport);

        VkRect2D scissor = VULKANO_SCISSOR(&vksdl.vk);
        vkCmdSetScissor(frame.state.render_command, 0, 1, &scissor);

        vkCmdBindVertexBuffers(
            frame.state.render_command,
            0,
            2,
            (VkBuffer[]){vertex_buffer.handle, instance_attibutes_buffer.handle},
            (VkDeviceSize[]){0, 0}
        );
        vkCmdBindIndexBuffer(frame.state.render_command, index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(
            frame.state.render_command,
            sizeof(indices) / sizeof(indices[0]),
            sizeof(instanced_attributes) / sizeof(instanced_attributes[0]),
            0,
            0,
            0
        );

        vulkano_frame_submit(&vksdl.vk, &frame, (struct VkSubmitInfo){0}, &error);
        if (error.code) goto cleanup;
    }

cleanup:
    vkDeviceWaitIdle(vksdl.vk.device);

    vkDestroyRenderPass(vksdl.vk.device, render_pass, NULL);
    vkDestroyShaderModule(vksdl.vk.device, vertex_shader_module, NULL);
    vkDestroyShaderModule(vksdl.vk.device, fragment_shader_module, NULL);
    vkDestroyPipelineLayout(vksdl.vk.device, pipeline_layout, NULL);
    vkDestroyPipeline(vksdl.vk.device, pipeline, NULL);
    free(vertex_shader_data.bytes);
    free(fragment_shader_data.bytes);

    vulkano_buffer_destroy(&vksdl.vk, &vertex_buffer);
    vulkano_buffer_destroy(&vksdl.vk, &instance_attibutes_buffer);
    vulkano_buffer_destroy(&vksdl.vk, &index_buffer);

    vulkano_sdl_destroy(&vksdl);

    if (error.code) fprintf(stderr, "ERROR: %s\n", error.message);

    return error.code;
}

struct vulkano_data
read_file_content(const char* filepath)
{
    struct vulkano_data data = {0};

    FILE* fp = fopen(filepath, "r");
    int   end;

    if (!fp) {
        fprintf(stderr, "ERROR: failed to open file `%s` (%s)\n", filepath, strerror(errno));
        exit(1);
    }
    if (fseek(fp, 0, SEEK_END)) {
        fprintf(stderr, "ERROR: failed to file operation `%s` (%s)\n", filepath, strerror(errno));
        fclose(fp);
        exit(1);
    }
    if ((end = ftell(fp)) < 0) {
        fprintf(stderr, "ERROR: failed to file operation `%s` (%s)\n", filepath, strerror(errno));
        fclose(fp);
        exit(1);
    }

    data.length = end;
    if (fseek(fp, 0, SEEK_SET)) {
        fprintf(stderr, "ERROR: failed to file operation `%s` (%s)\n", filepath, strerror(errno));
        fclose(fp);
        exit(1);
    }

    if (!(data.bytes = (uint8_t*)malloc(data.length))) {
        fprintf(stderr, "ERROR: out of memory\n");
        fclose(fp);
        exit(1);
    }
    if (fread(data.bytes, 1, data.length, fp) < data.length) {
        fprintf(stderr, "ERROR: failed to read file contents `%s` (%s)\n", filepath, strerror(errno));
        fclose(fp);
        exit(1);
    };

    fclose(fp);
    return data;
}
