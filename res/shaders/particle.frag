#version 400 core
in vec2 tex_coords;
in vec4 particle_color;
out vec4 color;

uniform sampler2D sprite;

void main()
{
    color = (texture(sprite, tex_coords) * particle_color);
}