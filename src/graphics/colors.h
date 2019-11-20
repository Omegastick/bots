#pragma once

#include <glm/vec4.hpp>

namespace SingularityTrainer
{
// Converts hex colours to RGB.
// From: https://stackoverflow.com/questions/3723846/convert-from-hex-color-to-rgb-struct-in-c
constexpr glm::vec4 hex_to_rgb(int hex)
{
    return {static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
            static_cast<float>((hex >> 8) & 0xFF) / 255.0f,
            static_cast<float>((hex)&0xFF) / 255.0f,
            1.f};
}

constexpr glm::vec4 set_alpha(glm::vec4 color, float alpha)
{
    return {color.r, color.g, color.b, alpha};
}

// const glm::vec4 cl_background(0.05, 0.06, 0.06, 1.0);
// const glm::vec4 cl_white(0.95, 0.95, 0.96, 1.0);
// const glm::vec4 cl_accent(0.16, 0.89, 0.47, 1.0);
// const glm::vec4 cl_dark_neutral(0.42, 0.44, 0.34, 1.0);
// const glm::vec4 cl_light_neutral(0.57, 0.67, 0.61, 1.0);

constexpr glm::vec4 cl_base03(hex_to_rgb(0x001e26));
constexpr glm::vec4 cl_base02(hex_to_rgb(0x073642));
constexpr glm::vec4 cl_base01(hex_to_rgb(0x586e75));
constexpr glm::vec4 cl_base00(hex_to_rgb(0x657b83));
constexpr glm::vec4 cl_base0(hex_to_rgb(0x839496));
constexpr glm::vec4 cl_base1_l(hex_to_rgb(0xc6d4d4));
constexpr glm::vec4 cl_base1(hex_to_rgb(0x93a1a1));
constexpr glm::vec4 cl_base2(hex_to_rgb(0xeee8d5));
constexpr glm::vec4 cl_base3(hex_to_rgb(0xfefaf1));
constexpr glm::vec4 cl_yellow(hex_to_rgb(0xb58900));
constexpr glm::vec4 cl_orange(hex_to_rgb(0xcb4b16));
constexpr glm::vec4 cl_red(hex_to_rgb(0xe94361));
constexpr glm::vec4 cl_magenta(hex_to_rgb(0xd33682));
constexpr glm::vec4 cl_violet(hex_to_rgb(0x6c71c4));
constexpr glm::vec4 cl_blue(hex_to_rgb(0x3ca5ee));
constexpr glm::vec4 cl_cyan(hex_to_rgb(0x2aa198));
constexpr glm::vec4 cl_green(hex_to_rgb(0x859900));

constexpr glm::vec4 cl_background = cl_base03;
constexpr glm::vec4 cl_white = cl_base3;
constexpr glm::vec4 cl_accent = cl_red;
constexpr glm::vec4 cl_dark_neutral = cl_base01;
constexpr glm::vec4 cl_light_neutral = cl_base1;
}