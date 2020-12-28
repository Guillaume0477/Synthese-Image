
#include <cfloat>
#include <cmath>
#include <algorithm>

#include "app_time.h"

#include "vec.h"
#include "color.h"
#include "mat.h"

#include "mesh.h"
#include "wavefront.h"

#include "program.h"
#include "uniforms.h"

#include "orbiter.h"
#include <random>


// cf tuto_storage

namespace glsl 
{
    // type de base alignes sur 4 octets
    template < typename T >
    struct alignas(4) gscalar
    { 
        alignas(4) T x;
        
        gscalar( ) : x(T()) {}
        gscalar( const T& v ) : x(v) {}
        gscalar& operator= ( const T& v ) { x= v; return *this; }
        operator T ( ) { return x; }
    };

    typedef gscalar<float> gfloat;
    typedef gscalar<int> gint;
    typedef gscalar<unsigned int> guint;
    typedef gscalar<bool> gbool;
    
    // vec2, alignes sur 2 * alignement type de base du vecteur
    template < typename T >
    struct alignas(8) gvec2
    {
        alignas(4) T x, y;
        
        gvec2( ) {}
        gvec2( const gvec2<T>& v ) : x(v.x), y(v.y) {}
        gvec2( const ::vec2& v ) : x(v.x), y(v.y) {}
        gvec2& operator= ( const gvec2<T>& v ) { x= v.x; y= v.y; return *this; }
        gvec2& operator= ( const ::vec2& v ) { x= v.x; y= v.y; return *this; }
        operator ::vec2 ( ) { return ::vec2(float(x), float(y)); }
    };
    
    typedef gvec2<float> vec2;
    typedef gvec2<int> ivec2;
    typedef gvec2<unsigned int> uvec2;
    typedef gvec2<int> bvec2;
    
    // vec3, alignes sur 4 * alignement type de base du vecteur
    template < typename T >
    struct alignas(16) gvec3
    {
        alignas(4) T x, y, z;
        
        gvec3( ) {}
        gvec3( const gvec3<T>& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const ::vec3& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Point& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3( const Vector& v ) : x(v.x), y(v.y), z(v.z) {}
        gvec3& operator= ( const gvec3<T>& v ) { x= v.x; y= v.y; z= v.z; return *this; }
        gvec3& operator= ( const ::vec3& v ) { x= v.x; y= v.y; z= v.z; return *this; }
        gvec3& operator= ( const Point& v ) { x= v.x; y= v.y; z= v.z; return *this; }
        gvec3& operator= ( const Vector& v ) { x= v.x; y= v.y; z= v.z; return *this; }
        operator ::vec3 ( ) { return ::vec3(float(x), float(y), float(y)); }
    };
    
    typedef gvec3<float> vec3;
    typedef gvec3<int> ivec3;
    typedef gvec3<unsigned int> uvec3;
    typedef gvec3<int> bvec3;
    
    // vec4, alignes sur 4 * alignement type de base du vecteur
    template < typename T >
    struct alignas(16) gvec4
    {
        alignas(4) T x, y, z, w;
        
