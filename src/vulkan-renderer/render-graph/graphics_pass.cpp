#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::InexorException;

GraphicsPass::GraphicsPass(
    std::string name, std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
    std::vector<std::weak_ptr<Buffer>> buffer_reads,
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> texture_writes,
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> swapchain_writes,
    const wrapper::DebugLabelColor pass_debug_label_color) {
    // Pick any extent and store it, they must be all the same at this point
    if (!texture_writes.empty()) {
        const auto &attachment = texture_writes[0].first.lock();
        m_extent = {
            .width = attachment->extent().width,
            .height = attachment->extent().height,
        };
    } else if (!swapchain_writes.empty()) {
        // No color attachments, so pick the extent from any of the swapchains specified
        const auto &swapchain = swapchain_writes[0].first.lock();
        m_extent = swapchain->extent();
    }
    // Check if either width or height is 0
    if (m_extent.width == 0) {
        throw InexorException("Error: m_extent.width is 0!");
    }
    if (m_extent.height == 0) {
        throw InexorException("Error: m_extent.height is 0!");
    }

    // Store the other data
    m_name = std::move(name);
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    m_debug_label_color = wrapper::get_debug_label_color(pass_debug_label_color);
    m_texture_writes = std::move(texture_writes);
    m_swapchain_writes = std::move(swapchain_writes);
}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    // TODO: Check me!
    m_name = std::move(other.m_name);
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
    m_rendering_info = std::move(other.m_rendering_info);
    m_texture_writes = std::move(other.m_texture_writes);
    m_swapchain_writes = std::move(other.m_swapchain_writes);
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment = std::move(other.m_depth_attachment);
    m_stencil_attachment = std::move(other.m_stencil_attachment);
    m_debug_label_color = other.m_debug_label_color;
}

void GraphicsPass::reset_rendering_info() {
    m_rendering_info = wrapper::make_info<VkRenderingInfo>();
    m_color_attachments.clear();
    m_depth_attachment = std::nullopt;
    m_stencil_attachment = std::nullopt;
}

} // namespace inexor::vulkan_renderer::render_graph
