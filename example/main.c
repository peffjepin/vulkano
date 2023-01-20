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
    float scale;
    struct vec2 offset;
};

struct vulkano_data read_file_content(const char* filepath);

int
main()
{
    struct vulkano_data vertex_shader = read_file_content("shader.vert.spv");
    struct vulkano_data fragment_shader = read_file_content("shader.frag.spv");

    struct vulkano_buffer vertex_buffer = {0};
    struct vulkano_buffer instance_attibutes_buffer = {0};
    struct vulkano_buffer index_buffer = {0};

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

    struct vulkano_error error = {0};

    struct vulkano_sdl vksdl = vulkano_sdl_init(
        (struct vulkano_config){
            .render_pass_count = 1,
            .render_passes =
                (struct VkRenderPassCreateInfo[]){
                    {
                        .attachmentCount = 1,
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
                                                .layout =
                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            },
                                        },
                                },
                            },
                    },
                },
            .pipeline_layout_count = 1,
            .pipeline_layout_configs =
                (struct vulkano_pipeline_layout_config[]){
                    {0},
                },
            .swapchain_image_count = 2,
            .swapchain_render_pass_index = 0,
            .pipeline_count = 1,
            .pipeline_configs =
                (struct vulkano_graphics_pipeline_config[]){
                    {
                        .vertex_shader = vertex_shader,
                        .fragment_shader = fragment_shader,
                        .vertex_input.vertexBindingDescriptionCount = 2,
                        .vertex_input.pVertexBindingDescriptions =
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
                        .vertex_input.vertexAttributeDescriptionCount = 4,
                        .vertex_input.pVertexAttributeDescriptions =
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
                        .input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                        .viewport.maxDepth = 1,
                        .rasterization.polygonMode = VK_POLYGON_MODE_FILL,
                        .rasterization.cullMode = VK_CULL_MODE_BACK_BIT,
                        .rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE,
                        .rasterization.lineWidth = 1,
                        .blend.attachmentCount = 1,
                        .blend.pAttachments =
                            (struct VkPipelineColorBlendAttachmentState[]){
                                {
                                    .blendEnable = VK_TRUE,
                                    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                                    .dstColorBlendFactor =
                                        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                                    .colorBlendOp = VK_BLEND_OP_ADD,
                                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .alphaBlendOp = VK_BLEND_OP_ADD,
                                    .colorWriteMask = VK_COLOR_COMPONENT_A_BIT |
                                                      VK_COLOR_COMPONENT_B_BIT |
                                                      VK_COLOR_COMPONENT_G_BIT |
                                                      VK_COLOR_COMPONENT_R_BIT,
                                },
                            },
                        .dynamic.dynamicStateCount = 2,
                        .dynamic.pDynamicStates =
                            (VkDynamicState[]){
                                VK_DYNAMIC_STATE_SCISSOR,
                                VK_DYNAMIC_STATE_VIEWPORT,
                            },
                        .render_pass_index = 0,
                        .pipeline_layout_index = 0,
                    },
                },
        },
        (struct sdl_config){0},
        &error
    );

    struct vulkano* vk = &vksdl.vk;

    free(vertex_shader.bytes);
    free(fragment_shader.bytes);

    if (error.code) goto cleanup;

    vertex_buffer = vulkano_create_buffer(
        vk,
        sizeof(vertices),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &error
    );
    vulkano_copy_to_buffer(
        vk,
        &vertex_buffer,
        (struct vulkano_data){
            .length = sizeof(vertices),
            .bytes = (uint8_t*)vertices,
        },
        &error
    );
    if (error.code) goto cleanup;

    instance_attibutes_buffer = vulkano_create_buffer(
        vk,
        sizeof(instanced_attributes),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &error
    );
    vulkano_copy_to_buffer(
        vk,
        &instance_attibutes_buffer,
        (struct vulkano_data){
            .length = sizeof(instanced_attributes),
            .bytes = (uint8_t*)instanced_attributes,
        },
        &error
    );
    if (error.code) goto cleanup;

    index_buffer = vulkano_create_buffer(
        vk,
        sizeof(indices),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &error
    );
    vulkano_copy_to_buffer(
        vk,
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
        vulkano_begin_frame(vk, &frame, 0, &error);
        if (error.code) goto cleanup;

        vkCmdBindPipeline(
            frame.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            VULKANO_GRAPHICS_PIPELINE(vk, 0)
        );

        VkViewport viewport = VULKANO_VIEWPORT(vk);
        vkCmdSetViewport(frame.command_buffer, 0, 1, &viewport);

        VkRect2D scissor = VULKANO_SCISSOR(vk);
        vkCmdSetScissor(frame.command_buffer, 0, 1, &scissor);

        vkCmdBindVertexBuffers(
            frame.command_buffer,
            0,
            2,
            (VkBuffer[]){vertex_buffer.handle, instance_attibutes_buffer.handle},
            (VkDeviceSize[]){0, 0}
        );
        vkCmdBindIndexBuffer(
            frame.command_buffer, index_buffer.handle, 0, VK_INDEX_TYPE_UINT16
        );

        vkCmdDrawIndexed(
            frame.command_buffer,
            sizeof(indices) / sizeof(indices[0]),
            sizeof(instanced_attributes) / sizeof(instanced_attributes[0]),
            0,
            0,
            0
        );

        vulkano_submit_frame(vk, &frame, &error);
        if (error.code) goto cleanup;
    }

cleanup:
    vkDeviceWaitIdle(vk->device);
    vulkano_destroy_buffer(vk, &vertex_buffer);
    vulkano_destroy_buffer(vk, &instance_attibutes_buffer);
    vulkano_destroy_buffer(vk, &index_buffer);
    vulkano_sdl_destroy(&vksdl);
    if (error.code) fprintf(stderr, "ERROR: %s\n", error.message);
    return error.code;
}

struct vulkano_data
read_file_content(const char* filepath)
{
    struct vulkano_data data = {0};

    FILE* fp = fopen(filepath, "r");
    int end;

    if (!fp) {
        fprintf(
            stderr, "ERROR: failed to open file `%s` (%s)\n", filepath, strerror(errno)
        );
        exit(1);
    }
    if (fseek(fp, 0, SEEK_END)) {
        fprintf(
            stderr,
            "ERROR: failed to file operation `%s` (%s)\n",
            filepath,
            strerror(errno)
        );
        fclose(fp);
        exit(1);
    }
    if ((end = ftell(fp)) < 0) {
        fprintf(
            stderr,
            "ERROR: failed to file operation `%s` (%s)\n",
            filepath,
            strerror(errno)
        );
        fclose(fp);
        exit(1);
    }

    data.length = end;
    if (fseek(fp, 0, SEEK_SET)) {
        fprintf(
            stderr,
            "ERROR: failed to file operation `%s` (%s)\n",
            filepath,
            strerror(errno)
        );
        fclose(fp);
        exit(1);
    }

    if (!(data.bytes = (uint8_t*)malloc(data.length))) {
        fprintf(stderr, "ERROR: out of memory\n");
        fclose(fp);
        exit(1);
    }
    if (fread(data.bytes, 1, data.length, fp) < data.length) {
        fprintf(
            stderr,
            "ERROR: failed to read file contents `%s` (%s)\n",
            filepath,
            strerror(errno)
        );
        fclose(fp);
        exit(1);
    };

    fclose(fp);
    return data;
}
