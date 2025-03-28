#include <iostream>
#include <limits>
#include <unordered_set>
#include <unordered_map>

#include <gtest/gtest.h>

#include <renderer/core/GraphicsHandles.hpp>

using namespace kst::core;

// Test the default initialization of all handle types
TEST(GraphicsHandlesTest, DefaultInitialization) {
  // Test that default constructor initializes id to 0
  BufferHandle bufferHandle{};
  EXPECT_EQ(bufferHandle.id, 0);
  
  TextureHandle textureHandle{};
  EXPECT_EQ(textureHandle.id, 0);
  
  SamplerHandle samplerHandle{};
  EXPECT_EQ(samplerHandle.id, 0);
  
  ShaderHandle shaderHandle{};
  EXPECT_EQ(shaderHandle.id, 0);
  
  PipelineHandle pipelineHandle{};
  EXPECT_EQ(pipelineHandle.id, 0);
  
  FramebufferHandle framebufferHandle{};
  EXPECT_EQ(framebufferHandle.id, 0);
  
  RenderPassHandle renderPassHandle{};
  EXPECT_EQ(renderPassHandle.id, 0);
  
  CommandBufferHandle commandBufferHandle{};
  EXPECT_EQ(commandBufferHandle.id, 0);
  
  QueryHandle queryHandle{};
  EXPECT_EQ(queryHandle.id, 0);
}

// Test explicit initialization with different values
TEST(GraphicsHandlesTest, ExplicitInitialization) {
  // Test initialization with specific values
  BufferHandle bufferHandle{42};
  EXPECT_EQ(bufferHandle.id, 42);
  
  TextureHandle textureHandle{123};
  EXPECT_EQ(textureHandle.id, 123);
  
  SamplerHandle samplerHandle{456};
  EXPECT_EQ(samplerHandle.id, 456);
  
  ShaderHandle shaderHandle{789};
  EXPECT_EQ(shaderHandle.id, 789);
  
  // Test with larger values
  PipelineHandle pipelineHandle{12345678};
  EXPECT_EQ(pipelineHandle.id, 12345678);
  
  // Test with maximum uint64_t value
  uint64_t max_uint64 = std::numeric_limits<uint64_t>::max();
  FramebufferHandle framebufferHandle{max_uint64};
  EXPECT_EQ(framebufferHandle.id, max_uint64);
}

// Test isValid() method for all handle types
TEST(GraphicsHandlesTest, IsValidMethod) {
  // Test isValid() method returns false for id=0
  BufferHandle invalidBuffer{0};
  EXPECT_FALSE(invalidBuffer.isValid());
  
  // Test isValid() method returns true for id!=0
  BufferHandle validBuffer{1};
  EXPECT_TRUE(validBuffer.isValid());
  
  TextureHandle invalidTexture{0};
  EXPECT_FALSE(invalidTexture.isValid());
  
  TextureHandle validTexture{1};
  EXPECT_TRUE(validTexture.isValid());
  
  SamplerHandle invalidSampler{0};
  EXPECT_FALSE(invalidSampler.isValid());
  
  SamplerHandle validSampler{1};
  EXPECT_TRUE(validSampler.isValid());
  
  ShaderHandle invalidShader{0};
  EXPECT_FALSE(invalidShader.isValid());
  
  ShaderHandle validShader{1};
  EXPECT_TRUE(validShader.isValid());
  
  PipelineHandle invalidPipeline{0};
  EXPECT_FALSE(invalidPipeline.isValid());
  
  PipelineHandle validPipeline{1};
  EXPECT_TRUE(validPipeline.isValid());
  
  FramebufferHandle invalidFramebuffer{0};
  EXPECT_FALSE(invalidFramebuffer.isValid());
  
  FramebufferHandle validFramebuffer{1};
  EXPECT_TRUE(validFramebuffer.isValid());
  
  RenderPassHandle invalidRenderPass{0};
  EXPECT_FALSE(invalidRenderPass.isValid());
  
  RenderPassHandle validRenderPass{1};
  EXPECT_TRUE(validRenderPass.isValid());
  
  CommandBufferHandle invalidCommandBuffer{0};
  EXPECT_FALSE(invalidCommandBuffer.isValid());
  
  CommandBufferHandle validCommandBuffer{1};
  EXPECT_TRUE(validCommandBuffer.isValid());
  
  QueryHandle invalidQuery{0};
  EXPECT_FALSE(invalidQuery.isValid());
  
  QueryHandle validQuery{1};
  EXPECT_TRUE(validQuery.isValid());
}

