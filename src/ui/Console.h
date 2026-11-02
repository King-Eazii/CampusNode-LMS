#pragma once
#include <exception>
#include <string>

namespace lms::console {

/// Thrown when the input ends (for example Ctrl+D), so the program can stop cleanly.
struct InputClosed : std::exception {
    const char* what() const noexcept override { return "Input closed"; }
};

std::string readLine(const std::string& prompt);
/// Reads a password without showing it when the input is a terminal.
std::string readSecret(const std::string& prompt);
void print(const std::string& text = "");
void heading(const std::string& title);
void error(const std::string& message);
std::string fit(const std::string& text, std::size_t width);  // pads or shortens to an exact width
std::string number(double value);                              // "12.5" or "12"

}  // namespace lms::console
