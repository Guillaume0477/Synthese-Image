
#include <cfloat>
#include <cstdio>
#include <cassert>
#include <random>
#include <chrono>

#include "vec.h"
#include "mesh.h"
#include "wavefront.h"
#include "orbiter.h"

#include "image.h"
#include "image_io.h"
#include "image_hdr.h"


struct Ray
{
    Point o;
    float pad;
    Vector d;
    float tmax;
    
    Ray( ) : o(), d(), tmax(0) {}
    Ray( const Point& _o, const Point& _e ) : o(_o), d(Vector(_o, _e)), tmax(1) {}
    Ray( const Point& _o, const Vector& _d ) : o(_o), d(_d), tmax(FLT_MAX) {}
};


// intersection rayon / triangle.
struct Hit
{
    int triangle_id;
    float t;
    float u, v;
    
    Hit( ) : triangle_id(-1), t(0), u(0), v(0) {}       // pas d'intersection
    Hit( const int _id, const float _t, const float _u, const float _v ) : triangle_id(_id), t(_t), u(_u), v(_v) {}
    
    operator bool( ) const { return (triangle_id != -1); }      // renvoie vrai si l'intersection est initialisee...
};

// renvoie la normale interpolee d'un triangle.
Vector normal( const Hit& hit, const TriangleData& triangle )
{
    return normalize((1 - hit.u - hit.v) * Vector(triangle.na) + hit.u * Vector(triangle.nb) + hit.v * Vector(triangle.nc));
}

// renvoie le point d'intersection sur le triangle.
Point point( const Hit& hit, const TriangleData& triangle )
{
    return (1 - hit.u - hit.v) * Point(triangle.a) + hit.u * Point(triangle.b) + hit.v * Point(triangle.c);
}

// renvoie le point d'intersection sur le rayon
Point point( const Hit& hit, const Ray& ray )
{
    return ray.o + hit.t * ray.d;
}


// triangle "intersectable".
struct Triangle
{
    Point p;
    Vector e1, e2;
    int id;
    
    Triangle( const Point& _a, const Point& _b, const Point& _c, const int _id ) : p(_a), e1(Vector(_a, _b)), e2(Vector(_a, _c)), id(_id) {}
    
    /* calcule l'intersection ray/triangle
        cf "fast, minimum storage ray-triangle intersection" 
        http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
        
        renvoie faux s'il n'y a pas d'intersection valide (une intersection peut exister mais peut ne pas se trouver dans l'intervalle [0 htmax] du rayon.)
        renvoie vrai + les coordonnees barycentriques (u, v) du point d'intersection + sa position le long du rayon (t).
        convention barycentrique : p(u, v)= (1 - u - v) * a + u * b + v * c
    */
    Hit intersect( const Ray &ray, const float htmax ) const
    {
        Vector pvec= cross(ray.d, e2);
        float det= dot(e1, pvec);
        
        float inv_det= 1 / det;
        Vector tvec(p, ray.o);

        float u= dot(tvec, pvec) * inv_det;
        if(u < 0 || u > 1) return Hit();

        Vector qvec= cross(tvec, e1);
        float v= dot(ray.d, qvec) * inv_det;
        if(v < 0 || u + v > 1) return Hit();

        float t= dot(e2, qvec) * inv_det;
        if(t > htmax || t < 0) return Hit();
        
        return Hit(id, t, u, v);           // p(u, v)= (1 - u - v) * a + u * b + v * c
    }
};


// ensemble de triangles. 
// a remplacer par une vraie structure acceleratrice, un bvh, par exemple
struct BVH
{
    std::vector<Triangle> triangles;
    
    BVH( ) = default;
    BVH( const Mesh& mesh ) { build(mesh); }
    
    void build( const Mesh& mesh )
    {
        triangles.clear();
        triangles.reserve(mesh.triangle_count());
        for(int id= 0; id < mesh.triangle_count(); id++)
        {
            TriangleData data= mesh.triangle(id);
            triangles.push_back( Triangle(data.a, data.b, data.c, id) );
        }
        
        printf("%d triangles\n", int(triangles.size()));
        assert(triangles.size());
    }
    
