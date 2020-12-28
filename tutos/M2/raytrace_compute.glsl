
#version 430

#ifdef COMPUTE_SHADER

struct Triangle
{
    vec3 a;		// sommet
    vec3 ab;	// arete 1
    vec3 ac;	// arete 2
    int id;
};

vec3 global( const in vec3 n) { 
    
    float sign= n.z < 0 ? -1.0f : 1.0f;
    float a= -1.0f / (sign + n.z);
    float d= n.x * n.y * a;
    vec3 t= vec3(1.0f + sign * n.x * n.x * a, sign * d, -sign * n.x);
    vec3 b= vec3(d, sign + n.y * n.y * a, -n.y);
    return  vec3(n.x * t + n.y * b + n.z * n); 
}

/*
struct World
{
    World( const vec3 _n ) : n(_n){
        float sign= std::copysign(1.0f, n.z);
        float a= -1.0f / (sign + n.z);
        float d= n.x * n.y * a;
        t= Vector(1.0f + sign * n.x * n.x * a, sign * d, -sign * n.x);
        b= Vector(d, sign + n.y * n.y * a, -n.y);
    }

    // transforme le vecteur du repere local vers le repere du monde
    Vector operator( ) ( const Vector& local )  const { return local.x * t + local.y * b + local.z * n; }

    // transforme le vecteur du repere du monde vers le repere local
    Vector inverse( const Vector& global ) const { return Vector(dot(global, t), dot(global, b), dot(global, n)); }

    Vector t;
    Vector b;
    Vector n;
};

*/

const uint rng_a= 1103515245;
const uint rng_b= 12345;
const uint rng_mask= (1u << 31) -1u;
const float rng_m= 1u << 31;

// renvoie un reel aleatoire dans [0 1]
float rng( inout uint state )
{
    state= (rng_a * state + rng_b) % uint(rng_m);
    return float(state) / rng_m;
}

#define M_PI 3.1415926535897932384626433832795


