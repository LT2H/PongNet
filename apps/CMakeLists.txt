add_executable(App App.cpp)

target_compile_features(App PRIVATE cxx_std_20)
target_link_libraries(App
    PRIVATE
        glad
        glfw
        GameCommon
)

# Optional if using Windows APIs directly (likely if you're using GLFW)
if(WIN32)
    target_link_libraries(App PRIVATE
        opengl32
    )
endif()