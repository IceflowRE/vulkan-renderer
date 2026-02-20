#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain.hpp"

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;

// Using declarations
using wrapper::CommandBuffer;
using wrapper::descriptors::DescriptorSetLayout;
using wrapper::swapchains::Swapchain;

/// A wrapper for graphics passes inside of rendergraph
class GraphicsPass {
    friend class RenderGraph;

private:
    /// The name of the graphics pass
    std::string m_name;
    /// The command buffer recording function of the graphics pass
    std::function<void(const CommandBuffer &)> m_on_record_cmd_buffer{[](auto &) {}};
    /// The descriptor set layout of the pass (this will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
    /// The descriptor set of the pass (this will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};
    /// The color of the debug label region (visible in graphics debuggers like RenderDoc)
    std::array<float, 4> m_debug_label_color;
    /// The extent
    VkExtent2D m_extent{0, 0};

    /// The buffers which are read by this graphics pass
    std::vector<std::weak_ptr<Buffer>> m_buffer_reads;
    /// The texture attachments of this pass (unified means color, depth, stencil attachment or a swapchain)
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> m_texture_writes;
    /// The swapchains this graphics pass writes to
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> m_swapchain_writes;

    // All the data below will be filled and used by rendergraph only

    /// The rendering info will be filled during rendergraph compilation so we don't have to do this while rendering.
    /// This means we must make sure that the memory of the attachment infos below is still valid during rendering,
    /// which is why we store them as members here.
    VkRenderingInfo m_rendering_info{};
    /// The color attachments inside of m_rendering_info
    std::vector<VkRenderingAttachmentInfo> m_color_attachments{};
    /// The depth attachment inside of m_rendering_info
    std::optional<VkRenderingAttachmentInfo> m_depth_attachment{std::nullopt};
    /// The stencil attachment inside of m_rendering_info
    std::optional<VkRenderingAttachmentInfo> m_stencil_attachment{std::nullopt};

    /// Reset the rendering info
    void reset_rendering_info();

public:
    /// Default constructor
    /// @param name The name of the graphics pass
    /// @param on_record_cmd_buffer The command buffer recording function of the graphics pass
    /// @param buffer_reads The buffers which are read by this graphics pass
    /// @param texture_writes The textures which are written to by this graphics pass
    /// @param swapchain_writes The swapchains which are written to by this graphics pass
    /// @param pass_debug_label_color The debug label of the pass (visible in graphics debuggers like RenderDoc)
    GraphicsPass(std::string name, std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
                 std::vector<std::weak_ptr<Buffer>> buffer_reads,
                 std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> texture_writes,
                 std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> swapchain_writes,
                 wrapper::DebugLabelColor pass_debug_label_color);

    GraphicsPass(const GraphicsPass &) = delete;
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