    Hit intersect( const Ray& ray ) const
    {
        Hit hit;
        float tmax= ray.tmax;
        for(int id= 0; id < int(triangles.size()); id++)
            // ne renvoie vrai que si l'intersection existe dans l'intervalle [0 tmax]
            if(Hit h= triangles[id].intersect(ray, tmax))
            {
                hit= h;
                tmax= h.t;
            }
        
        return hit;        
    }
    
    bool visible( const Ray& ray ) const
    {
        for(int id= 0; id < int(triangles.size()); id++)
            if(triangles[id].intersect(ray, ray.tmax))
                return false;
        
        return true;
    }
};


struct Source
{
    Point a, b, c;
    Color emission;
    Vector n;
    float area;
    
    Source( ) : a(), b(), c(), emission(), n(), area() {}
    
    Source( const TriangleData& data, const Color& color ) : a(data.a), b(data.b), c(data.c), emission(color)
    {
       // normale geometrique du triangle abc, produit vectoriel des aretes ab et ac
        Vector ng= cross(Vector(a, b), Vector(a, c));
        n= normalize(ng);
        area= length(ng) / 2;
    }
    
    Point sample( const float u1, const float u2 ) const
    {
        // cf GI compemdium eq 18
        float r1= std::sqrt(u1);
        float alpha= 1 - r1;
        float beta= (1 - u2) * r1;
        float gamma= u2 * r1;
        return alpha*a + beta*b + gamma*c;
    }
    
    float pdf( const Point& p ) const
    {
        // todo : devrait renvoyer 0 pour les points a l'exterieur du triangle...
        return 1.f / area;
    }
};


struct Sources
{
    std::vector<Source> sources;
    float emission;     // emission totale des sources
    float area;         // aire totale des sources
    
    Sources( const Mesh& mesh ) : sources()
    {
        build(mesh);
        
        printf("%d sources\n", int(sources.size()));
        assert(sources.size());
    }
    
    void build( const Mesh& mesh )
    {
        area= 0;
        emission= 0;
        sources.clear();
        for(int id= 0; id < mesh.triangle_count(); id++)
        {
            const TriangleData& data= mesh.triangle(id);
            const Material& material= mesh.triangle_material(id);
            if(material.emission.power() > 0)
            {
                Source source(data, material.emission);
                emission= (emission + source.area * source.emission.power());
                area= area + source.area;
                
                sources.push_back(source);
            }
        }
    }
    
    int size( ) const { return int(sources.size()); }
    const Source& operator() ( const int id ) const { return sources[id]; }
};


// utilitaires
// construit un repere ortho tbn, a partir d'un seul vecteur, la normale d'un point d'intersection, par exemple.
// permet de transformer un vecteur / une direction dans le repere du monde.

