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
uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D material_texture;
uniform sampler2D z_texture;

out vec4 color;

void main()
{
    vec2 uv_tex = (vec2(posi.x, posi.y)+1)/2;

    vec3 pos = texelFetch(position_texture, ivec2(uv_tex), 0).xyz;//texture(position_texture, (vec2(posi.x, posi.y)+1)/2);
    vec3 l= normalize(-pos);// la camera est la source de lumiere.
    vec4 normal = (texture(normal_texture, uv_tex))*2-1;//texelFetch(normal_texture, ivec2(posi.x, posi.y), 0).xyz;
    vec4 n= normalize(normal);
    float cos_theta= max(0, dot(n.xyz, l));
    vec4 colorm = texture(material_texture, (uv_tex));
	color = colorm*cos_theta;//vec4(0.5-cos_theta,0.5-cos_theta,0.5-cos_theta,1);//*cos_theta;

};

#endif