// Test comparison operators for each handle type
TEST(GraphicsHandlesTest, ComparisonOperators) {
  // BufferHandle comparison operators
  BufferHandle buffer1{1};
  BufferHandle buffer2{1};
  BufferHandle buffer3{2};
  
  EXPECT_TRUE(buffer1 == buffer2);
  EXPECT_FALSE(buffer1 != buffer2);
  EXPECT_FALSE(buffer1 == buffer3);
  EXPECT_TRUE(buffer1 != buffer3);
  
  // TextureHandle comparison operators
  TextureHandle texture1{1};
  TextureHandle texture2{1};
  TextureHandle texture3{2};
  
  EXPECT_TRUE(texture1 == texture2);
  EXPECT_FALSE(texture1 != texture2);
  EXPECT_FALSE(texture1 == texture3);
  EXPECT_TRUE(texture1 != texture3);
  
  // SamplerHandle comparison operators
  SamplerHandle sampler1{1};
  SamplerHandle sampler2{1};
  SamplerHandle sampler3{2};
  
  EXPECT_TRUE(sampler1 == sampler2);
  EXPECT_FALSE(sampler1 != sampler2);
  EXPECT_FALSE(sampler1 == sampler3);
  EXPECT_TRUE(sampler1 != sampler3);
  
  // ShaderHandle comparison operators
  ShaderHandle shader1{1};
  ShaderHandle shader2{1};
  ShaderHandle shader3{2};
  
  EXPECT_TRUE(shader1 == shader2);
  EXPECT_FALSE(shader1 != shader2);
  EXPECT_FALSE(shader1 == shader3);
  EXPECT_TRUE(shader1 != shader3);
  
  // PipelineHandle comparison operators
  PipelineHandle pipeline1{1};
  PipelineHandle pipeline2{1};
  PipelineHandle pipeline3{2};
  
  EXPECT_TRUE(pipeline1 == pipeline2);
  EXPECT_FALSE(pipeline1 != pipeline2);
  EXPECT_FALSE(pipeline1 == pipeline3);
  EXPECT_TRUE(pipeline1 != pipeline3);
  
  // FramebufferHandle comparison operators
  FramebufferHandle framebuffer1{1};
  FramebufferHandle framebuffer2{1};
  FramebufferHandle framebuffer3{2};
  
  EXPECT_TRUE(framebuffer1 == framebuffer2);
  EXPECT_FALSE(framebuffer1 != framebuffer2);
  EXPECT_FALSE(framebuffer1 == framebuffer3);
  EXPECT_TRUE(framebuffer1 != framebuffer3);
  
  // RenderPassHandle comparison operators
  RenderPassHandle renderPass1{1};
  RenderPassHandle renderPass2{1};
  RenderPassHandle renderPass3{2};
  
  EXPECT_TRUE(renderPass1 == renderPass2);
  EXPECT_FALSE(renderPass1 != renderPass2);
  EXPECT_FALSE(renderPass1 == renderPass3);
  EXPECT_TRUE(renderPass1 != renderPass3);
  
  // CommandBufferHandle comparison operators
  CommandBufferHandle commandBuffer1{1};
  CommandBufferHandle commandBuffer2{1};
  CommandBufferHandle commandBuffer3{2};
  
  EXPECT_TRUE(commandBuffer1 == commandBuffer2);
  EXPECT_FALSE(commandBuffer1 != commandBuffer2);
  EXPECT_FALSE(commandBuffer1 == commandBuffer3);
  EXPECT_TRUE(commandBuffer1 != commandBuffer3);
  
  // QueryHandle comparison operators
  QueryHandle query1{1};
  QueryHandle query2{1};
  QueryHandle query3{2};
  
  EXPECT_TRUE(query1 == query2);
  EXPECT_FALSE(query1 != query2);
  EXPECT_FALSE(query1 == query3);
  EXPECT_TRUE(query1 != query3);
}

