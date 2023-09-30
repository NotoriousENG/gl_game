#version 300 es
precision highp float;

// Vertex attributes for position and color
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

uniform mat4 projection; // Projection matrix
uniform mat4 view;       // View matrix

// output variables
out vec2 uv;
out vec4 color;

void main(void) {
  // Calculate the final vertex position
  vec4 finalPosition = projection * view * vec4(in_position, 0.0, 1.0);
    
  // Set the gl_Position
  gl_Position = finalPosition;

  // pass through uv and color
  uv = in_uv;
  color = in_color;
}