        gvec4( ) {}
        gvec4( const gvec4<T>& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        gvec4( const ::vec4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        gvec4& operator= ( const gvec4<T>& v ) { x(v.x), y(v.y), z(v.z), w(v.w) ; return *this; }
        gvec4& operator= ( const ::vec4& v ) { x(v.x), y(v.y), z(v.z), w(v.w) ; return *this; }
        gvec4& operator= ( const Color& c ) { x= c.r; y= c.g; z= c.b; w= c.a; return *this; }
        operator ::vec4 ( ) { return ::vec4(float(x), float(y), float(y), float(w)); }
    };
    
    typedef gvec4<float> vec4;
    typedef gvec4<int> ivec4;
    typedef gvec4<unsigned int> uvec4;
    typedef gvec4<int> bvec4;
}

struct triangle 
{
    vec3 a;
    int pada;
    vec3 ab;
    int padb;
    vec3 ac;
    int id;
    // int pad1;
    // int pad2;
    // int pad3;
};

// struct BBox
// {
//     glsl::vec3 pmin;
//     glsl::vec3 pmax;
// };

// struct Node
// {
//     BBox bounds;
//     int left;
//     int right;
// };

// int build( const BBox& _bounds, const std::vector<Triangle>& _triangles )
// {
//     triangles= _triangles;  // copie les triangles pour les trier
//     nodes.clear();          // efface les noeuds
//     nodes.reserve(triangles.size());
    
//     // construit l'arbre... 
//     root= build(_bounds, 0, triangles.size());
//     // et renvoie la racine
//     return root;
// }

// int build( const BBox& bounds, const int begin, const int end )
// {
//     if(end - begin < 2)
//     {
//         // inserer une feuille et renvoyer son indice
//         int index= nodes.size();
//         nodes.push_back(make_leaf(bounds, begin, end));
//         return index;
//     }
    
//     // axe le plus etire de l'englobant
//     Vector d= Vector(bounds.pmin, bounds.pmax);
//     int axis;
//     if(d.x > d.y && d.x > d.z)  // x plus grand que y et z ?
//         axis= 0;
//     else if(d.y > d.z)          // y plus grand que z ? (et que x implicitement)
//         axis= 1;
//     else                        // x et y ne sont pas les plus grands...
//         axis= 2;

//     // coupe l'englobant au milieu
//     float cut= bounds.centroid(axis);
    
//     // repartit les triangles 
//     Triangle *pm= std::partition(triangles.data() + begin, triangles.data() + end, triangle_less1(axis, cut));
//     int m= std::distance(triangles.data(), pm);
    
//     // la repartition des triangles peut echouer, et tous les triangles sont dans la meme partie... 
//     // forcer quand meme un decoupage en 2 ensembles 
//     if(m == begin || m == end)
//         m= (begin + end) / 2;
//     assert(m != begin);
//     assert(m != end);
    
//     // construire le fils gauche
//     // les triangles se trouvent dans [begin .. m)
//     BBox bounds_left= triangle_bounds(begin, m);
//     int left= build(bounds_left, begin, m);
    
//     // on recommence pour le fils droit
//     // les triangles se trouvent dans [m .. end)
//     BBox bounds_right= triangle_bounds(m, end);
//     int right= build(bounds_right, m, end);
    
//     int index= nodes.size();
//     nodes.push_back(make_node(bounds, left, right));
//     return index;
// }


struct RT : public AppTime
{
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    RT( const char *filename ) : AppTime(1024, 640) 
    {
        m_mesh= read_mesh(filename);
    }
    
    int init( )
    {
        if(m_mesh.triangle_count() == 0)
            return -1;
        
        Point pmin, pmax;
        m_mesh.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);
        
        // utilise un compute shader qui :
        // 1. cree un rayon pour chaque pixel de l'image
        // 2. calcule les intersections du rayon avec tous les triangles. (et garde la plus proche...)
        // 3. ecrit le resultat dans une image.
        
        // il faut donc creer le buffer et la texture, les entrees / sorties du shader
        
        // recupere les triangles du mesh
        // structure declaree par le shader, en respectant l'alignement std430



        
        std::vector<triangle> data;
        data.reserve(m_mesh.triangle_count());
        for(int i= 0; i < m_mesh.triangle_count(); i++)
        {
            TriangleData t= m_mesh.triangle(i);
            data.push_back( { Point(t.a),1, Point(t.b) - Point(t.a),1, Point(t.c) - Point(t.a),i } );
        }


        // std::vector<Node> nodes;
        // data.reserve(m_mesh.triangle_count());
        // for(int i= 0; i < m_mesh.triangle_count(); i++)
        // {
        //     TriangleData t= m_mesh.triangle(i);
        //     data.push_back( { Point(t.a), Point(t.b) - Point(t.a), Point(t.c) - Point(t.a) } );
        // }
        
        // cree et initialise le storage buffer
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(triangle), data.data(), GL_STATIC_READ);
        
        // texture / image resultat
        // cree la texture, 4 canaux, entiers 8bits normalises, standard
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA8, window_width(), window_height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        
        // pas la peine de construire les mipmaps, le shader ne va ecrire que le mipmap 0
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        // oui, c'est une texture tout a fait normale.




        int wh = window_height()*window_width();
        std::random_device seed_generator;
        std::vector<unsigned int> seeds(wh);
        for (int i = 0; i< wh; i++ ){
            seeds[i] = seed_generator();
        }



