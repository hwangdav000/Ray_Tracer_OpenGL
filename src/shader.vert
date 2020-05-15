#version 330 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_color;
layout(location=2) in vec3 in_normal;

out vec3 position;
out vec3 color;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // directly pass the color and normal information to the fragment shader without any modification
	color = in_color;
	normal = in_normal;
    
    // apply ONLY the relevant model transformation to the vertex position value that will be used to calculate the Phong illumination equation in the fragment shader
    position = vec3(model * vec4(in_position,1.0));
    
    // apply the model, view, and projection transformations to the vertex position value that will be sent to the clipper, rasterizer, ...
    gl_Position = projection * view * model * vec4(in_position,1.0);
}
