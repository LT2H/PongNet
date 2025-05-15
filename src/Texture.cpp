#include <GameCommon/Texture.h>
#include <GameCommon/Common.h>

gm::Texture2D::Texture2D() {}


void gm::Texture2D::generate(unsigned int width, unsigned int height,
                             unsigned char* data)
{
    width_  = width;
    height_ = height;

    // Create Texture
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 internal_format_,
                 width,
                 height,
                 0,
                 image_format_,
                 GL_UNSIGNED_BYTE,
                 data);

    // set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_max_);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void gm::Texture2D::bind() const { glBindTexture(GL_TEXTURE_2D, id_); }