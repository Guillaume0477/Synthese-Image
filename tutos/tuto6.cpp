
//! \file tuto6.cpp dessiner un objet texture
// utiliser mesh pour charger un objet .obj
// camera pour le dessiner du point de vue d'une camera + controle de la camera a la souris
// texture : creation a partir d'un fichier, utilisation avec draw(mesh, ...) et destruction avec glDeleteTextures( )

#include <stdio.h>
#include "window.h"

#include "mesh.h"
#include "wavefront.h"  // pour charger un objet au format .obj
#include "texture.h"
#include "program.h"
#include "uniforms.h"


#include "orbiter.h"

#include "draw.h"        // pour dessiner du point de vue d'une camera

int l=11;
int w=11;
int lon = l*w;
Mesh objet[11*11];
Transform model[11*11];
GLuint texture;
GLuint m_program;
Orbiter camera;

int init( )
{
    // etape 1 : charger un objet
    //objet= read_mesh("data/Cyclops.obj");

    //CHANGE
    
    Point pmin, pmax;
    for (int i=0; i< l*w; i++) {
	objet[i]= read_mesh("data/cube.obj");
    }
  	
    objet[0].bounds(pmin, pmax);
    camera.lookat(0*pmin,22*pmax);
   

    m_program= read_program("tutos/tuto9_color.glsl");
    program_print_errors(m_program);
        
    // etape 2 : creer une camera pour observer l'objet
    // construit l'englobant de l'objet, les extremites de sa boite englobante
    //Point pmin, pmax, pmin2, pmax2;
    //objet.bounds(pmin, pmax);

    //CHANGE
    //objet2.bounds(pmin2, pmax2);


    // regle le point de vue de la camera pour observer l'objet
    //camera.lookat(pmin,pmax);
    //camera.lookat(pmin2,pmax2);

    printf("pmin %d", pmin);
    printf("pmax %d", pmax);
    // etape 3 : charger une texture a aprtir d'un fichier .bmp, .jpg, .png, .tga, etc, utilise read_image( ) et sdl_image
/*
    openGL peut utiliser plusieurs textures simultanement pour dessiner un objet, il faut les numeroter.
    une texture et ses parametres sont selectionnes sur une unite de texture.
    et ce sont les unites de texture qui sont utilisees pour dessiner un objet.

    l'exemple cree la texture sur l'unite 0 avec les parametres par defaut
 */
    //texture= read_texture(0, "data/debug2x2red.png");

    // etat openGL par defaut
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre

    // etape 3 : configuration du pipeline.
    glClearDepth(3.f);                          // profondeur par defaut
    glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
    glEnable(GL_DEPTH_TEST);                    // activer le ztest

    return 0;   // ras, pas d'erreur
}

int draw( )
{
    // etape 2 : dessiner l'objet avec opengl

    // on commence par effacer la fenetre avant de dessiner quelquechose
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // on efface aussi le zbuffer

    // recupere les mouvements de la souris, utilise directement SDL2
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);

    // deplace la camera
    if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
        // tourne autour de l'objet
        camera.rotation(mx, my);

    else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
        // approche / eloigne l'objet
        camera.move(mx);

    else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
        // deplace le point de rotation
        camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());
    
    for (int i=0; i<l; i++){
	for (int j=0; j<w; j++){
	    model[i*j+j] = Translation(2*i, 0, 2*j);
	    //draw(objet[i*j+j], model[i*j+j], camera, texture);
            Transform view= camera.view();
	    Transform projection= camera.projection(window_width(), window_height(), 45);
            Transform mvp= projection * view * model[i*j+j];

	    program_uniform(m_program, "mvpMatrix", mvp);

	    program_uniform(m_program, "color", vec4(0, 1, 0, 1));
            
            objet[i*j+j].draw(m_program, /* use position */ true, /* use texcoord */ false, /* use normal */ false, /* use color */ false, /* use material index*/ false);
        
	}
    }
    //CHANGE
    //model= Translation(2, 0, 0);


    // passer la texture en parametre 
    //draw(objet, Translation(-2, 0, 0), camera, texture);
    //draw(objet2, model, camera, texture);
    return 1;
}

int quit( )
{
    // etape 3 : detruire la description de l'objet
    
    for (int i=0; i< 1; i++) {
         objet[i].release();
    }
    
    // et la texture
    //glDeleteTextures(1, &texture);

    return 0;   // ras, pas d'erreur
}


int main( int argc, char **argv )
{
    // etape 1 : creer la fenetre
    Window window= create_window(1024, 640);
    if(window == NULL)
        return 1;

    // etape 2 : creer un contexte opengl pour pouvoir dessiner
    Context context= create_context(window);
    if(context == NULL)
        return 1;

    // etape 3 : creation des objets
    if(init() < 0)
    {
        printf("[error] init failed.\n");
        return 1;
    }

    // etape 4 : affichage de l'application, tant que la fenetre n'est pas fermee. ou que draw() ne renvoie pas 0
    run(window, draw);

    // etape 5 : nettoyage
    quit();
    release_context(context);
    release_window(window);
    return 0;
}
