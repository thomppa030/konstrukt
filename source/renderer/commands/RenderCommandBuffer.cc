#include "RenderCommandBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <boost/range/algorithm/stable_sort.hpp>

#include "renderer/framegraph/FramegraphBuilder.hpp"
#include "renderer/commands/RenderCommand.hpp"

namespace kst::renderer::command {
  void RenderCommandBuffer::clear() {
    m_commands.clear();
  }

  auto RenderCommandBuffer::getCommands() const -> const RenderCommand* {
    return m_commands.data();
  }

  auto RenderCommandBuffer::getCommandCount() const -> size_t {
    return m_commands.size();
  }

  void RenderCommandBuffer::sort() {
    // Sort commands for better GPU performance
    // This can be done based on various criteria:
    // 1. Sort by command type to minimize state changed
    // 2. Sort by resources to improve cache coherence
    // 3. Sort by pipeline / material to minimize binding changes

    boost::range::stable_sort(m_commands, [](const RenderCommand& lhs, const RenderCommand& rhs) {
      if (lhs.type != rhs.type) {
        return static_cast<uint8_t>(lhs.type) < static_cast<uint8_t>(rhs.type);
      }

      switch (lhs.type) {
      case RenderCommandType::DRAW:
      case RenderCommandType::DRAW_INDEXED:
        return lhs.drawData.materialID.index < rhs.drawData.materialID.index;
      default:
        return false;
      }
    });
  }

  template <>
  void RenderCommandBuffer::submit<ClearCommandData>(
      RenderCommandType type,
      const ClearCommandData& data
  ) {
    RenderCommand cmd;
    cmd.type      = type;
    cmd.clearData = data;
    m_commands.push_back(cmd);
  }

  template <>
  void RenderCommandBuffer::submit<DrawCommandData>(
      RenderCommandType type,
      const DrawCommandData& data
  ) {
    RenderCommand cmd;
    cmd.type     = type;
    cmd.drawData = data;
    m_commands.push_back(cmd);
  }
} // namespace kst::renderer::command
