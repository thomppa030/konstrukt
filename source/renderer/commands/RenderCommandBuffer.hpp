#pragma once

#include <cstddef>
#include <vector>

#include "RenderCommand.hpp"

namespace kst::renderer::command {
  /*
   * The RenderCommandBuffer collects and manages rendering commands:
   * - stores commands in a contiguous array
   *   - provides methods to add different command types
   *   - can sort and optimize commands before execution
   *   - designed for efficient batch processing
   * */
  class RenderCommandBuffer {
  public:
    void clear();

    template <typename T>
    void submit(RenderCommandType type, const T& data);

    auto getCommands() const -> const RenderCommand*;
    auto getCommandCount() const -> size_t;

    void sort();

  private:
    std::vector<RenderCommand> m_commands;
  };

  // clang-format off
  template <>
  void RenderCommandBuffer::submit<ClearCommandData>(RenderCommandType type, const ClearCommandData& data );

  template <>
  void RenderCommandBuffer::submit<DrawCommandData>(RenderCommandType type, const DrawCommandData& data );
  // clang-format on

} // namespace kst::renderer::command
