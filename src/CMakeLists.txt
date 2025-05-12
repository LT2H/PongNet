# GLAD
set(GLAD_HEADER_LIST "${Breakout_SOURCE_DIR}/include/glad/glad.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(glad glad.c ${GLAD_HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(glad PUBLIC ../include)

# Use C compiler instead?
target_compile_features(glad PUBLIC cxx_std_20)


# GameCommon
set(GAME_COMMON_HEADER_LIST "${Breakout_SOURCE_DIR}/include")

add_library(GameCommon Game.cpp ${GAME_COMMON_HEADER_LIST})
target_include_directories(GameCommon PUBLIC ../include)
target_compile_features(GameCommon PUBLIC cxx_std_20)
