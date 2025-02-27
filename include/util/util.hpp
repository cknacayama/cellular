#ifndef UTIL_HPP
#define UTIL_HPP

#include <format>
#include <iostream>
#include <iterator>
#include <ostream>
#include <utility>

template <typename... Args>
inline void
print(std::ostream &os, std::format_string<Args...> _fmt, Args &&..._args) {
    std::ostream_iterator<char> out(os);
    std::vformat_to(
        out, _fmt.get(), std::make_format_args(std::forward<Args>(_args)...)
    );
}

template <typename... Args>
inline void
println(std::ostream &os, std::format_string<Args...> _fmt, Args &&..._args) {
    print(os, _fmt, std::forward<Args>(_args)...);
    os << '\n';
}

template <typename... Args>
inline void print(std::format_string<Args...> _fmt, Args &&..._args) {
    print(std::cout, _fmt, std::forward<Args>(_args)...);
}

template <typename... Args>
inline void println(std::format_string<Args...> _fmt, Args &&..._args) {
    println(std::cout, _fmt, std::forward<Args>(_args)...);
}

template <typename... Args>
inline void eprint(std::format_string<Args...> _fmt, Args &&..._args) {
    print(std::cerr, _fmt, std::forward<Args>(_args)...);
}

template <typename... Args>
inline void eprintln(std::format_string<Args...> _fmt, Args &&..._args) {
    println(std::cerr, _fmt, std::forward<Args>(_args)...);
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
