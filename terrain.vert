#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 MVP;
uniform float u_max;
float random (vec2 st) {
	return fract(sin(dot(st.xy,
						 vec2(12.9898,78.233)))*
		43758.5453123);
}
void main(){
	 //float max = 0.01;
	 float zed = mod(random(position.xy),u_max)-(u_max/2.0);
	 vec4 newPos = position.xzyw;
	 newPos.y = zed;
	 //gl_Position = MVP * position.xzyw;
	 gl_Position = MVP * newPos;
}