// cf "generating a consistently oriented tangent space" 
// http://people.compute.dtu.dk/jerf/papers/abstracts/onb.html
// cf "Building an Orthonormal Basis, Revisited", Pixar, 2017
// http://jcgt.org/published/0006/01/01/
struct World
{
    World( const Vector& _n ) : n(_n) 
    {
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


float f( const Vector& w )
{
    // evalue cos theta
    Vector n= Vector(0, 0, 1);
    assert(w.z > 0);
    return dot(n, w);
}

// genere une direction sur l'hemisphere, 
// cf GI compendium, eq 34
Vector sample34( const float u1, const float u2 )
{
    // coordonnees theta, phi
    float cos_theta= u1;
    float phi= float(2 * M_PI) * u2;
    
    // passage vers x, y, z
    float sin_theta= std::sqrt(1 - cos_theta*cos_theta);
    return Vector( std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta );
}

// evalue la densite de proba, la pdf de la direction, cf GI compendium, eq 34
float pdf34( const Vector& w )
{
    if(w.z < 0) return 0;
    return 1 / float(2 * M_PI);
}

// genere une direction sur l'hemisphere, 
// cf GI compendium, eq 35
Vector sample35( const float u1, const float u2 )
{
    // coordonnees theta, phi
    float cos_theta= std::sqrt(u1);
    float phi= float(2 * M_PI) * u2;
    
    // passage vers x, y, z
    float sin_theta= std::sqrt(1 - cos_theta*cos_theta);
    return Vector( std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta );
}

// evalue la densite de proba, la pdf de la direction, cf GI compendium, eq 35
float pdf35( const Vector& w )
{
    if(w.z < 0) return 0;
    return w.z / float(M_PI);
}

Color color_direct_direction(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material, Mesh mesh, BVH bvh, int N_point_Source, Vector pn, Point p){

    Color color= Black();

    for (int ni=0; ni<N_point_Source; ni++){

        float u1= u01(rng);
        float u2= u01(rng);
        
        // generer une direction (dans un repere local, arbitraire)
        Vector v= sample35(u1, u2);
        float pdf= pdf35(v);
        
        // changement de repere vers la scene
        World world(pn);
        Vector d= world(v);
        
        // evaluer la visibilite
        const float scale= 10;  
        // le vecteur d est de longueur 1, utiliser un vecteur plus grand, en fonction du rayon de la sphere englobante de la scene
        // rappel : on veut savoir si le point p voit le ciel... qui est a l'exterieur de la scene.
        
        Ray ray2(p +0.001f*pn, d * scale);
        //ray2.tmax = 1 - .0001f ;
        Hit hit2= bvh.intersect(ray2);
        if(hit2 == true)
        {
            // pas de geometrie dans la direction d, p est donc eclaire par le ciel
            
            // evaluer les termes de la fonction a integrer
            // V(p, d) == 1, puisqu'on a pas trouve d'intersection
            float cos_theta= std::max(float(0), dot(pn, d));

            //
            // occultation ambiante 
            //
            //color= color + 1 / float(M_PI) * material.diffuse * cos_theta / (pdf*N_point_Source);

            const Material& material2= mesh.triangle_material(hit2.triangle_id);
            Color emission_si = material2.emission;

            
            const TriangleData& triangle2= mesh.triangle(hit2.triangle_id);
            Vector sn= normal(hit2, triangle2);
            Point p_sn= point(hit2, ray2);

            float cos_theta_s = std::max(0.f, dot(sn, normalize(-d)));
            //
            // full calcul
            //
            color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * 1.f / (length2(p_sn-p)*N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* cos_theta * 1.f / (N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* float(M_PI)* 1.f / (N_point_Source);
            
        }
    }
    return color;
}

Color color_ambiant_direction(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material, Mesh mesh, BVH bvh, int N_point_Source, Vector pn, Point p){

    Color color= Black();

    for (int ni=0; ni<N_point_Source; ni++){

        float u1= u01(rng);
        float u2= u01(rng);
        
        // generer une direction (dans un repere local, arbitraire)
        Vector v= sample34(u1, u2);
        float pdf= pdf34(v);
        
        // changement de repere vers la scene
        World world(pn);
        Vector d= world(v);
        
        // evaluer la visibilite
        const float scale= 10;  
        // le vecteur d est de longueur 1, utiliser un vecteur plus grand, en fonction du rayon de la sphere englobante de la scene
        // rappel : on veut savoir si le point p voit le ciel... qui est a l'exterieur de la scene.
        
        Ray ray2(p +0.001f*pn, d * scale);
        //ray2.tmax = 1 - .0001f ;
        Hit hit2= bvh.intersect(ray2);
        if(hit2 == false)
        {
            // pas de geometrie dans la direction d, p est donc eclaire par le ciel
            
            // evaluer les termes de la fonction a integrer
            // V(p, d) == 1, puisqu'on a pas trouve d'intersection
            float cos_theta= std::max(float(0), dot(pn, d));

            //
            // occultation ambiante 
            //
            color= color + 1 * material.diffuse * cos_theta / (float(M_PI)*pdf*N_point_Source);

            // const Material& material2= mesh.triangle_material(hit2.triangle_id);
            // Color emission_si = material2.emission;

            
            // const TriangleData& triangle2= mesh.triangle(hit2.triangle_id);
            // Vector sn= normal(hit2, triangle2);
            // Point p_sn= point(hit2, ray2);

            // float cos_theta_s = std::max(0.f, dot(sn, normalize(-d)));
            // //
            // // full calcul
            // //
            // color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * 1.f / (length2(p_sn-p)*N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* cos_theta * 1.f / (N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* float(M_PI)* 1.f / (N_point_Source);
            
        }
    }
    return color;
}



Color color_direct_sources_cornell(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material, Sources sources, BVH bvh, int N_Source, int N_point_Source, Vector pn, Point p){

    Color color= Black();

    for (int ni=0; ni<N_point_Source; ni++){
        //
        // random source
        //
        int si = (int) N_Source* u01(rng);

        //
        // random source according area
        //
        // int si;
        // int P_si = (int) sources.area*u01(rng); //entre 0 et 20000
        // if (P_si <= 5000){ 
        //     si=N_Source-1;
        // }
        // else if (P_si <= 10000){
        //     si=N_Source-2;
        // }
        // else {
        //     si = (int) (N_Source-2)* u01(rng);
        // }

        // position et emission de la source de lumiere si
        float u1=u01(rng);
        float u2=u01(rng);

        //Point s= (Point(sources(si).a) + Point(sources(si).b) + Point(sources(si).c))/3.0;
        Point s= sources(si).sample(u1,u2);
        Color emission_si= sources(si).emission;

        // direction de p vers la source s
        Vector p_to_s= Vector(p, s);

        // visibilite entre p et s
        float v= 1;

        //rayon de p vers s
        Ray shadow_ray(p + 0.0001f * pn, p_to_s);//+ 0.001f * pn
        shadow_ray.tmax = 1 - .0001f ;//

        //if(bvh.visible(shadow_ray) != 1)
        if(Hit hit2= bvh.intersect(shadow_ray))
        {
            // on vient de trouver un triangle entre p et s. p est donc a l'ombre
            v= 0;
        }

        Vector sn= sources(si).n;// normale du triangle au point de la source  interpolee ?


        // accumuler la couleur de l'echantillon
        float cos_theta= std::max(0.f, dot(pn, normalize(p_to_s)));
        float cos_theta_s= std::max(0.f, dot(sn, normalize(-p_to_s)));
        //
        // random source
        //
        color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * v * 1.f / (length2(p_to_s)*N_point_Source*N_Source*sources(si).pdf(s) );

        //
        // random source according area
        //
        //color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * v * sources.area * 1.f / (length2(p_to_s)*N_point_Source );

    }
    return color;
}


Color color_indirect_direction(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material, Mesh mesh,Sources sources, BVH bvh, int N_point_Source,int N_Source, Vector pn, Point p){

    Color color= Black();

    for (int ni=0; ni<N_point_Source; ni++){

        float u1= u01(rng);
        float u2= u01(rng);
        
        // generer une direction (dans un repere local, arbitraire)
        Vector v= sample35(u1, u2);
        float pdf= pdf35(v);
        
        // changement de repere vers la scene
        World world(pn);
        Vector d= world(v);
        
        // evaluer la visibilite
        const float scale= 10;  
        // le vecteur d est de longueur 1, utiliser un vecteur plus grand, en fonction du rayon de la sphere englobante de la scene
        // rappel : on veut savoir si le point p voit le ciel... qui est a l'exterieur de la scene.
        
        Ray ray2(p +0.001f*pn, d * scale);
        //ray2.tmax = 1 - .0001f ;
        Hit hit2= bvh.intersect(ray2);
        if(hit2 == true)
        {
            // pas de geometrie dans la direction d, p est donc eclaire par le ciel
            
            // evaluer les termes de la fonction a integrer
            // V(p, d) == 1, puisqu'on a pas trouve d'intersection
            float cos_theta= std::max(float(0), dot(pn, d));



            const Material& material2= mesh.triangle_material(hit2.triangle_id);
            //Color emission_q = material2.emission;

            
            const TriangleData& triangle2= mesh.triangle(hit2.triangle_id);
            Vector qn= normal(hit2, triangle2);
            Point p_qn= point(hit2, ray2);

            float cos_theta_s = std::max(0.f, dot(qn, normalize(-d)));
            int N_point_Source_direct = 4;

            Color emission_q = color_direct_sources_cornell( rng, u01, material2, sources, bvh, N_Source, N_point_Source_direct, qn, p_qn);
            //color= color +  emission_q*material.diffuse* cos_theta_s* cos_theta * 1.f / (length2(p_qn-p)*N_point_Source*pdf );
            //
            // occultation ambiante 
            //
            color= color + emission_q * material.diffuse * cos_theta / (pdf*N_point_Source);


            // //
            // // full calcul
            // //
            //color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * 1.f / (length2(p_sn-p)*N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* cos_theta * 1.f / (N_point_Source*pdf );
            //color= color +  emission_si*material.diffuse* float(M_PI)* 1.f / (N_point_Source);
            
        }
    }
    return color;
}



 
Color color_direct_sources_Area_emission(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material, Sources sources, BVH bvh, int N_Source, int N_point_Source, Vector pn, Point p){

    Color color= Black();

    for (int ni=0; ni<N_point_Source; ni++){
        //
        // random source
        //
        int si = (int) N_Source* u01(rng);

        //
        // random source according area
        //
        // int si;
        // int P_si = (int) sources.area*u01(rng); //entre 0 et 20000
        // if (P_si <= 5000){ 
        //     si=N_Source-1;
        // }
        // else if (P_si <= 10000){
        //     si=N_Source-2;
        // }
        // else {
        //     si = (int) (N_Source-2)* u01(rng);
        // }

        // position et emission de la source de lumiere si
        float u1=u01(rng);
        float u2=u01(rng);

        //Point s= (Point(sources(si).a) + Point(sources(si).b) + Point(sources(si).c))/3.0;
        Point s= sources(si).sample(u1,u2);
        Color emission_si= sources(si).emission;

        // direction de p vers la source s
        Vector p_to_s= Vector(p, s);

        // visibilite entre p et s
        float v= 1;

        //rayon de p vers s
        Ray shadow_ray(p + 0.0001f * pn, p_to_s);//+ 0.001f * pn
        shadow_ray.tmax = 1 - .0001f ;//

        //if(bvh.visible(shadow_ray) != 1)
        if(Hit hit2= bvh.intersect(shadow_ray))
        {
            // on vient de trouver un triangle entre p et s. p est donc a l'ombre
            v= 0;
        }

        Vector sn= sources(si).n;// normale du triangle au point de la source  interpolee ?


        // accumuler la couleur de l'echantillon
        float cos_theta= std::max(0.f, dot(pn, normalize(p_to_s)));
        float cos_theta_s= std::max(0.f, dot(sn, normalize(-p_to_s)));
        //
        // random source
        //
        color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta* sources.area * v * 1.f / (length2(p_to_s)*N_point_Source*N_Source*sources(si).pdf(s) );

        //
        // random source according area
        //
        //color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * v * sources.area * 1.f / (length2(p_to_s)*N_point_Source );

    }
    return color;
}




Color color_Ultime(std::default_random_engine &rng, std::uniform_real_distribution<float> &u01, Material material,Mesh mesh, Sources sources, BVH bvh, int N_Source, int N_point_Source, Vector pn, Point p){


    Color color= Black();
    Color I1= Black();
    Color I2= Black();

    for (int ni=0; ni<N_point_Source; ni++){
        //
        // random source
        //
        //int si = (int) N_Source* u01(rng);

        //
        // random source according area
        //
        int si;
        int P_si = (int) sources.area*u01(rng); //entre 0 et 20000
        if (P_si <= 5000){ 
            si=N_Source-1; 
        }
        else if (P_si <= 10000){
            si=N_Source-2;
        }
        else {
            si = (int) (N_Source-2)* u01(rng);
        }
        float P1y1=1/sources.area;
        float P1y2=1/sources.area;


        //std::cout<<"area"<<sources(si).area<<std::endl;
        // position et emission de la source de lumiere si
        float u1=u01(rng);
        float u2=u01(rng);

        //Point s= (Point(sources(si).a) + Point(sources(si).b) + Point(sources(si).c))/3.0;
        Point s= sources(si).sample(u1,u2);   ///s = Y1i 
        Color emission_si= sources(si).emission;
        
        
        //Point p= (Point(data.a) + Point(data.b) + Point(data.c)) / 3;
        // interpoler la normale au point d'intersection
        //Vector pn= normal(mesh, hit);
        // direction de p vers la source s
        Vector l= Vector(p, s);

        // visibilite entre p et s
        float vu= 1;

        Ray shadow_ray(p + 0.0001f * pn, l);//+ 0.001f * pn
        shadow_ray.tmax = 1 - .0001f ;//

        //if(bvh.visible(shadow_ray) != 1)
        if(Hit hit2= bvh.intersect(shadow_ray))
        {
            // on vient de trouver un triangle entre p et s. p est donc a l'ombre
            vu= 0;
        }

        Vector sn= sources(si).n;// normale du triangle au point de la source  interpolee ?


        // accumuler la couleur de l'echantillon
        float cos_theta1= std::max(0.f, dot(pn, normalize(l)));
        float cos_theta_s1= std::max(0.f, dot(sn, normalize(-l)));
        //
        // random source
        //
        //color= color +  emission_si*material.diffuse* cos_theta_s* cos_theta * v * 1.f / (length2(l)*N_point_Source*N_Source*sources(si).pdf(s) );

        //
        // random source according area
        //
        //I1= emission_si*material.diffuse* cos_theta_s1* cos_theta1 * vu * 1.f / (length2(l)*N_point_Source );
        I1= emission_si*material.diffuse* cos_theta_s1* cos_theta1 * vu * sources.area * 1.f / (length2(l)*N_point_Source );




        float u12= u01(rng);
        float u22= u01(rng);
        
        // generer une direction (dans un repere local, arbitraire)
        Vector v= sample35(u12, u22);
        float pdf= pdf35(v);
        
        // changement de repere vers la scene
        World world(pn);
        Vector d= world(v);
        
        // evaluer la visibilite
        const float scale= 10;  
        // le vecteur d est de longueur 1, utiliser un vecteur plus grand, en fonction du rayon de la sphere englobante de la scene
        // rappel : on veut savoir si le point p voit le ciel... qui est a l'exterieur de la scene.
        
        Ray ray2(p +0.001f*pn, d * scale);
        //ray2.tmax = 1 - .0001f ;
        Hit hit2= bvh.intersect(ray2);

        float P2y1;
        float P2y2;

        if(hit2 == true)
        {
            // pas de geometrie dans la direction d, p est donc eclaire par le ciel
            
            // evaluer les termes de la fonction a integrer
            // V(p, d) == 1, puisqu'on a pas trouve d'intersection
            float cos_theta2= std::max(float(0), dot(pn, d));

            //
            // occultation ambiante 
            //
            //color= color + 1 / float(M_PI) * material.diffuse * cos_theta / (pdf*N_point_Source);

            const Material& material2= mesh.triangle_material(hit2.triangle_id);
            Color emission_si = material2.emission;

            
            const TriangleData& triangle2= mesh.triangle(hit2.triangle_id);
            Vector sn= normal(hit2, triangle2);
            Point p_sn= point(hit2, ray2); ///p_sn = Y2i 

            float cos_theta_s2 = std::max(0.f, dot(sn, normalize(-d)));
            //std::cout<<cos_theta2/float(M_PI)<<"  "<<pdf<<std::endl;

            P2y2=pdf*cos_theta_s2/(length2(p_sn-p));
            P2y1=cos_theta1*cos_theta_s1/(float(M_PI)*length2(l));
            //
            // full calcul
            //
            //I2= emission_si*material.diffuse* cos_theta_s2* cos_theta2 * 1.f / (length2(p_sn-p)*N_point_Source*pdf );
            //I2 = emission_si*material.diffuse* cos_theta2 * 1.f / (N_point_Source*pdf );
            I2 = (emission_si*material.diffuse* 1.f / (N_point_Source));

        }

        color = color + (P1y1/(P1y1+P2y1))*I1 + (P2y2/(P1y2+P2y2))*I2;

    }


    return color;
}





int main( const int argc, const char **argv )
{
    const char *mesh_filename= "data/cornell.obj";
    //const char *mesh_filename= "data/emission.obj";
    const char *orbiter_filename= "data/cornell_orbiter.txt";
    //const char *orbiter_filename= "data/emission_orbiter.txt";
    //const char *orbiter_filename= "data/orbiter.txt";
    //const char *orbiter_filename= "orbiter.txt";
    
    if(argc > 1) mesh_filename= argv[1];
    if(argc > 2) orbiter_filename= argv[2];
    
    printf("%s: '%s' '%s'\n", argv[0], mesh_filename, orbiter_filename);
    
    // creer l'image resultat
    Image image(1024, 640);
    
    // charger un objet
    Mesh mesh= read_mesh(mesh_filename);
    if(mesh.triangle_count() == 0)
        // erreur de chargement, pas de triangles
        return 1;
    
    // creer l'ensemble de triangles / structure acceleratrice
    BVH bvh(mesh);
    Sources sources(mesh);
    int N_Source=sources.size();
    std::cout<<sources.area<<std::endl;
    std::cout<<sources(N_Source-3).area<<std::endl;
    

    
    // charger la camera
    Orbiter camera;
    if(camera.read_orbiter(orbiter_filename))
        // erreur, pas de camera
        return 1;
    
    // recupere les transformations view, projection et viewport pour generer les rayons
    Transform model= Identity();
    Transform view= camera.view();
    Transform projection= camera.projection(image.width(), image.height(), 45);
    Transform viewport= Viewport(image.width(), image.height());

    auto cpu_start= std::chrono::high_resolution_clock::now();
    
    // parcourir tous les pixels de l'image
    // en parallele avec openMP, un thread par bloc de 16 lignes
#pragma omp parallel for schedule(dynamic, 1)
    for(int py= 0; py < image.height(); py++)
    {
        // nombres aleatoires, version c++11
        std::random_device seed;
        // un generateur par thread... pas de synchronisation
        std::default_random_engine rng(seed());
        // nombres aleatoires entre 0 et 1
        std::uniform_real_distribution<float> u01(0.f, 1.f);
        
        for(int px= 0; px < image.width(); px++)
        {
            Color color_direct= Black();
            Color color_indirect= Black();
            Color color= Black();
            
            // generer le rayon pour le pixel (x, y)
            float x= px + u01(rng);
            float y= py + u01(rng);
            
            //Point o= { (viewport*projection*view).inverse()(Point(x,y,0)) }; // origine dans l'image
            Point o= { camera.position() }; // origine dans l'image
            Point e= { (viewport*projection*view).inverse()(Point(x,y,1)) }; // extremite dans l'image
            
            Ray ray(o, e);
            Hit hit;
            // calculer les intersections 
            if(hit= bvh.intersect(ray))
            {
                const TriangleData& triangle= mesh.triangle(hit.triangle_id);           // recuperer le triangle
                const Material& material= mesh.triangle_material(hit.triangle_id);      // et sa matiere


                // position du point d'intersection
                //Point p= ray.o + hit.t * ray.d;
                Point p= point(hit, ray);               // point d'intersection
                Vector pn= normal(hit, triangle);       // normale interpolee du triangle au point d'intersection
                // retourne la normale pour faire face a la camera / origine du rayon...
                if(dot(pn, ray.d) > 0)
                    pn= -pn;

                Color emission= material.emission;
                World world(pn);



                int N_point_Source=32;
                int N_point_Source_direct=16;



                float P_Si_float = (int) (N_Source)* u01(rng);
                //int P_Si = (int) P_Si_float;
                //std::cout<<P_Si_float<<" int "<<P_Si<<std::endl;

                //color = color_direct_direction( rng, u01, material, mesh, bvh, N_point_Source,pn, p);
                //color = color_direct_sources_Area_emission( rng, u01, material, sources, bvh, N_Source, N_point_Source,pn, p); //OK
                color = color_ambiant_direction( rng, u01, material, mesh, bvh, N_point_Source,pn, p);

                //color_direct = color_direct_sources_cornell( rng, u01, material, sources, bvh, N_Source, N_point_Source_direct,pn, p);
                //color_indirect = color_indirect_direction(rng, u01, material, mesh,sources, bvh, N_point_Source, N_Source, pn, p);
                //color = color_Ultime(rng, u01, material, mesh, sources, bvh, N_Source, N_point_Source, pn, p);
                
                //color = color_indirect/2 + color_direct/2;
                float gamma = 2.2;
                color.r=pow(color.r,1.0/gamma);
                color.g=pow(color.g,1.0/gamma);
                color.b=pow(color.b,1.0/gamma);
                color =  emission + color;

            }


        image(px, py)= Color(color, 1);
        }
    }
    
    auto cpu_stop= std::chrono::high_resolution_clock::now();
    int cpu_time= std::chrono::duration_cast<std::chrono::milliseconds>(cpu_stop - cpu_start).count();
    printf("cpu  %ds %03dms\n", int(cpu_time / 1000), int(cpu_time % 1000));
    
    // enregistrer l'image resultat
    write_image(image, "render.png");
    write_image_hdr(image, "render.hdr");
    return 0;
}
