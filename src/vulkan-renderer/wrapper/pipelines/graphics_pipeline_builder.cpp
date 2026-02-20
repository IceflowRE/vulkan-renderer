#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Using declaration
using wrapper::InexorException;

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device &device, const PipelineCache &pipeline_cache)
    : m_device(device), m_pipeline_cache(pipeline_cache) {}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept
    : m_device(other.m_device), m_pipeline_cache(other.m_pipeline_cache) {
    // @TODO: Implement move constructor for GraphicsPipelineSetupData and use std::move here
    m_data = other.m_data;
}

// @TODO Remove bool parameter once we switch to dynamic rendering only
std::shared_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build(std::string name, bool use_dynamic_rendering) {
    if (name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    // NOTE: Inside of GraphicsPipelineBuilder, we do not carry out error checks when it comes to the data which is used
    // to build the graphics pipeline. This is because validation of this data is job of the validation layers, and not
    // the job of GraphicsPipelineBuilder. We should not mimic the behavious of validation layers here.

    auto graphics_pipeline =
        std::make_shared<GraphicsPipeline>(m_device, m_pipeline_cache, m_data, use_dynamic_rendering, std::move(name));

    // @TODO: Does this work as intended?
    // NOTE: We reset the data of the builder here so it can be re-used
    m_data = {};

    // Return the graphics pipeline we created
    return graphics_pipeline;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_color_attachment_format(const VkFormat format) {
    // @TODO How does this relate to add_color_blend_attachment?
    m_data.color_attachments.push_back(format);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
    m_data.color_blend_attachment_states.push_back(attachment);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_default_color_blend_attachment() {
    // @TODO Is this default generalized enough?
    return add_color_blend_attachment({
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_push_constant_range(const VkShaderStageFlags shader_stage,
                                                                          const std::uint32_t size,
                                                                          const std::uint32_t offset) {
    m_data.push_constant_ranges.emplace_back(VkPushConstantRange{
        .stageFlags = shader_stage,
        .offset = offset,
        .size = size,
    });
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_shader(std::weak_ptr<Shader> shader) {
    m_data.shader_stages.emplace_back(wrapper::make_info<VkPipelineShaderStageCreateInfo>({
        .stage = shader.lock()->shader_stage(),
        .module = shader.lock()->shader_module(),
        .pName = shader.lock()->entry_point().c_str(),
    }));
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend) {
    m_data.color_blend_sci = color_blend;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_color_blend_attachments(
    const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
    m_data.color_blend_attachment_states = attachments;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_culling_mode(const VkBool32 culling_enabled) {
    if (culling_enabled == VK_FALSE) {
        spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
    }
    m_data.rasterization_sci.cullMode = culling_enabled == VK_TRUE ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_depth_attachment_format(const VkFormat format) {
    m_data.depth_attachment_format = format;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
    m_data.depth_stencil_sci = depth_stencil;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_stencil_attachment_format(const VkFormat format) {
    m_data.stencil_attachment_format = format;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_descriptor_set_layout(const VkDescriptorSetLayout descriptor_set_layout) {
    assert(descriptor_set_layout);
    m_data.descriptor_set_layouts = {descriptor_set_layout};
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_descriptor_set_layouts(std::vector<VkDescriptorSetLayout> descriptor_set_layouts) {
    assert(!descriptor_set_layouts.empty());
    m_data.descriptor_set_layouts = std::move(descriptor_set_layouts);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states) {
    assert(!dynamic_states.empty());
    m_data.dynamic_states = dynamic_states;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly) {
    m_data.input_assembly_sci = input_assembly;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_line_width(const float width) {
    m_data.rasterization_sci.lineWidth = width;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_multisampling(const VkSampleCountFlagBits sample_count,
                                                                    const std::optional<float> min_sample_shading) {
    m_data.multisample_sci.rasterizationSamples = sample_count;
    if (min_sample_shading) {
        m_data.multisample_sci.minSampleShading = min_sample_shading.value();
    }
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_pipeline_layout(const VkPipelineLayout layout) {
    assert(layout);
    m_data.pipeline_layout = layout;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_push_constant_ranges(std::vector<VkPushConstantRange> push_constant_ranges) {
    m_data.push_constant_ranges = std::move(push_constant_ranges);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_primitive_topology(const VkPrimitiveTopology topology) {
    m_data.input_assembly_sci.topology = topology;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
    m_data.rasterization_sci = rasterization;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_render_pass(const VkRenderPass &render_pass) {
    m_data.render_pass = render_pass;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor(const VkRect2D &scissor) {
    m_data.scissors = {scissor};
    m_data.viewport_sci.scissorCount = 1;
    m_data.viewport_sci.pScissors = m_data.scissors.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor(const VkExtent2D &extent) {
    return set_scissor({
        // Convert VkExtent2D to VkRect2D
        .extent = extent,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_shaders(std::vector<VkPipelineShaderStageCreateInfo> shaders) {
    assert(!shaders.empty());
    m_data.shader_stages = std::move(shaders);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_tesselation_control_point_count(const std::uint32_t control_point_count) {
    m_data.tesselation_sci.patchControlPoints = control_point_count;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_vertex_input_attributes(
    const std::vector<VkVertexInputAttributeDescription> &descriptions) {
    assert(!descriptions.empty());
    m_data.vertex_input_attribute_descriptions = descriptions;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &descriptions) {
    assert(!descriptions.empty());
    m_data.vertex_input_binding_descriptions = descriptions;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewport(const VkViewport &viewport) {
    m_data.viewports = {viewport};
    m_data.viewport_sci.viewportCount = 1;
    m_data.viewport_sci.pViewports = m_data.viewports.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewport(const VkExtent2D &extent) {
    return set_viewport({
        // Convert VkExtent2D to VkViewport
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .maxDepth = 1.0f,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_wireframe(const VkBool32 wireframe) {
    m_data.rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