// genere une direction sur l'hemisphere,
// cf GI compendium, eq 35
vec3 sample35(const float u1, const float u2)
{
    // coordonnees theta, phi
    float cos_theta = sqrt(u1);
    float phi = float(2 * M_PI) * u2;

    // passage vers x, y, z
    float sin_theta = sqrt(1 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// evalue la densite de proba, la pdf de la direction, cf GI compendium, eq 35
float pdf35(const vec3 w)
{
    if (w.z < 0)
        return 0;
    return w.z / float(M_PI);
}


// shader storage buffer 0
layout(std430, binding= 0) readonly buffer triangleData
{
    Triangle triangles[];
};


bool intersect( const Triangle triangle, const vec3 o, const vec3 d, const float tmax, out float rt, out float ru, out float rv )
{
    vec3 pvec= cross(d, triangle.ac);
    float det= dot(triangle.ab, pvec);
    float inv_det= 1.0f / det;
    
    vec3 tvec= o - triangle.a;
    float u= dot(tvec, pvec) * inv_det;
    vec3 qvec= cross(tvec, triangle.ab);
    float v= dot(d, qvec) * inv_det;
    
    /* calculate t, ray intersects triangle */
    rt= dot(triangle.ac, qvec) * inv_det;
    ru= u;
    rv= v;
    
    // ne renvoie vrai que si l'intersection est valide : 
    // interieur du triangle, 0 < u < 1, 0 < v < 1, 0 < u+v < 1
    if(any(greaterThan(vec3(u, v, u+v), vec3(1, 1, 1))) || any(lessThan(vec2(u, v), vec2(0, 0))))
        return false;
    // comprise entre 0 et tmax du rayon
    return (rt < tmax && rt > 0);
}

uniform mat4 invMatrix;
uniform int frame;

// image resultat
layout(binding= 0, rgba8)  coherent uniform image2D image;
layout(binding= 1, r32ui)  coherent uniform uimage2D seeds;


// 8x8 threads
layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    // recupere le threadID 2d, et l'utilise directement comme coordonnees de pixel
    vec2 position= vec2(gl_GlobalInvocationID.xy);
    
    // construction du rayon pour le pixel, passage depuis le repere image vers le repere monde
    vec4 oh= invMatrix * vec4(position, 0, 1);       // origine sur near
    vec4 eh= invMatrix * vec4(position, 1, 1);       // extremite sur far

    // origine et direction
    vec3 o= oh.xyz / oh.w;                              // origine
    vec3 d= eh.xyz / eh.w - oh.xyz / oh.w;              // direction

    float hit= 1.0;	// tmax = far, une intersection valide est plus proche que l'extremite du rayon / far...
    float hitu= 0.0;
    float hitv= 0.0;
    int id;
    for(int i= 0; i < triangles.length(); i++)
    {
        float t, u, v;
        if(intersect(triangles[i], o, d, hit, t, u, v))
        {
            hit= t;
            hitu=u;//1-t;//
            hitv=v; //1-t;//
            id=i;
        }
    }
    
    float w = 1.0 - hitu - hitv;
    vec3 p = triangles[id].a + hitu*triangles[id].ab + hitv*triangles[id].ac;
    vec3 n_p = normalize(cross(triangles[id].ab,triangles[id].ac));

    int N_ray = 8;
    vec4 ambient = vec4(0.0);
    //uint state = 0;

    uint state = imageLoad(seeds, ivec2(gl_GlobalInvocationID.xy)).x;

    
    for (int ni = 0; ni<N_ray; ni++){


 
        uint k = uint(frame*N_ray+state);

        //vec3 d_l = normalize(vec3(0.0,0.0,1.98)-p);///
        //vec3 d_l = vec3(0.0,1.0,0.0);
        float u1 = rng(k);
        float u2 = rng(k);
        
        vec3 d_l = sample35(u1,u2);
        float pdf = pdf35(d_l);



        // if(dot(d_l, n_p) > 0){
        //     n_p= -n_p;
        // }

        float sign= n_p.z < 0 ? -1.0f : 1.0f;
        float a= -1.0f / (sign + n_p.z);
        float e= n_p.x * n_p.y * a;
        vec3 t= vec3(1.0f + sign * n_p.x * n_p.x * a, sign * e, -sign * n_p.x);
        vec3 b= vec3(e, sign + n_p.y * n_p.y * a, -n_p.y);
        d_l = d_l.x * t + d_l.y * b + d_l.z * n_p; 
        
        
        //d_l = global(d_l);
        int v = 0;
        float vu_sun = 1;
        


        float hit2= 1.0;	// tmax = far, une intersection valide est plus proche que l'extremite du rayon / far...
        float hitu2= 0.0;
        float hitv2= 0.0;
        int id2;
        for(int j= 0; j < triangles.length(); j++)
        {
            float t2, u2, v2;
            if(intersect(triangles[j], p+0.01*n_p, d_l, 1000, t2, u2, v2))
            //if(intersect(triangles[j], p, d_l, 1000000, t2, u2, v2))
            {
                hit2= t2;
                hitu2=u2;//1-t;//
                hitv2=v2; //1-t;//
                id2=j;
                v = 0;
                vu_sun = 0;
                break;
                
            }
        }

        if (vu_sun==1){
            //vec4 diffuse = vec4(0,0.140,0.450,0.0911);
            vec4 diffuse = vec4(1.0);
            float cos_theta = max(float(0.0), dot(n_p, d_l));
            //float cos_theta = abs(dot(n_p, d_l));
            ambient = ambient + diffuse*cos_theta / (float(M_PI)*pdf);// * (cos_theta / (float(M_PI) * pdf35 * N_ray));
        }

    }
    // vec4 Color = vec4(0, 0, 0, 1);

    // if (hit < 1.0 && hit > 0){ //if dans mesh
    //     //Color = vec4(hitu, hitv, 0, 1)*v;
    //     Color += vec4(1-hit, 1-hit, 1-hit, 1)*(1-v);
    //     Color += vec4(1-hit, 1-hit, 0, 1)*v;
    // }
    // else {
    //     Color = vec4(0, 0, 1, 1);
    // }
    vec4 Color = vec4(0.0);
    if (hit < 1.0 && hit > 0){ //if dans mesh
        Color = Color + ambient;//vec4(0, 0, 0, 1);
    }
    vec4 curr_im = imageLoad(image, ivec2(gl_GlobalInvocationID.xy));
    Color = (curr_im *frame*N_ray + Color) / (frame *N_ray+N_ray);
    // ecrire le resultat dans l'image
    imageStore(image, ivec2(gl_GlobalInvocationID.xy), Color );
}
#endif
