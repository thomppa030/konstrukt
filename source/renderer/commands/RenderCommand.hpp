#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#include "renderer/resources/ResourceID.hpp"

namespace kst::renderer::command {

  enum class RenderCommandType : std::uint8_t {
    CLEAR,
    DRAW,
    DRAW_INDEXED,
    DISPATCH,
    COPY,
    SET_VIEWPORT,
    SET_SCISSOR,
  };

  enum class ClearFlags : std::uint8_t {
    NONE    = 0U,
    COLOR   = 1U << 0U,
    DEPTH   = 1U << 1U,
    STENCIL = 1U << 2U,
    ALL     = COLOR | DEPTH | STENCIL
  };

  struct ClearCommandData {
    glm::vec4 color;
    float depth;
    uint32_t stencil;
    ClearFlags flags;
  };

  struct DrawCommandData {
    resource::ResourceID meshId;
    resource::ResourceID materialID;
    glm::mat4 transform;
    uint32_t vertexCount;
    uint32_t instanceCount;
  };

  /*
   * The RenderCommand represents a single rendering operation:
   *   - Uses Enums and union Types for type-safe, memory efficient commands
   *   - Contains all data needed for the operation
   *   - Designed for contiguous storage and cache-friendly passing
   * */
  struct RenderCommand {
    RenderCommandType type;
    union {
      ClearCommandData clearData{};
      DrawCommandData drawData;
    };
  };

} // namespace kst::renderer::command
