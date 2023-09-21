#version 300 es
precision highp float;

in vec2 uv;
in vec4 color;
out vec4 fragColor;

uniform sampler2D albedoTexture;

void main(void) { fragColor = texture(albedoTexture, uv) * color; }
