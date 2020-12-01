#version 400 core

layout (location = 0) in vec4 position;
uniform mat4 MVP;
uniform vec3 lightPosition;

void main(){
  gl_Position = MVP*(vec4(lightPosition,1) + position);
  //gl_Position = MVP*position;
}
