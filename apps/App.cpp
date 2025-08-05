#include "GameCommon/Game.h"

#include <GameCommon/Shader.h>
#include <GameCommon/Texture.h>
#include <GameCommon/ResourceManager.h>

#include <GameCommon/Common.h>
#include <iostream>

// void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// settings
constexpr u32 SCR_WIDTH{ 800 };
constexpr u32 SCR_HEIGHT{ 600 };

int main()
{
    
    gcom::Game game{SCR_WIDTH, SCR_HEIGHT};
    if(game.init())
    {
        game.run();
    }

    return 0;
}

// // process all input: query GLFW whether relevant keys are pressed/released this
// // frame and react accordingly
// // ---------------------------------------------------------------------------------------------------------
// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
// {
//     // when a user presses the escape key, we set the WindowShouldClose property to
//     // true, closing the application
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, true);
//     if (key >= 0 && key < 1024)
//     {
//         if (action == GLFW_PRESS)
//             game.keys_[key] = true;
//         else if (action == GLFW_RELEASE)
//         {
//             game.keys_[key] = false;
//             game.keys_processed_[key] = false;
//         }
//     }
// }

// // glfw: whenever the window size changed (by OS or user resize) this callback
// // function executes
// // ---------------------------------------------------------------------------------------------
// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
//     // make sure the viewport matches the new window dimensions; note that width and
//     // height will be significantly larger than specified on retina displays.
//     glViewport(0, 0, width, height);
// }