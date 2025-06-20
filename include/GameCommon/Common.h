#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stb/stb_image.h>

#include <miniaudio.h>

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <map>
#include <unordered_map>
#include <string_view>
#include <fstream>
#include <sstream>
#include <exception>
#include <tuple>
#include <stdexcept>
#include <span>

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;