// Test boolean conversion for all handle types
TEST(GraphicsHandlesTest, BooleanConversion) {
  // BufferHandle boolean conversion
  BufferHandle invalidBuffer{0};
  BufferHandle validBuffer{1};
  EXPECT_FALSE(static_cast<bool>(invalidBuffer));
  EXPECT_TRUE(static_cast<bool>(validBuffer));
  
  // TextureHandle boolean conversion
  TextureHandle invalidTexture{0};
  TextureHandle validTexture{1};
  EXPECT_FALSE(static_cast<bool>(invalidTexture));
  EXPECT_TRUE(static_cast<bool>(validTexture));
  
  // SamplerHandle boolean conversion
  SamplerHandle invalidSampler{0};
  SamplerHandle validSampler{1};
  EXPECT_FALSE(static_cast<bool>(invalidSampler));
  EXPECT_TRUE(static_cast<bool>(validSampler));
  
  // ShaderHandle boolean conversion
  ShaderHandle invalidShader{0};
  ShaderHandle validShader{1};
  EXPECT_FALSE(static_cast<bool>(invalidShader));
  EXPECT_TRUE(static_cast<bool>(validShader));
  
  // PipelineHandle boolean conversion
  PipelineHandle invalidPipeline{0};
  PipelineHandle validPipeline{1};
  EXPECT_FALSE(static_cast<bool>(invalidPipeline));
  EXPECT_TRUE(static_cast<bool>(validPipeline));
  
  // FramebufferHandle boolean conversion
  FramebufferHandle invalidFramebuffer{0};
  FramebufferHandle validFramebuffer{1};
  EXPECT_FALSE(static_cast<bool>(invalidFramebuffer));
  EXPECT_TRUE(static_cast<bool>(validFramebuffer));
  
  // RenderPassHandle boolean conversion
  RenderPassHandle invalidRenderPass{0};
  RenderPassHandle validRenderPass{1};
  EXPECT_FALSE(static_cast<bool>(invalidRenderPass));
  EXPECT_TRUE(static_cast<bool>(validRenderPass));
  
  // CommandBufferHandle boolean conversion
  CommandBufferHandle invalidCommandBuffer{0};
  CommandBufferHandle validCommandBuffer{1};
  EXPECT_FALSE(static_cast<bool>(invalidCommandBuffer));
  EXPECT_TRUE(static_cast<bool>(validCommandBuffer));
  
  // QueryHandle boolean conversion
  QueryHandle invalidQuery{0};
  QueryHandle validQuery{1};
  EXPECT_FALSE(static_cast<bool>(invalidQuery));
  EXPECT_TRUE(static_cast<bool>(validQuery));
  
  // Test if condition usage
  BufferHandle testBuffer{42};
  int count = 0;
  if (testBuffer) {
    count++;
  }
  EXPECT_EQ(count, 1);
  
  testBuffer = BufferHandle{0};
  if (testBuffer) {
    count++;
  }
  EXPECT_EQ(count, 1); // Should not increment
}

// Test NULL handle constants
TEST(GraphicsHandlesTest, NullHandles) {
  // Test NULL handle constants
  EXPECT_EQ(NULL_BUFFER_HANDLE.id, 0);
  EXPECT_EQ(NULL_TEXTURE_HANDLE.id, 0);
  EXPECT_EQ(NULL_SAMPLER_HANDLE.id, 0);
  EXPECT_EQ(NULL_SHADER_HANDLE.id, 0);
  EXPECT_EQ(NULL_PIPELINE_HANDLE.id, 0);
  EXPECT_EQ(NULL_FRAMEBUFFER_HANDLE.id, 0);
  EXPECT_EQ(NULL_RENDERPASS_HANDLE.id, 0);
  EXPECT_EQ(NULL_COMMAND_BUFFER_HANDLE.id, 0);
  EXPECT_EQ(NULL_QUERY_HANDLE.id, 0);
  
  // Test isValid() on NULL handles
  EXPECT_FALSE(NULL_BUFFER_HANDLE.isValid());
  EXPECT_FALSE(NULL_TEXTURE_HANDLE.isValid());
  EXPECT_FALSE(NULL_SAMPLER_HANDLE.isValid());
  EXPECT_FALSE(NULL_SHADER_HANDLE.isValid());
  EXPECT_FALSE(NULL_PIPELINE_HANDLE.isValid());
  EXPECT_FALSE(NULL_FRAMEBUFFER_HANDLE.isValid());
  EXPECT_FALSE(NULL_RENDERPASS_HANDLE.isValid());
  EXPECT_FALSE(NULL_COMMAND_BUFFER_HANDLE.isValid());
  EXPECT_FALSE(NULL_QUERY_HANDLE.isValid());
  
  // Test boolean conversion of NULL handles
  EXPECT_FALSE(static_cast<bool>(NULL_BUFFER_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_TEXTURE_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_SAMPLER_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_SHADER_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_PIPELINE_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_FRAMEBUFFER_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_RENDERPASS_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_COMMAND_BUFFER_HANDLE));
  EXPECT_FALSE(static_cast<bool>(NULL_QUERY_HANDLE));
  
  // Test equality of handles with NULL handles
  BufferHandle buffer{0};
  EXPECT_TRUE(buffer == NULL_BUFFER_HANDLE);
  EXPECT_FALSE(buffer != NULL_BUFFER_HANDLE);
  
  TextureHandle texture{0};
  EXPECT_TRUE(texture == NULL_TEXTURE_HANDLE);
  EXPECT_FALSE(texture != NULL_TEXTURE_HANDLE);
}

