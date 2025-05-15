#pragma once

#include "Common.h"
#include "GameCommon/Common.h"
#include <vector>

namespace gm
{
// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D
{
  public:
    Texture2D();

    void generate(unsigned int width, unsigned int height,
      unsigned char* data);

    void bind() const;

    unsigned int id() const { return id_; }

    void set_internal_format(unsigned int internal_format)
    {
        internal_format_ = internal_format;
    }

    void set_image_format(unsigned int image_format)
    {
        image_format_ = image_format;
    }

  private:
    // Holds the ID of the texture object, used for all texture operations to
    // reference to this particular texture
    unsigned int id_{};

    // Texture image dimensions, width and height of loaded image in pixels
    unsigned int width_{ 0 };
    unsigned int height_{ 0 };

    // Texture Format
    unsigned int internal_format_{ GL_RGB }; // format of texture object
    unsigned int image_format_{ GL_RGB };    // format of loaded image

    // Texture configuration
    unsigned int wrap_s_{ GL_REPEAT }; // wrapping mode on S axis
    unsigned int wrap_t_{ GL_REPEAT }; // wrapping mode on T axis
    unsigned int filter_min_{
        GL_LINEAR
    }; // filtering mode if texture pixels < screen pixels
    unsigned int filter_max_{
        GL_LINEAR
    }; // filtering mode if texture pixels > screen pixels
};
} // namespace gm