        // texture / image resultat
        // cree la texture, 4 canaux, entiers 8bits normalises, standard
        glGenTextures(1, &m_seed_image);
        glBindTexture(GL_TEXTURE_2D, m_seed_image);
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_R32UI, window_width(), window_height(), 0,
            GL_RED_INTEGER, GL_UNSIGNED_INT, seeds.data());
        
        // pas la peine de construire les mipmaps, le shader ne va ecrire que le mipmap 0
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        // oui, c'est une texture tout a fait normale.

        // pour afficher l'image resultat, 2 solutions : 
        //      1. utiliser un shader qui copie un pixel de la texture vers un pixel du framebuffer par defaut / la fenetre, 
        //      2. ou copier directement la texture sur le framebuffer par defaut / la fenetre, en utilisant glBlitFramebuffer
        
        // pour changer, on va utiliser glBlitFramebuffer, 
        // mais il faut configurer un framebuffer...
        glGenFramebuffers(1, &m_blit_framebuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blit_framebuffer);
        glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);
        // selectionner la texture du framebuffer a copier...
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        
        // compute shader
        m_program= read_program("tutos/M2/raytrace_compute.glsl");
        program_print_errors(m_program);

        frame=0;
        
        // nettoyage
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 0;
    }
    
    int quit( )
    {
        release_program(m_program);
        glDeleteTextures(1, &m_texture);
        glDeleteBuffers(1, &m_buffer);
        glDeleteFramebuffers(1, &m_blit_framebuffer);
        return 0;
    }
    
    int render( )
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT);
        
        if(key_state('f'))
        {
            clear_key_state('f');
            // recentrer la camera
            Point pmin, pmax;
            m_mesh.bounds(pmin, pmax);
            m_camera.lookat(pmin, pmax);        
        }
        
        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1)){           // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
            frame=0;
        } 
        else if(mb & SDL_BUTTON(3)){         // le bouton droit est enfonce
            m_camera.move(mx); 
            frame=0;
        } 
        else if(mb & SDL_BUTTON(2)){     // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());
            frame=0;
        }    
        
        
        // recupere les transformations standards.
        Transform m= Identity();
        Transform v= m_camera.view();
        Transform p= m_camera.projection(window_width(), window_height(), 45);
        Transform im= Viewport(window_width(), window_height());
        // ou Transform im= m_camera.viewport();
        // compose toutes les transformations, jusqu'au repere image
        Transform T= im * p * v * m;
        
        // config pipeline
        glUseProgram(m_program);
        
        // storage buffer 0
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer);
        
        // image texture 0, ecriture seule, mipmap 0 + format rgba8 classique
        glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        // configurer le shader
        program_uniform(m_program, "image", 0);

        glBindImageTexture(1, m_seed_image, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        // configurer le shader
        program_uniform(m_program, "seeds", 1);
        
        // uniforms
        program_uniform(m_program, "invMatrix", T.inverse());

        program_uniform(m_program, "frame", frame);
        //glUniform1i( location(m_program, "frame"), frame)
        frame++;
        
        // nombre de groupes de shaders pour executer un compute shader par pixel de l'image resultat. on utilise un domaine 2d...
        // le shader declare un groupe de threads de 8x8.
        int nx= window_width() / 8;
        int ny= window_height() / 8;
        // on suppose que les dimensions de l'image sont multiples de 8...
        // sinon calculer correctement le nombre de groupes pour x et y. 
        
        // go !!
        glDispatchCompute(nx, ny, 1);
        
        // attendre le resultat
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        
        // afficher le resultat
        // copier la texture resultat vers le framebuffer par defaut / de la fenetre
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blit_framebuffer);
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0,0, window_width(),window_height(), 
            0,0, window_width(),window_height(),
             GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
        return 1;
    }
    
protected:
    Mesh m_mesh;
    Orbiter m_camera;

    GLuint m_blit_framebuffer;
    GLuint m_program;
    GLuint m_texture;
    GLuint m_seed_image;
    GLuint m_buffer;

    int frame;
};

    
int main( int argc, char **argv )
{
    const char *filename= "data/cornell.obj";
    if(argc > 1)
        filename= argv[1];
    
    RT app(filename);
    app.run();
    
    return 0;
}
