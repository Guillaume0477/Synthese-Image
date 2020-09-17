
//! \file tuto9_buffers.cpp utilisation d'un shader 'utilisateur' pour afficher un m_objet Mesh + creation des buffers / vertex array object

#include "mat.h"
#include "mesh.h"
#include "wavefront.h"
#include "texture.h"
#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "draw.h"

#include "app.h" // classe Application a deriver

struct Buffers
{
    GLuint vao;
    GLuint vertex_buffer;
    int vertex_count;

    Buffers() : vao(0), vertex_buffer(0), vertex_count(0) {}

    void create(const Mesh &mesh)
    {
        if (!mesh.vertex_buffer_size())
            return;

        if (mesh.materials().count() == 0)
            // pas de matieres, pas d'affichage
            return;
        printf("%d materials.\n", mesh.materials().count());

        // cree et initialise le buffer: conserve la positions des sommets
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        size_t size = mesh.vertex_buffer_size() + mesh.texcoord_buffer_size() + mesh.normal_buffer_size() + mesh.vertex_count() * sizeof(unsigned char);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

        // cree et configure le vertex array object: conserve la description des attributs de sommets
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        /*
         // attribut 0, position des sommets, declare dans le vertex shader : in vec3 position;
         glVertexAttribPointer(0, 
             3, GL_FLOAT,    // size et type, position est un vec3 dans le vertex shader
             GL_FALSE,       // pas de normalisation des valeurs
             0,              // stride 0, les valeurs sont les unes a la suite des autres
             0               // offset 0, les valeurs sont au debut du buffer
         );
         glEnableVertexAttribArray(0);
         */

        // transfere les positions des sommets
        size_t offset = 0;
        size = mesh.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.vertex_buffer());
        // et configure l'attribut 0, vec3 position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(0);

        // transfere les texcoords des sommets
        offset = offset + size;
        size = mesh.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.texcoord_buffer());
        // et configure l'attribut 1, vec2 texcoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(1);

        // transfere les normales des sommets
        offset = offset + size;
        size = mesh.normal_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.normal_buffer());
        // et configure l'attribut 2, vec3 normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(2);

        // transfere les indices materiel
        offset = offset + size;
        size = mesh.vertex_count() * sizeof(unsigned char);
        assert(int(mesh.vertex_count() / 3) == mesh.triangle_count());

        // prepare un indice de matiere par sommet / 3 indices par triangle
        std::vector<unsigned char> buffer(mesh.vertex_count());
        for (int i = 0; i < int(mesh.vertex_count() / 3); i++)
        {
            int index = mesh.triangle_material_index(i);
            buffer[3 * i] = index;
            buffer[3 * i + 1] = index;
            buffer[3 * i + 2] = index;
        }

        glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer.data());
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 0, (const void *)offset);
        glEnableVertexAttribArray(3);

        // conserve le nombre de sommets
        vertex_count = mesh.vertex_count();
    }

    void release()
    {
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteVertexArrays(1, &vao);
        
    }
};

