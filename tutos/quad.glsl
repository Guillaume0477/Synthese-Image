#version 330 core



#ifdef VERTEX_SHADER
layout (location = 0) in vec3 aPos;

out vec3 posi;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    posi = vec3(aPos.x, aPos.y, aPos.z);

};

#endif

#ifdef FRAGMENT_SHADER
in vec3 posi;

uniform sampler2D color_texture;

out vec4 color;

void main()
{
	color = texture(color_texture, (vec2(posi.x, posi.y)+1)/2);
};

#endif