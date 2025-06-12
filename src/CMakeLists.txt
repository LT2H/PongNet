# GLAD
set(GLAD_HEADER_LIST "${Breakout_SOURCE_DIR}/include/glad/glad.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(glad glad/glad.c ${GLAD_HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(glad PUBLIC ../include)

# Use C compiler instead?
target_compile_features(glad PUBLIC cxx_std_20)

# stb
set(STB_HEADER_LIST "${Breakout_SOURCE_DIR}/include/stb/stb_image.h")
add_library(stb stb/stb_image.cpp ${STB_HEADER_LIST})
target_include_directories(stb PUBLIC ../include)
target_compile_features(stb PUBLIC cxx_std_20)

# GameCommon
add_library(GameCommon GameCommon/Game.cpp GameCommon/ResourceManager.cpp GameCommon/Shader.cpp GameCommon/Texture.cpp GameCommon/SpriteRenderer.cpp GameCommon/GameObject.cpp GameCommon/GameLevel.cpp GameCommon/BallObject.cpp GameCommon/ParticleGenerator.cpp GameCommon/PostProcessor.cpp)
target_include_directories(GameCommon PUBLIC ../include)

# This depends on glm, stb, glad, glfw
target_link_libraries(GameCommon PRIVATE glm stb glad glfw)

target_compile_features(GameCommon PUBLIC cxx_std_20)
