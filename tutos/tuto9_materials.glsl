//! \file tuto9_materials.glsl

#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position1;
layout(location= 1) in vec3 position2;
layout(location= 2) in vec2 texcoord;
layout(location= 3) in vec3 normal1;
layout(location= 4) in vec3 normal2;
layout(location= 5) in uint material;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

uniform float temps;

out vec3 vertex_position;
out vec2 vertex_texcoord;
out vec3 vertex_normal;

flat out uint vertex_material;
/* decoration flat : le varying est un entier, donc pas vraiment interpolable... il faut le declarer explicitement */

void main( )
{
    vec4 p1=mvMatrix * vec4(position1, 1);
    vec4 p2=mvMatrix * vec4(position2, 1);
    vec4 position3 = p1*(1-sin(temps))+p2*sin(temps);
    vec4 position4 = projection*position3;
    gl_Position=position4;// mvpMatrix * vec4(position, 1);
    //gl_Position = mvpMatrix * vec4(position1, 1);
    
    // position et normale dans le repere camera
    vertex_position= vec3(position3);
    //vertex_position= vec3(mvMatrix * vec4(position1, 1));
    //vertex_texcoord= texcoord;

    vec3 normal=normal1*(1-cos(temps))+normal2;
    vertex_normal= mat3(mvMatrix) * normal1;
    // ... comme d'habitude
    
    // et transmet aussi l'indice de la matiere au fragment shader...
    vertex_material= material;
}

#endif


#ifdef FRAGMENT_SHADER
out vec4 fragment_color;

in vec3 vertex_position;
in vec2 vertex_texcoord;
in vec3 vertex_normal;
flat in uint vertex_material;	// !! decoration flat, le varying est marque explicitement comme non interpolable  !!

#define MAX_MATERIALS 16
uniform vec4 materials[MAX_MATERIALS];
uniform sampler2D texture0;
uniform sampler2D texture1;


void main( )
{
    vec3 l= normalize(-vertex_position);        // la camera est la source de lumiere.
    vec3 n= normalize(vertex_normal);
    float cos_theta= max(0, dot(n, l));
    
    // recupere la couleur de la matiere du triangle, en fonction de son indice.
    vec4 colorm= materials[vertex_material];
    //vec4 color0= texture(texture0, vertex_texcoord);
    //vec4 color1= texture(texture1, vertex_texcoord);
    //fragment_color= vec4(1, 0.5, 0, 1);
    //fragment_color= color0 * color1 * colorm * cos_theta;
    fragment_color= colorm * cos_theta;
}

#endif