class TP : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP() : App(1024, 640) {}

    int init()
    {
        // etape 1 : charger un m_objet
        //m_objet= read_mesh("data/Cyclops.obj");

        //CHANGE

        GLuint vao;
        GLuint vertex_buffer;
        int vertex_count;

        Point pmin, pmax;
        for (int i = 0; i < l * w; i++)
        {
            Mesh mesh = read_mesh("data/robot.obj");
            if (i == 0) // tous les m_objet sont identiques (meme matieres)
            {
                // recupere les matieres.
                // le shader declare un tableau de 16 matieres
                m_colors.resize(16);

                // copier les matieres utilisees
                const Materials &materials = mesh.materials();
                assert(materials.count() <= int(m_colors.size()));
                for (int i = 0; i < materials.count(); i++)
                {
                    m_colors[i] = materials.material(i).diffuse;
                }
            }
            m_objet[i].create(mesh);

            mesh.bounds(pmin, pmax);

            /*glGenBuffers(1, &vertex_buffer);
         glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
         glBufferData(GL_ARRAY_BUFFER, m_objet[i].vertex_buffer_size(), m_objet[i].vertex_buffer(), GL_STATIC_DRAW);
    
         // cree et configure le vertex array object: conserve la description des attributs de sommets
         glGenVertexArrays(1, &vao);
         glBindVertexArray(vao);

         glEnableVertexAttribArray(0);

         // conserve le nombre de sommets
         vertex_count= m_objet[i].vertex_count();
    */
        }

        //mesh.bounds(pmin, pmax);
        pmax(1) = 0;
        m_camera.lookat(pmin, 4 * l * pmax);

        m_program = read_program("tutos/tuto9_materials.glsl");
        program_print_errors(m_program);

        // etape 2 : creer une m_camera pour observer l'm_objet
        // construit l'englobant de l'm_objet, les extremites de sa boite englobante
        //Point pmin, pmax, pmin2, pmax2;
        //m_objet.bounds(pmin, pmax);

        //CHANGE
        //m_objet2.bounds(pmin2, pmax2);

        // regle le point de vue de la m_camera pour observer l'm_objet
        //m_camera.lookat(pmin,pmax);
        //m_camera.lookat(pmin2,pmax2);

        printf("pmin %d", pmin);
        printf("pmax %d", pmax);
        // etape 3 : charger une texture a aprtir d'un fichier .bmp, .jpg, .png, .tga, etc, utilise read_image( ) et sdl_image
        /*
    openGL peut utiliser plusieurs textures simultanement pour dessiner un m_objet, il faut les numeroter.
    une texture et ses parametres sont selectionnes sur une unite de texture.
    et ce sont les unites de texture qui sont utilisees pour dessiner un m_objet.

    l'exemple cree la texture sur l'unite 0 avec les parametres par defaut
 */

        //textures
        m_texture0 = read_texture(0, "data/debug2x2red.png");
        m_texture1 = read_texture(1, "data/pacman.png");

        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f); // couleur par defaut de la fenetre

        // etape 3 : configuration du pipeline.
        glClearDepth(3.f);       // profondeur par defaut
        glDepthFunc(GL_LESS);    // ztest, conserver l'intersection la plus proche de la m_camera
        glEnable(GL_DEPTH_TEST); // activer le ztest

        return 0; // ras, pas d'erreur
    }

    int quit( )
    {
        // etape 4 : detruire le shader program
        release_program(m_program);
        for (int i = 0; i < 1; i++)
        {
            m_objet[i].release();
        }
        glDeleteTextures(1, &m_texture0);
        glDeleteTextures(1, &m_texture1);
        return 0;
    }

    // dessiner une nouvelle image
    int render()
    {
    // etape 2 : dessiner l'm_objet avec opengl

    // on commence par effacer la fenetre avant de dessiner quelquechose
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // on efface aussi le zbuffer

    // recupere les mouvements de la souris, utilise directement SDL2
    int mx, my;
    unsigned int mb = SDL_GetRelativeMouseState(&mx, &my);

    // deplace la m_camera
    if (mb & SDL_BUTTON(1)) // le bouton gauche est enfonce
        // tourne autour de l'm_objet
        m_camera.rotation(mx, my);

    else if (mb & SDL_BUTTON(3)) // le bouton droit est enfonce
        // approche / eloigne l'm_objet
        m_camera.move(mx);

    else if (mb & SDL_BUTTON(2)) // le bouton du milieu est enfonce
        // deplace le point de rotation
        m_camera.translation((float)mx / (float)window_width(), (float)my / (float)window_height());

    for (int i = 0; i < l; i++)
    {
        for (int j = 0; j < w; j++)
        {

            int location;
            glUseProgram(m_program);

            m_model[i * j + j] = Translation(8 * i, 0, 8 * j);
            //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
            Transform view = m_camera.view();
            Transform projection = m_camera.projection(window_width(), window_height(), 45);
            Transform mv = view * m_model[i * j + j];
            Transform mvp = projection * view * m_model[i * j + j];

            //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
            location = glGetUniformLocation(m_program, "mvpMatrix");
            glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
            //program_uniform(m_program, "mvMatrix", mv.buffer());

            location = glGetUniformLocation(m_program, "mvMatrix");
            glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

            //color
            //program_uniform(m_program, "color", vec4(0, 1, 0, 1));

            //textures
            program_use_texture(m_program, "texture0", 0, m_texture0);
            program_use_texture(m_program, "texture1", 1, m_texture1);

            location = glGetUniformLocation(m_program, "materials");
            glUniform4fv(location, m_colors.size(), &m_colors[0].r);

            glBindVertexArray(m_objet[i * j + j].vao);
            glDrawArrays(GL_TRIANGLES, 0, m_objet[i * j + j].vertex_count);

            //m_objet[i*j+j].draw(m_groups[k].first, m_groups[k].n, m_program, /* use position */ true, /* use texcoord */ true, /* use normal */ false, /* use color */ false, /* use material index*/ false);
        }
    }
    //CHANGE
    //m_model= Translation(2, 0, 0);

    // passer la texture en parametre
    //draw(m_objet, Translation(-2, 0, 0), m_camera, texture);
    //draw(m_objet2, m_model, m_camera, texture);
    return 1;
}

protected:
    int l = 4;
    int w = 4;
    int lon = l * w;
    Transform m_model[4 * 4];
    Buffers m_objet[4 * 4];
    Orbiter m_camera;
    GLuint m_texture0;
    GLuint m_texture1;
    std::vector<Color> m_colors;
    GLuint m_program;
};

int main(int argc, char **argv)
{
    TP tp;
    tp.run();

    return 0;
}
