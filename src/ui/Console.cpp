#include "ui/Console.h"

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#define LMS_ISATTY _isatty
#define LMS_FILENO _fileno
#else
#include <termios.h>
#include <unistd.h>
#define LMS_ISATTY isatty
#define LMS_FILENO fileno
#endif

namespace lms::console {

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed();
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

std::string readSecret(const std::string& prompt) {
    if (!LMS_ISATTY(LMS_FILENO(stdin))) return readLine(prompt);
    std::cout << prompt << std::flush;
    std::string secret;
#ifdef _WIN32
    int c;
    while ((c = _getch()) != '\r' && c != '\n' && c != EOF) {
        if (c == '\b') { if (!secret.empty()) secret.pop_back(); }
        else secret.push_back(static_cast<char>(c));
    }
#else
    termios oldSettings{};
    tcgetattr(STDIN_FILENO, &oldSettings);
    termios hidden = oldSettings;
    hidden.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
    const bool ok = static_cast<bool>(std::getline(std::cin, secret));
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    if (!ok) throw InputClosed();
#endif
    std::cout << '\n';
    return secret;
}

void print(const std::string& text) { std::cout << text << '\n'; }

void heading(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
}

void error(const std::string& message) { std::cout << "Error: " << message << '\n'; }

std::string fit(const std::string& text, std::size_t width) {
    if (text.size() > width) return text.substr(0, width > 1 ? width - 1 : width) + (width > 1 ? "~" : "");
    return text + std::string(width - text.size(), ' ');
}

std::string number(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << value;
    std::string s = out.str();
    if (s.size() > 2 && s.substr(s.size() - 2) == ".0") s.resize(s.size() - 2);
    return s;
}

}  // namespace lms::console
