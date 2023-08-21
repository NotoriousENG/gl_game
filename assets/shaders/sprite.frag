#version 300 es
precision highp float;

in vec2 uv;
in vec4 color;
out vec4 fragColor;

uniform sampler2D texture;

void main(void) {
  // texelFetch gets a pixel by its index in the texture instead of 0-1 spacing
  fragColor = texelFetch(texture, ivec2(uv), 0) * color;
}