add_executable(App App.cpp)

target_compile_features(App PRIVATE cxx_std_20)

target_link_libraries(App
    PRIVATE
        glad
        glfw
        glm
        stb
        miniaudio
        GameCommon
        freetype
        NetCommon
)

# if using Windows APIs directly
if(WIN32)
    target_link_libraries(App PRIVATE
        opengl32
    )
endif()