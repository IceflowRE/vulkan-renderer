#pragma once

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <volk.h>

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::pipelines {
class PipelineCache;
class PipelineLayout;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

// Using declaration
using inexor::vulkan_renderer::wrapper::pipelines::PipelineLayout;

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement RAII wrapper for ComputePipeline

/// When creating a graphics pipeline, the lifetime of certain data which is used to create the pipeline must be
/// ensured. In particular, the VkGraphicsPipelineCreateInfo struct must not be stored, however, the memory to which the
/// pointers inside of VkGraphicsPipelineCreateInfo point to must be stored. For example, VkGraphicsPipelineCreateInfo
/// has a member VkPipelineViewportStateCreateInfo, which itself has a pointer that point to VkViewport data, for
/// example. This means we must make sure the lifetime of all data that the pointers point to must be preserved.
/// Initially, we collected all the data to create the graphics pipeline in GraphicsPipelineBuilder, and reset all the
/// data of the builder after the build() method has been called. However, this is wrong, because the lifetime of the
/// data ends with calling reset(). This causes some bugs which are hard to find.
///
/// @TODO: Implement move constructor for GraphicsPipelineSetupData
struct GraphicsPipelineSetupData {
    // This is the underlying data for the create info structures
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
    std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions{};
    std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions{};
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states{};
    std::vector<VkViewport> viewports{};
    std::vector<VkRect2D> scissors{};
    std::vector<VkPushConstantRange> push_constant_ranges{};
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkRenderPass render_pass{VK_NULL_HANDLE};
    VkFormat depth_attachment_format{};
    VkFormat stencil_attachment_format{};
    std::vector<VkFormat> color_attachments{};
    std::vector<VkDynamicState> dynamic_states{};
    VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};

    // @TODO: Can we make this more pretty?

    // These are the create info structures required to fill the VkGraphicsPipelineCreateInfo
    VkPipelineVertexInputStateCreateInfo vertex_input_sci{wrapper::make_info<VkPipelineVertexInputStateCreateInfo>()};
    VkPipelineInputAssemblyStateCreateInfo input_assembly_sci{
        wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>()};
    VkPipelineTessellationStateCreateInfo tesselation_sci{wrapper::make_info<VkPipelineTessellationStateCreateInfo>()};
    VkPipelineViewportStateCreateInfo viewport_sci{wrapper::make_info<VkPipelineViewportStateCreateInfo>()};
    VkPipelineRasterizationStateCreateInfo rasterization_sci{
        wrapper::make_info<VkPipelineRasterizationStateCreateInfo>()};
    VkPipelineDepthStencilStateCreateInfo depth_stencil_sci{
        wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>()};
    VkPipelineRenderingCreateInfo pipeline_rendering_ci{wrapper::make_info<VkPipelineRenderingCreateInfo>()};
    VkPipelineMultisampleStateCreateInfo multisample_sci{wrapper::make_info<VkPipelineMultisampleStateCreateInfo>()};
    VkPipelineColorBlendStateCreateInfo color_blend_sci{wrapper::make_info<VkPipelineColorBlendStateCreateInfo>()};
    VkPipelineDynamicStateCreateInfo dynamic_states_sci{wrapper::make_info<VkPipelineDynamicStateCreateInfo>()};
};

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
    friend class commands::CommandBuffer;
    friend class render_graph::RenderGraph;

private:
    const Device &m_device;
    std::string m_name;
    VkPipeline m_pipeline;
    std::unique_ptr<PipelineLayout> m_pipeline_layout;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    /// @param pipeline_setup_data The graphics pipeline setup data
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device, const PipelineCache &pipeline_cache,
                     GraphicsPipelineSetupData pipeline_setup_data, bool use_dynamic_rendering, std::string name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

    [[nodiscard]] auto pipeline() const {
        return m_pipeline;
    }

    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
