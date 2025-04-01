#include "Layer.hpp"

#include <string>
#include <utility>

namespace kst::core::application {

  Layer::Layer(std::string name) : m_name(std::move(name)) {}

  Layer::~Layer() = default;

} // namespace kst::core::application
