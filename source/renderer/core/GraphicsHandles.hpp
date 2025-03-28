#pragma once

#include <cstdint>
#include <functional>

namespace kst::core {

  struct BufferHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const BufferHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const BufferHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct TextureHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const TextureHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const TextureHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct SamplerHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const SamplerHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const SamplerHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct ShaderHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const ShaderHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const ShaderHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct PipelineHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const PipelineHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const PipelineHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct FramebufferHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const FramebufferHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const FramebufferHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct RenderPassHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const RenderPassHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const RenderPassHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct CommandBufferHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const CommandBufferHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const CommandBufferHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  struct QueryHandle {
    uint64_t id = 0;

    constexpr auto isValid() const -> bool { return id != 0; }
    constexpr auto operator==(const QueryHandle& other) const -> bool { return id == other.id; }
    constexpr auto operator!=(const QueryHandle& other) const -> bool { return id != other.id; }
    explicit constexpr operator bool() const { return isValid(); }
  };

  // NULL Handles

  constexpr BufferHandle NULL_BUFFER_HANDLE                = {0};
  constexpr TextureHandle NULL_TEXTURE_HANDLE              = {0};
  constexpr SamplerHandle NULL_SAMPLER_HANDLE              = {0};
  constexpr ShaderHandle NULL_SHADER_HANDLE                = {0};
  constexpr PipelineHandle NULL_PIPELINE_HANDLE            = {0};
  constexpr FramebufferHandle NULL_FRAMEBUFFER_HANDLE      = {0};
  constexpr RenderPassHandle NULL_RENDERPASS_HANDLE        = {0};
  constexpr CommandBufferHandle NULL_COMMAND_BUFFER_HANDLE = {0};
  constexpr QueryHandle NULL_QUERY_HANDLE                  = {0};

} // namespace kst::core

namespace std {
  template <>
  struct hash<kst::core::BufferHandle> {
    auto operator()(const kst::core::BufferHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::TextureHandle> {
    auto operator()(const kst::core::TextureHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::SamplerHandle> {
    auto operator()(const kst::core::SamplerHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::ShaderHandle> {
    auto operator()(const kst::core::ShaderHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::PipelineHandle> {
    auto operator()(const kst::core::PipelineHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::FramebufferHandle> {
    auto operator()(const kst::core::FramebufferHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::RenderPassHandle> {
    auto operator()(const kst::core::RenderPassHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::CommandBufferHandle> {
    auto operator()(const kst::core::CommandBufferHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };

  template <>
  struct hash<kst::core::QueryHandle> {
    auto operator()(const kst::core::QueryHandle& handle) const -> size_t {
      return hash<uint64_t>()(handle.id);
    }
  };
} // namespace std
