#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipeline::GraphicsPipeline(const Device &device, const PipelineCache &pipeline_cache,
                                   GraphicsPipelineSetupData pipeline_setup_data, bool use_dynamic_rendering,
                                   std::string name)
    : m_device(device), m_name(std::move(name)) {

    // NOTE: It's important to fill VkGraphicsPipelineCreateInfo in the constructor of GraphicsPipeline!

    if (use_dynamic_rendering) {
        pipeline_setup_data.pipeline_rendering_ci = make_info<VkPipelineRenderingCreateInfo>({
            // TODO: Support multiview rendering and expose viewMask parameter
            .colorAttachmentCount = static_cast<std::uint32_t>(pipeline_setup_data.color_attachments.size()),
            .pColorAttachmentFormats = pipeline_setup_data.color_attachments.data(),
            .depthAttachmentFormat = pipeline_setup_data.depth_attachment_format,
            .stencilAttachmentFormat = pipeline_setup_data.stencil_attachment_format,
        });
    }

    pipeline_setup_data.vertex_input_sci = make_info<VkPipelineVertexInputStateCreateInfo>({
        .vertexBindingDescriptionCount =
            static_cast<std::uint32_t>(pipeline_setup_data.vertex_input_binding_descriptions.size()),
        .pVertexBindingDescriptions = pipeline_setup_data.vertex_input_binding_descriptions.data(),
        .vertexAttributeDescriptionCount =
            static_cast<std::uint32_t>(pipeline_setup_data.vertex_input_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = pipeline_setup_data.vertex_input_attribute_descriptions.data(),

    });

    pipeline_setup_data.viewport_sci = make_info<VkPipelineViewportStateCreateInfo>({
        .viewportCount = static_cast<uint32_t>(pipeline_setup_data.viewports.size()),
        .pViewports = pipeline_setup_data.viewports.data(),
        .scissorCount = static_cast<uint32_t>(pipeline_setup_data.scissors.size()),
        .pScissors = pipeline_setup_data.scissors.data(),
    });

    pipeline_setup_data.color_blend_sci = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>({
        .attachmentCount = static_cast<std::uint32_t>(pipeline_setup_data.color_blend_attachment_states.size()),
        .pAttachments = pipeline_setup_data.color_blend_attachment_states.data(),
    });

    pipeline_setup_data.dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>({
        .dynamicStateCount = static_cast<std::uint32_t>(pipeline_setup_data.dynamic_states.size()),
        .pDynamicStates = pipeline_setup_data.dynamic_states.data(),
    });

    // @TODO Expose the pipeline layout as parameter
    m_pipeline_layout = std::make_unique<PipelineLayout>(m_device, m_name, pipeline_setup_data.descriptor_set_layouts,
                                                         pipeline_setup_data.push_constant_ranges);

    // @TODO Remove 'use_dynamic_rendering' parameter once we move away from renderpasses!
    auto pipeline_ci = make_info<VkGraphicsPipelineCreateInfo>({
        // NOTE: This is one of those rare cases where pNext is actually not nullptr!
        .pNext = (use_dynamic_rendering) ? &pipeline_setup_data.pipeline_rendering_ci : nullptr,
        .stageCount = static_cast<std::uint32_t>(pipeline_setup_data.shader_stages.size()),
        .pStages = pipeline_setup_data.shader_stages.data(),
        .pVertexInputState = &pipeline_setup_data.vertex_input_sci,
        .pInputAssemblyState = &pipeline_setup_data.input_assembly_sci,
        .pTessellationState = &pipeline_setup_data.tesselation_sci,
        .pViewportState = &pipeline_setup_data.viewport_sci,
        .pRasterizationState = &pipeline_setup_data.rasterization_sci,
        .pMultisampleState = &pipeline_setup_data.multisample_sci,
        .pDepthStencilState = &pipeline_setup_data.depth_stencil_sci,
        .pColorBlendState = &pipeline_setup_data.color_blend_sci,
        .pDynamicState = &pipeline_setup_data.dynamic_states_sci,
        .layout = m_pipeline_layout->pipeline_layout(),
        // @TODO Make this VK_NULL_HANDLE and use dynamic rendering!
        .renderPass = (use_dynamic_rendering) ? VK_NULL_HANDLE : pipeline_setup_data.render_pass,
    });

    if (const auto result = vkCreateGraphicsPipelines(m_device.device(), pipeline_cache.m_pipeline_cache, 1,
                                                      &pipeline_ci, nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed!", result, m_name);
    }
    m_device.set_debug_name(m_pipeline, m_name);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept : m_device(other.m_device) {
    // TODO: Implement me!
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

VkPipelineLayout GraphicsPipeline::pipeline_layout() const {
    return m_pipeline_layout->pipeline_layout();
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