// Test handle hash functions
TEST(GraphicsHandlesTest, HashFunction) {
  // Test hash function for BufferHandle
  BufferHandle buffer1{1};
  BufferHandle buffer2{1};
  BufferHandle buffer3{2};
  
  std::hash<BufferHandle> bufferHasher;
  EXPECT_EQ(bufferHasher(buffer1), bufferHasher(buffer2));
  EXPECT_NE(bufferHasher(buffer1), bufferHasher(buffer3));
  
  // Test hash function for TextureHandle
  TextureHandle texture1{1};
  TextureHandle texture2{1};
  TextureHandle texture3{2};
  
  std::hash<TextureHandle> textureHasher;
  EXPECT_EQ(textureHasher(texture1), textureHasher(texture2));
  EXPECT_NE(textureHasher(texture1), textureHasher(texture3));
  
  // Test hash function for SamplerHandle
  SamplerHandle sampler1{1};
  SamplerHandle sampler2{1};
  SamplerHandle sampler3{2};
  
  std::hash<SamplerHandle> samplerHasher;
  EXPECT_EQ(samplerHasher(sampler1), samplerHasher(sampler2));
  EXPECT_NE(samplerHasher(sampler1), samplerHasher(sampler3));
  
  // Test hash function for other handle types
  std::hash<ShaderHandle> shaderHasher;
  std::hash<PipelineHandle> pipelineHasher;
  std::hash<FramebufferHandle> framebufferHasher;
  std::hash<RenderPassHandle> renderPassHasher;
  std::hash<CommandBufferHandle> commandBufferHasher;
  std::hash<QueryHandle> queryHasher;
  
  ShaderHandle shader1{1}, shader2{1}, shader3{2};
  PipelineHandle pipeline1{1}, pipeline2{1}, pipeline3{2};
  FramebufferHandle framebuffer1{1}, framebuffer2{1}, framebuffer3{2};
  RenderPassHandle renderPass1{1}, renderPass2{1}, renderPass3{2};
  CommandBufferHandle cmdBuffer1{1}, cmdBuffer2{1}, cmdBuffer3{2};
  QueryHandle query1{1}, query2{1}, query3{2};
  
  EXPECT_EQ(shaderHasher(shader1), shaderHasher(shader2));
  EXPECT_NE(shaderHasher(shader1), shaderHasher(shader3));
  
  EXPECT_EQ(pipelineHasher(pipeline1), pipelineHasher(pipeline2));
  EXPECT_NE(pipelineHasher(pipeline1), pipelineHasher(pipeline3));
  
  EXPECT_EQ(framebufferHasher(framebuffer1), framebufferHasher(framebuffer2));
  EXPECT_NE(framebufferHasher(framebuffer1), framebufferHasher(framebuffer3));
  
  EXPECT_EQ(renderPassHasher(renderPass1), renderPassHasher(renderPass2));
  EXPECT_NE(renderPassHasher(renderPass1), renderPassHasher(renderPass3));
  
  EXPECT_EQ(commandBufferHasher(cmdBuffer1), commandBufferHasher(cmdBuffer2));
  EXPECT_NE(commandBufferHasher(cmdBuffer1), commandBufferHasher(cmdBuffer3));
  
  EXPECT_EQ(queryHasher(query1), queryHasher(query2));
  EXPECT_NE(queryHasher(query1), queryHasher(query3));
}

