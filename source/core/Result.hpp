#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace kst::core {

/**
 *  @class Result
 *  @brief A generic container for operation results that can either succeed
 * with a value or fail with and error
 *
 *  @tparam T The type of the success value
 * */
template <typename T> class Result {
public:
  /**
   *  @brief Construct a failed Result
   *  @param error Description of the error
   * */
  static auto error(std::string error) -> Result<T> {
    Result<T> result;
    result.m_error = std::move(error);
    return result;
  }

  /**
   *  @brief Construct a successful Result
   *  @param value The success value
   * */
  static auto success(T value) -> Result<T> {
    Result<T> result;
    result.m_value = std::move(value);
    return result;
  }

  /**
   *  @brief Default constructor
   *  creates a failed Result with empty error
   * */
  Result() = default;

  /**
   *  @brief Construct a Result from a value
   *  @param value The success value
   * */
  explicit Result(T value) : m_value(std::move(value)) {}

  /**
   * @brief
   */
  explicit operator bool() const { return hasValue(); }

  auto hasValue() const -> bool { return m_value.has_value(); }

  auto hasError() const -> bool { return !m_value.has_value(); }

  auto value() -> T & { return *m_value; }

  auto value() const -> const T & { return *m_value; }

  auto error() const -> const std::string & { return m_error; }

  auto operator->() -> T * { return &(*m_value); }

  auto operator->() const -> const T * { return &(*m_value); }

  auto valueOr(const T &defaultValue) const -> T {
    return m_value.value_or(defaultValue);
  }

  template <typename U>
  auto map(std::function<U(const T &)> func) const -> Result<U> {
    if (hasValue()) {
      return Result<U>::success(func(*m_value));
    }
    return Result<U>::error(m_error);
  }

  template <typename Fn>
  auto andThen(Fn &&func) const -> decltype(func(std::declval<T>())) {
    using ReturnType = decltype(func(std::declval<T>()));

    if (hasValue()) {
      return func(*m_value);
    }

    return ReturnType::error(m_error);
  }

  auto onSuccess(std::function<void(const T &)> func) -> Result<T> & {
    if (hasValue()) {
      func(*m_value);
    }
    return *this;
  }

  template <typename Fn,
            typename = std::enable_if<std::is_invocable_v<Fn, const T &>>>
  auto onSuccess(Fn &&func) -> Result<T> & {
    if (hasValue()) {
      func(*m_value);
    }

    return *this;
  }

  auto onError(std::function<void(const std::string &)> func) -> Result<T> & {
    if (hasError()) {
      func(m_error);
    }
    return *this;
  }

  template <typename Fn, typename std::enable_if<
                             std::is_invocable_v<Fn, const std::string &>>>
  auto onError(Fn &&func) -> Result<T> & {
    if (hasError()) {
      func(m_error);
    }
    return *this;
  }

private:
  std::optional<T> m_value;
  std::string m_error;
};

template <> class Result<void> {
public:
  static auto error(std::string error) -> Result<void> {
    Result<void> result;
    result.m_error = std::move(error);
    result.m_success = false;
    return result;
  }

  static auto success() -> Result<void> {
    Result<void> result;
    result.m_success = true;
    return result;
  }

  Result() : m_success(true) {}

  explicit operator bool() const { return m_success; }

  auto hasValue() const -> bool { return m_success; }

  auto hasError() const -> bool { return !m_success; }

  auto error() const -> const std::string & { return m_error; }

  template <typename Fn> auto andThen(Fn &&func) const -> decltype(func()) {
    using ReturnType = decltype(func());

    if (m_success) {
      return func;
    }

    return ReturnType::error(m_error);
  }

  template <typename Fn> auto onSuccess(std::function<void()> func) {
    if (m_success) {
      func();
    }

    return *this;
  }

  template <typename Fn, typename = std::enable_if<std::is_invocable_v<Fn>>>
  auto onSuccess(Fn &&func) -> Result<void> & {
    if (m_success) {
      func();
    }

    return *this;
  }

  template <typename Fn, typename std::enable_if<
                             std::is_invocable_v<Fn, const std::string &>>>
  auto onError(std::function<void(const std::string &)> func)
      -> Result<void> & {
    if (hasError()) {
      func(m_error);
    }

    return *this;
  }

  auto onError(std::function<void(const std::string &)> func)
      -> Result<void> & {
    if (hasError()) {
      func(m_error);
    }

    return *this;
  }

private:
  bool m_success;
  std::string m_error;
};

} // namespace kst::core
