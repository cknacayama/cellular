#ifndef CELLULAR_ALIAS_H
#define CELLULAR_ALIAS_H

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdint>

namespace cell {

using f64   = double;
using f32   = float;
using i8    = std::int8_t;
using u8    = std::uint8_t;
using i16   = std::int16_t;
using u16   = std::uint16_t;
using i32   = std::int32_t;
using u32   = std::uint32_t;
using i64   = std::int64_t;
using u64   = std::uint64_t;
using isize = std::ptrdiff_t;
using usize = std::size_t;

} // namespace cell

#endif
