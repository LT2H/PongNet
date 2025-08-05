#pragma once

#include "Common.h"
#include "GameCommon/Common.h"

namespace gcom
{
// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D
{
  public:
    explicit Texture2D();

    void generate(u32 width, u32 height,
      unsigned char* data);

    void bind() const;

    u32 id() const { return id_; }

    void set_internal_format(u32 internal_format)
    {
        internal_format_ = internal_format;
    }

    void set_image_format(u32 image_format)
    {
        image_format_ = image_format;
    }

  private:
    // Holds the ID of the texture object, used for all texture operations to
    // reference to this particular texture
    u32 id_{};

    // Texture image dimensions, width and height of loaded image in pixels
    u32 width_{ 0 };
    u32 height_{ 0 };

    // Texture Format
    u32 internal_format_{ GL_RGB }; // format of texture object
    u32 image_format_{ GL_RGB };    // format of loaded image

    // Texture configuration
    u32 wrap_s_{ GL_REPEAT }; // wrapping mode on S axis
    u32 wrap_t_{ GL_REPEAT }; // wrapping mode on T axis
    u32 filter_min_{
        GL_LINEAR
    }; // filtering mode if texture pixels < screen pixels
    u32 filter_max_{
        GL_LINEAR
    }; // filtering mode if texture pixels > screen pixels
};
} // namespace gcom