#ifndef UTIL_HPP
#define UTIL_HPP

#include <exception>
#include <format>
#include <iostream>
#include <print>
#include <utility>

template <typename... Args>
inline void eprint(std::format_string<Args...> _fmt, Args &&..._args) {
    std::print(std::cerr, _fmt, std::forward<Args>(_args)...);
}

template <typename... Args>
inline void eprintln(std::format_string<Args...> _fmt, Args &&..._args) {
    std::println(std::cerr, _fmt, std::forward<Args>(_args)...);
}

template <typename... Args>
[[noreturn]] inline void
panic(std::format_string<Args...> _fmt, Args &&..._args) {
    println(std::cerr, _fmt, std::forward<Args>(_args)...);
    std::terminate();
}

template <std::formattable<char> T>
inline void debug(
    std::string_view file_name,
    std::size_t      line,
    std::string_view func,
    std::string_view expr,
    T const         &arg
) {
    eprintln("[{}:{}:{}] {} = {}", file_name, func, line, expr, arg);
}

#define DEBUG(arg) debug(__FILE__, __LINE__, __func__, #arg, arg)

#endif
