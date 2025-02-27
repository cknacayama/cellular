#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <format>
#include <iostream>
#include <iterator>
#include <ostream>
#include <utility>

template <typename... _Args>
inline void
print(std::ostream &os, std::format_string<_Args...> __fmt, _Args &&...__args) {
    std::ostream_iterator<char> out(os);
    std::vformat_to(out, __fmt.get(), std::make_format_args(__args...));
}

template <typename... _Args>
inline void println(
    std::ostream &os, std::format_string<_Args...> __fmt, _Args &&...__args
) {
    print(os, __fmt, std::forward<_Args>(__args)...);
    os << '\n';
}

template <typename... _Args>
inline void print(std::format_string<_Args...> __fmt, _Args &&...__args) {
    print(std::cout, __fmt, std::forward<_Args>(__args)...);
}

template <typename... _Args>
inline void println(std::format_string<_Args...> __fmt, _Args &&...__args) {
    println(std::cout, __fmt, std::forward<_Args>(__args)...);
}

template <typename... _Args>
inline void eprint(std::format_string<_Args...> __fmt, _Args &&...__args) {
    print(std::cerr, __fmt, std::forward<_Args>(__args)...);
}

template <typename... _Args>
inline void eprintln(std::format_string<_Args...> __fmt, _Args &&...__args) {
    println(std::cerr, __fmt, std::forward<_Args>(__args)...);
}

template <typename... _Args>
[[noreturn]] inline void
panic(std::format_string<_Args...> __fmt, _Args &&...__args) {
    println(std::cerr, __fmt, std::forward<_Args>(__args)...);
    std::terminate();
}

template <typename T>
inline void debug(
    std::string_view file_name,
    std::size_t      line,
    std::string_view func,
    std::string_view expr,
    T                arg
) {
    eprint("[{}:{}:{}] {} = ", file_name, func, line, expr);
    std::cerr << arg << '\n';
}

#define DEBUG(arg) debug(__FILE__, __LINE__, __func__, #arg, arg)

#endif
