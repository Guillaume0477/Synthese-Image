//! \file tuto9_materials.glsl

#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position1;
layout(location= 1) in vec3 position2;
layout(location= 2) in vec2 texcoord1;
layout(location= 3) in vec2 texcoord2;
layout(location= 4) in vec3 normal1;
layout(location= 5) in vec3 normal2;
layout(location= 6) in uint material;

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
    vec4 position3 = p1*(1-temps)+p2*temps;
    vec4 position4 = projection*position3;
    gl_Position=position4;// mvpMatrix * vec4(position, 1); //ok
    //gl_Position = mvpMatrix * vec4(position1, 1);
    
    // position et normale dans le repere camera
    vertex_position= vec3(position3); //ok
    //vertex_position= vec3(mvMatrix * vec4(position1, 1));
    vertex_texcoord= texcoord1;//*(1-temps)+temps*texcoord2; // pas ok

    vec3 normal=normal1*(1-temps)+temps*normal2; //ok
    vertex_normal= mat3(mvMatrix) * normal;
    // ... comme d'habitude
    
    // et transmet aussi l'indice de la matiere au fragment shader...
    vertex_material= material;
}

#endif


#ifdef FRAGMENT_SHADER

layout(location= 0) out vec4 buffer_color;
layout(location= 1) out vec4 buffer_position;
layout(location= 2) out vec4 buffer_normal;
layout(location= 3) out vec4 buffer_material;


in vec3 vertex_position;
in vec2 vertex_texcoord;
in vec3 vertex_normal;
flat in uint vertex_material;	// !! decoration flat, le varying est marque explicitement comme non interpolable  !!

#define MAX_MATERIALS 16
uniform vec4 materials[MAX_MATERIALS];
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D color_texture;


void main( )
{
    vec3 l= normalize(-vertex_position);        // la camera est la source de lumiere.
    vec3 n= normalize(vertex_normal);
    float cos_theta= max(0, dot(n, l));
    
    // recupere la couleur de la matiere du triangle, en fonction de son indice.
    int int_vertex_material = int(vertex_material);
    vec4 colorm= materials[vertex_material];
    if ( 1 == int_vertex_material){
        colorm = colorm * vec4(0.486, 0.988, 0, 1);
    } 
    //vec4 color0= texture(texture0, vertex_texcoord);
    //vec4 color1= texture(texture1, vertex_texcoord);
    
    //color0= color0 * 0.5 + texture(texture1, vertex_texcoord) * 0.5;
    
    //fragment_color= vec4(1, 0.5, 0, 1);
    //fragment_color= color0 * colorm * cos_theta;
    buffer_color = colorm * cos_theta;//* vec4(0.486, 0.988, 0, 1);
    buffer_position = vec4(vertex_position,1);
    buffer_normal = vec4(vertex_normal,1);
    buffer_material = colorm;


    // vec4 color= texture(color_texture, vertex_texcoord);
    // fragment_color= cos_theta * color;


}

#endif
