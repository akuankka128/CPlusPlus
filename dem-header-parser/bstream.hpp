#pragma once

#include <memory>     // Smart pointers
#include <cstring>    // `memcmp`
#include <istream>    // `istream`
#include <stdexcept>  // Exceptions

// chore(nb): Should this be moved into its own
// namespace or named differently to distinguish
// between custom and standard classes?

/// Utility class for binary streams
class bstream : std::istream {
public:
  bstream (std::streambuf *buf)
    : std::istream(buf)
  {
  }

  /// Read a value and advance stream by `sizeof(T)` bytes
  /// @throws `std::out_of_range` If the stream doesn't
  ///   have enough data to read
  template<typename T>
  T get () {
    char buffer[sizeof(T)];
    read(buffer, sizeof(T));

    if (gcount() != sizeof(T)) {
      throw std::out_of_range(
        "could not satisfy size of T from stream\n"
      );
    }

    T value = *reinterpret_cast<T *>(buffer);
    return value;
  }

  /// Read `N` characters and advance stream by that much
  /// @throws `std::out_of_range` If the stream doesn't
  ///   have enough data to read
  template<size_t N>
  std::unique_ptr<char[]> get () {
    auto buffer = std::make_unique<char[]>(N);
    read(buffer.get(), N);

    if (gcount() != N) {
      throw std::out_of_range(
        "could not satisfy width N from stream\n"
      );
    }

    return buffer;
  }

  /// Assert that the next `T` value matches the
  /// given value and advance stream by `sizeof(T)`
  /// @throws `std::logic_error` If inequal
  /// @throws `std::out_of_range` For OOB reads
  /// @returns `true` If equal
  template<typename T>
  bool expect (T value) {
    T data = get<T>();
    if (data != value) {
      // chore(if-minor): DRY error reporting
      std::string error_msg = "Assertion error.\n";
      error_msg += "\tExpected: ";
      error_msg += value;
      error_msg += "\n";

      error_msg += "\tProvided: ";
      error_msg += data;
      error_msg += "\n";

      throw std::logic_error(error_msg);
    }

    return true;
  }

  /// Assert that the next `N` bytes match the
  /// given value and advance stream by `N` bytes
  /// @throws `std::logic_error` If inequal
  /// @throws `std::out_of_range` For OOB reads
  /// @returns `true` If equal
  template<size_t N>
  bool expect_chars (const char (&value)[N + 1]) {
    auto data = get<N>();

    // std::strncmp() stops at null bytes, memcmp doesn't
    if (std::memcmp(data.get(), value, N) != 0) {
      std::string error_msg = "Assertion error.\n";
      error_msg += "\tExpected: ";
      error_msg += value;
      error_msg += "\n";

      error_msg += "\tProvided: ";
      error_msg += data.get();
      error_msg += "\n";

      throw std::logic_error(error_msg);
    }

    return true;
  }
};