// Test handle usage in std::unordered_set
TEST(GraphicsHandlesTest, UnorderedSetUsage) {
  // Create handles with different IDs
  BufferHandle buffer1{1};
  BufferHandle buffer2{2};
  BufferHandle buffer3{3};
  BufferHandle buffer1Duplicate{1}; // Same ID as buffer1
  
  // Test std::unordered_set with BufferHandle
  std::unordered_set<BufferHandle> bufferSet;
  bufferSet.insert(buffer1);
  bufferSet.insert(buffer2);
  bufferSet.insert(buffer3);
  
  // Set should have 3 elements
  EXPECT_EQ(bufferSet.size(), 3);
  
  // Inserting duplicate should not change size
  bufferSet.insert(buffer1Duplicate);
  EXPECT_EQ(bufferSet.size(), 3);
  
  // Check existence in set
  EXPECT_TRUE(bufferSet.count(buffer1) == 1);
  EXPECT_TRUE(bufferSet.count(buffer1Duplicate) == 1); // Should find duplicate
  EXPECT_TRUE(bufferSet.count(BufferHandle{4}) == 0);  // Shouldn't find non-inserted handle
  
  // Similar test for TextureHandle
  TextureHandle texture1{1};
  TextureHandle texture2{2};
  TextureHandle texture1Duplicate{1};
  
  std::unordered_set<TextureHandle> textureSet;
  textureSet.insert(texture1);
  textureSet.insert(texture2);
  EXPECT_EQ(textureSet.size(), 2);
  textureSet.insert(texture1Duplicate);
  EXPECT_EQ(textureSet.size(), 2);
  
  // Test with NULL handles
  textureSet.insert(NULL_TEXTURE_HANDLE);
  EXPECT_EQ(textureSet.size(), 3); // Should add NULL handle
  
  // Check all handle types work in unordered_set
  std::unordered_set<ShaderHandle> shaderSet;
  std::unordered_set<PipelineHandle> pipelineSet;
  std::unordered_set<FramebufferHandle> framebufferSet;
  
  shaderSet.insert(ShaderHandle{1});
  pipelineSet.insert(PipelineHandle{1});
  framebufferSet.insert(FramebufferHandle{1});
  
  EXPECT_EQ(shaderSet.size(), 1);
  EXPECT_EQ(pipelineSet.size(), 1);
  EXPECT_EQ(framebufferSet.size(), 1);
}

// Test handle usage in std::unordered_map
TEST(GraphicsHandlesTest, UnorderedMapUsage) {
  // Create test handles
  BufferHandle buffer1{1};
  BufferHandle buffer2{2};
  TextureHandle texture1{1};
  TextureHandle texture2{2};
  
  // Test with BufferHandle as key
  std::unordered_map<BufferHandle, std::string> bufferMap;
  bufferMap[buffer1] = "Buffer 1";
  bufferMap[buffer2] = "Buffer 2";
  
  EXPECT_EQ(bufferMap.size(), 2);
  EXPECT_EQ(bufferMap[buffer1], "Buffer 1");
  EXPECT_EQ(bufferMap[buffer2], "Buffer 2");
  
  // Test with TextureHandle as key
  std::unordered_map<TextureHandle, int> textureMap;
  textureMap[texture1] = 100;
  textureMap[texture2] = 200;
  
  EXPECT_EQ(textureMap.size(), 2);
  EXPECT_EQ(textureMap[texture1], 100);
  EXPECT_EQ(textureMap[texture2], 200);
  
  // Test updating values
  bufferMap[buffer1] = "Updated Buffer 1";
  EXPECT_EQ(bufferMap[buffer1], "Updated Buffer 1");
  
  // Test with NULL handle
  bufferMap[NULL_BUFFER_HANDLE] = "Null Buffer";
  EXPECT_EQ(bufferMap.size(), 3);
  EXPECT_EQ(bufferMap[NULL_BUFFER_HANDLE], "Null Buffer");
  
  // Check other handle types as map keys
  std::unordered_map<ShaderHandle, std::string> shaderMap;
  std::unordered_map<PipelineHandle, std::string> pipelineMap;
  
  ShaderHandle shader{42};
  PipelineHandle pipeline{42};
  
  shaderMap[shader] = "Shader 42";
  pipelineMap[pipeline] = "Pipeline 42";
  
  EXPECT_EQ(shaderMap[shader], "Shader 42");
  EXPECT_EQ(pipelineMap[pipeline], "Pipeline 42");
}

// Test edge cases
TEST(GraphicsHandlesTest, EdgeCases) {
  // Test with maximum uint64_t value
  uint64_t max_uint64 = std::numeric_limits<uint64_t>::max();
  BufferHandle maxBuffer{max_uint64};
  
  EXPECT_EQ(maxBuffer.id, max_uint64);
  EXPECT_TRUE(maxBuffer.isValid());
  EXPECT_TRUE(static_cast<bool>(maxBuffer));
  EXPECT_TRUE(maxBuffer != NULL_BUFFER_HANDLE);
  
  // Test with sequential values
  std::vector<BufferHandle> handles;
  for (uint64_t i = 0; i < 100; i++) {
    handles.push_back(BufferHandle{i});
  }
  
  for (uint64_t i = 0; i < 100; i++) {
    EXPECT_EQ(handles[i].id, i);
    EXPECT_EQ(handles[i].isValid(), i != 0);
  }
  
  // Test comparison between different handle types
  // This would ideally cause a compilation error, but we can't test that in runtime tests
} 