
//! \file tuto9_buffers.cpp utilisation d'un shader 'utilisateur' pour afficher un m_objet Mesh + creation des buffers / vertex array object

#include <chrono>

#include "mat.h"
#include "mesh.h"
#include "wavefront.h"
#include "texture.h"
#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "draw.h"
#include <stdio.h>

#include "app.h" // classe Application a deriver
#include "text.h"

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

        /******************************************
        *    Init vao et buffers for cubes        *
        ******************************************/

        // cree et initialise le buffer: conserve la positions des sommets
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        size_t size = mesh.vertex_buffer_size() + mesh.texcoord_buffer_size() + mesh.normal_buffer_size() + mesh.vertex_count() * sizeof(unsigned char);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

        // cree et configure le vertex array object: conserve la description des attributs de sommets
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

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

    void create(const Mesh &mesh, const Mesh &mesh2)
    {
        if (!mesh.vertex_buffer_size())
            return;

        if (mesh.materials().count() == 0)
            // pas de matieres, pas d'affichage
            return;

        
        /******************************************
        *    Init vao et buffers for robots       *
        ******************************************/


        // cree et initialise le buffer: conserve la positions des sommets
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        size_t size = mesh.vertex_buffer_size() + mesh2.vertex_buffer_size() + mesh.texcoord_buffer_size() + mesh2.texcoord_buffer_size() + mesh.normal_buffer_size() + mesh2.normal_buffer_size() + mesh.vertex_count() * sizeof(unsigned char);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

        // cree et configure le vertex array object: conserve la description des attributs de sommets
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // transfere les positions des sommets
        size_t offset = 0;
        size = mesh.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.vertex_buffer());
        // et configure l'attribut 0, vec3 position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(0);

        // transfere les positions des sommets2
        offset = offset + size;
        size = mesh2.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh2.vertex_buffer());
        // et configure l'attribut 0, vec3 position
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(1);

        // transfere les texcoords des sommets
        offset = offset + size;
        size = mesh.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.texcoord_buffer());
        // et configure l'attribut 1, vec2 texcoord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(2);

        // transfere les texcoords des sommets2
        offset = offset + size;
        size = mesh2.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh2.texcoord_buffer());
        // et configure l'attribut 1, vec2 texcoord
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(3);

        // transfere les normales des sommets
        offset = offset + size;
        size = mesh.normal_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.normal_buffer());
        // et configure l'attribut 2, vec3 normal
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(4);

        // transfere les normales des sommets2
        offset = offset + size;
        size = mesh2.normal_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh2.normal_buffer());
        // et configure l'attribut 2, vec3 normal
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *)offset);
        glEnableVertexAttribArray(5);

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
        glVertexAttribIPointer(6, 1, GL_UNSIGNED_BYTE, 0, (const void *)offset);
        glEnableVertexAttribArray(6);

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
    TP() : App(1024, 640)
    {
        // desactive vsync pour les mesures de temps
        SDL_GL_SetSwapInterval(0);
    }

    int init()
    {

        Point pmin_frame, pmax_frame;
        Point pmin, pmax;
        for (int i = 0; i < l * w; i++)
        {
            for (int k = 0; k < frame_s; k++)
            {
                char str_k[30];
                char str_k2[30];
                printf("\n CHHAAR : %s : %s : k = %d \n", str_k, str_k2, k);
                if ((k + 1) >= 10)
                {
                    sprintf(str_k, "data/Robot/Robot_0000%d.obj", (k % frame_s + 1));
                }
                else
                {
                    sprintf(str_k, "data/Robot/Robot_00000%d.obj", (k % frame_s + 1));
                }
                if (((k + 1) % frame_s + 1) >= 10)
                {
                    sprintf(str_k2, "data/Robot/Robot_0000%d.obj", (k + 1) % frame_s + 1);
                }
                else
                {
                    sprintf(str_k2, "data/Robot/Robot_00000%d.obj", (k + 1) % frame_s + 1);
                }

                Mesh mesh = read_mesh(str_k);
                Mesh mesh2 = read_mesh(str_k2);

                //Mesh mesh = read_mesh("data/cube.obj");
                //Mesh mesh2 = read_mesh("data/cube.obj");

                if (i == 0) // tous les m_objet sont identiques (meme matieres)
                {

                    mesh.bounds(pmin_frame, pmax_frame);
                    for (int j=0; j<3; j++)
                    {
                        if (pmin_frame(j)<pmin(j)){
                            pmin(j) = pmin_frame(j);
                        }
                        if (pmax_frame(j)>pmax(j)){
                            pmax(j) = pmax_frame(j);
                        }

                    }
                    if (k == 0) {
                        // recupere les matieres.
                        // le shader declare un tableau de 16 matieres
                        m_colors.resize(16);

                        // copier les matieres utilisees
                        const Materials &materials = mesh.materials();
                        assert(materials.count() <= int(m_colors.size()));
                        for (int j = 0; j < materials.count(); j++)
                        {
                            m_colors[j] = materials.material(j).diffuse;
                        }
                    }
                }
                m_objet[i][k].create(mesh, mesh2);
            }
        }

        Point pmin_frame_2, pmax_frame_2;
        Point pmin_2, pmax_2;
        for (int i = 0; i < l_2 * w_2; i++)
        {

            Mesh mesh = read_mesh("data/cube.obj");
            Mesh mesh2 = read_mesh("data/cube.obj");

            if (i == 0) // tous les m_objet sont identiques (meme matieres)
            {

                mesh.bounds(pmin_frame_2, pmax_frame_2);
                for (int j=0; j<3; j++)
                {
                    if (pmin_frame_2(j)<pmin_2(j)){
                        pmin_2(j) = pmin_frame_2(j);
                    }
                    if (pmax_frame_2(j)>pmax_2(j)){
                        pmax_2(j) = pmax_frame_2(j);
                    }

                }
            }
            
            m_objet_2_cube[i].create(mesh);
            
        }


        
        /******************************************
        *              Init camera                *
        ******************************************/


        Point maxi = Point();


        maxi(0) = (l -1) * 8 + pmax(0);
        maxi(2) = (w -1) * 8 + pmax(2);
        maxi(1) = pmax(1);


        Point maxi2 = Point(); // 0 0 0


        maxi2(0) = (l_2 -1) * 2 + pmax_2(0);
        maxi2(2) = (w_2 -1) * 2 + pmax_2(2);
        maxi2(1) = pmax_2(1);



        std::cout<< pmin(0)<<"  "<<pmin(1)<<"  "<<pmin(2)<< std::endl;
        std::cout<< pmax(0)<<"  "<<pmax(1)<<"  "<<pmax(2)<< std::endl;
        std::cout<< pmin_2(0)<<"  "<<pmin_2(1)<<"  "<<pmin_2(2)<< std::endl;
        std::cout<< pmax_2(0)<<"  "<<pmax_2(1)<<"  "<<pmax_2(2)<< std::endl;



        m_camera.lookat(pmin_2, maxi2);
        m_framebuffer_camera.lookat(pmin, maxi);
        m_camera_origin.lookat(pmin, maxi);

              
        /******************************************
        *             Init quad mesh              *
        ******************************************/


        glGenVertexArrays(1, &quad_VertexArrayID);
        glBindVertexArray(quad_VertexArrayID);

        static const GLfloat g_quad_vertex_buffer_data[] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f,  1.0f, 0.0f,
        };

        GLuint quad_vertexbuffer;
        glGenBuffers(1, &quad_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);


        
        /******************************************
        *           Init framebuffer              *
        ******************************************/


        m_framebuffer_width = 1024;
        m_framebuffer_height = 640;

        // etape 1 : creer une texture couleur...
        glGenTextures(1, &m_color_buffer);
        glBindTexture(GL_TEXTURE_2D, m_color_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // etape 1 : creer une texture couleur...
        glGenTextures(1, &m_position_buffer);
        glBindTexture(GL_TEXTURE_2D, m_position_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // ... et tous ses mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // etape 1 : creer une texture couleur...
        glGenTextures(1, &m_normal_buffer);
        glBindTexture(GL_TEXTURE_2D, m_normal_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // ... et tous ses mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // etape 1 : creer une texture couleur...
        glGenTextures(1, &m_material_buffer);
        glBindTexture(GL_TEXTURE_2D, m_material_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // ... et tous ses mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // etape 3 : sampler, parametres de filtrage des textures
        glGenSamplers(1, &sampler);

        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAX_LEVEL, 0);
        //glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        //glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // etape 1 : creer aussi une texture depth, sinon pas de zbuffer...
        glGenTextures(1, &m_depth_buffer);
        glBindTexture(GL_TEXTURE_2D, m_depth_buffer);

        glTexImage2D(GL_TEXTURE_2D, 0,
                     GL_DEPTH_COMPONENT, m_framebuffer_width, m_framebuffer_height, 0,
                     GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        // etape 2 : creer et configurer un framebuffer object
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_COLOR_ATTACHMENT0, /* texture */ m_color_buffer, /* mipmap level */ 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_COLOR_ATTACHMENT1, /* texture */ m_position_buffer, /* mipmap level */ 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_COLOR_ATTACHMENT2, /* texture */ m_normal_buffer, /* mipmap level */ 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_COLOR_ATTACHMENT3, /* texture */ m_material_buffer, /* mipmap level */ 0);

        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_DEPTH_ATTACHMENT, /* texture */ m_depth_buffer, /* mipmap level */ 0);

        //le fragment shader ne declare qu'une seule sortie, indice 0
        GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, buffers);

        // nettoyage
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


        /******************************************
        *               Init shaders              *
        ******************************************/



        m_program = read_program("tutos/texcoords.glsl");
        program_print_errors(m_program);
        m_program_origin = read_program("tutos/tuto9_materials copy.glsl");
        program_print_errors(m_program2);
        m_program2 = read_program("tutos/tuto9_materials.glsl");
        program_print_errors(m_program2);
        m_program_quad = read_program("tutos/quad.glsl");
        program_print_errors(m_program_quad);

        // mesure du temps gpu de glDraw
        glGenQueries(1, &m_time_query);

        // affichage du temps  dans la fenetre
        m_console = create_text();

        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f); // couleur par defaut de la fenetre

        // etape 3 : configuration du pipeline.
        glClearDepth(1.f);       // profondeur par defaut
        glDepthFunc(GL_LESS);    // ztest, conserver l'intersection la plus proche de la m_camera
        glEnable(GL_DEPTH_TEST); // activer le ztest

        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        return 0; // ras, pas d'erreur
    }

    int quit()
    {
        // etape 4 : detruire le shader program
        release_program(m_program);
        for (int i = 0; i < 1; i++)
        {
            for (int k = 0; k < 2 * w; k++)
            {
                m_objet[i][k].release();
            }
        }
        glDeleteSamplers(1, &sampler);
        glDeleteTextures(1, &m_texture0);
        glDeleteTextures(1, &m_texture1);
        return 0;
    }


    void render_origin(){

        /******************************************
        *       Render avec 1 seule passe         *
        ******************************************/

        float my_time = global_time()/3; //same time for all robots
        //int k_frame; //same k_krame pour tout les passes //inutile

                // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer

        for (int i = 0; i < l; i++)
        {
            for (int j = 0; j < w; j++)
            {

                int location;
                glUseProgram(m_program_origin);

                m_model[i * j + j] = Translation(8 * i, 0, 8 * j);
                //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
                Transform view = m_camera_origin.view();
                Transform projection = m_camera_origin.projection(window_width(), window_height(), 45);
                Transform mv = view * m_model[i * j + j];
                Transform mvp = projection * view * m_model[i * j + j];

                //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
                location = glGetUniformLocation(m_program_origin, "mvpMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
                //program_uniform(m_program, "mvMatrix", mv.buffer());
                location = glGetUniformLocation(m_program_origin, "mvMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

                location = glGetUniformLocation(m_program_origin, "view");
                glUniformMatrix4fv(location, 1, GL_TRUE, view.buffer());

                location = glGetUniformLocation(m_program_origin, "model");
                glUniformMatrix4fv(location, 1, GL_TRUE, m_model[i * j + j].buffer());

                location = glGetUniformLocation(m_program_origin, "projection");
                glUniformMatrix4fv(location, 1, GL_TRUE, projection.buffer());

                //program_uniform(m_program, "temps", );
                location = glGetUniformLocation(m_program_origin, "temps");

                glUniform1f( location,((float) ((int) (frame_s*my_time)%1000))/1000 );
        

                //textures
                // texture et parametres de filtrage de la texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_texture0);
                glBindSampler(0, sampler);
            
                glActiveTexture(GL_TEXTURE0+1);
                glBindTexture(GL_TEXTURE_2D, m_texture1);
                glBindSampler(1, sampler);

                // uniform sampler2D declares par le fragment shader  //not used
                location= glGetUniformLocation(m_program_origin, "texture0");
                glUniform1i(location, 0);
            
                location= glGetUniformLocation(m_program_origin, "texture1");
                glUniform1i(location, 1);

                //program_use_texture(m_program, "texture0", m_texture0, sampler);
                //program_use_texture(m_program, "texture1", m_texture1, sampler);

                location = glGetUniformLocation(m_program_origin, "materials");
                glUniform4fv(location, m_colors.size(), &m_colors[0].r);
                
                int k_frame = (int) (frame_s*my_time/1000) % frame_s;//(int) (time)%(frame_s*1000)/1000;
                
                //k_frame = 3;(i * j + j)
                glBindVertexArray(m_objet[i * j + j][(k_frame+(i * j + j))%23].vao);
                glDrawArrays(GL_TRIANGLES, 0, m_objet[i * j + j][(k_frame+(i * j + j))%23].vertex_count);

                //m_objet[i*j+j].draw(m_groups[k].first, m_groups[k].n, m_program, /* use position */ true, /* use texcoord */ true, /* use normal */ false, /* use color */ false, /* use material index*/ false);

                glBindTexture(GL_TEXTURE_2D, 0);
                glBindSampler(0, 0);
                glUseProgram(0);
                glBindVertexArray(0);
            }
        }
    }

    void render_indirect(){

        /**************************************************************************************
        *       Render avec 2 passes et affichage du blitframebuffer avec texture color       *
        ***************************************************************************************/


        float my_time = global_time()/3; 

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
        glClearColor(1, 1, 0, 1);

        // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer
        int location;
        glUseProgram(m_program2);


        for (int i = 0; i < l; i++)
        {
            for (int j = 0; j < w; j++)
            {


                m_model[i * j + j] = Translation(8 * i, 0, 8 * j);
                //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
                Transform view = m_framebuffer_camera.view();
                Transform projection = m_framebuffer_camera.projection(window_width(), window_height(), 45);
                Transform mv = view * m_model[i * j + j];
                Transform mvp = projection * view * m_model[i * j + j];

                //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
                location = glGetUniformLocation(m_program2, "mvpMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
                //program_uniform(m_program, "mvMatrix", mv.buffer());
                location = glGetUniformLocation(m_program2, "mvMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

                //program_uniform(m_program2, "normalMatrix", mv.normal());
                location = glGetUniformLocation(m_program2, "view");
                glUniformMatrix4fv(location, 1, GL_TRUE, view.buffer());

                location = glGetUniformLocation(m_program2, "model");
                glUniformMatrix4fv(location, 1, GL_TRUE, m_model[i * j + j].buffer());

                location = glGetUniformLocation(m_program2, "projection");
                glUniformMatrix4fv(location, 1, GL_TRUE, projection.buffer());

                //program_uniform(m_program, "temps", );
                location = glGetUniformLocation(m_program2, "temps");

                glUniform1f(location, ((float)((int)(frame_s * my_time) % 1000)) / 1000);

                location = glGetUniformLocation(m_program2, "materials");
                glUniform4fv(location, m_colors.size(), &m_colors[0].r);

                int k_frame = (int)(frame_s*my_time / 1000) % frame_s; //(int) (time)%(frame_s*1000)/1000;
 
                glBindVertexArray(m_objet[i * j + j][(k_frame % 23)].vao);
                glDrawArrays(GL_TRIANGLES, 0, m_objet[i * j + j][(k_frame % 23)].vertex_count);

            }
        }

        std::cout<<" r "<<std::endl;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());

        glClearColor(0.1, 0.1, 0.1, 1);
        // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer


        glUseProgram(m_program_quad);
        glBindVertexArray(quad_VertexArrayID);

        program_use_texture(m_program_quad, "color_texture", 0, m_color_buffer, sampler);
        program_use_texture(m_program_quad, "position_texture", 1, m_position_buffer, sampler);
        program_use_texture(m_program_quad, "normal_texture", 2, m_normal_buffer, sampler);
        program_use_texture(m_program_quad, "material_texture", 3, m_material_buffer, sampler);
        program_use_texture(m_program_quad, "z_texture", 4, m_depth_buffer, sampler);


        glDrawArrays(GL_TRIANGLES, 0, 2*3);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindSampler(0, 0);
        glUseProgram(0);
        glBindVertexArray(0);



    }


    void render_frambuffer(){

        /***********************************************************************************************
        *       Render avec 2 passes et affichage de la deuxième phase avec la texture color....       *
        ************************************************************************************************/

        float my_time = global_time()/3; 

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
        glClearColor(1, 1, 0, 1);

        // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer
        int location;
        glUseProgram(m_program2);


        for (int i = 0; i < l; i++)
        {
            for (int j = 0; j < w; j++)
            {


                m_model[i * j + j] = Translation(8 * i, 0, 8 * j);
                //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
                Transform view = m_framebuffer_camera.view();
                Transform projection = m_framebuffer_camera.projection(window_width(), window_height(), 45);
                Transform mv = view * m_model[i * j + j];
                Transform mvp = projection * view * m_model[i * j + j];

                //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
                location = glGetUniformLocation(m_program2, "mvpMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
                //program_uniform(m_program, "mvMatrix", mv.buffer());
                location = glGetUniformLocation(m_program2, "mvMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

                //program_uniform(m_program2, "normalMatrix", mv.normal());
                location = glGetUniformLocation(m_program2, "view");
                glUniformMatrix4fv(location, 1, GL_TRUE, view.buffer());

                location = glGetUniformLocation(m_program2, "model");
                glUniformMatrix4fv(location, 1, GL_TRUE, m_model[i * j + j].buffer());

                location = glGetUniformLocation(m_program2, "projection");
                glUniformMatrix4fv(location, 1, GL_TRUE, projection.buffer());

                //program_uniform(m_program, "temps", );
                location = glGetUniformLocation(m_program2, "temps");

                glUniform1f(location, ((float)((int)(frame_s * my_time) % 1000)) / 1000);

                location = glGetUniformLocation(m_program2, "materials");
                glUniform4fv(location, m_colors.size(), &m_colors[0].r);

                int k_frame = (int)(frame_s*my_time / 1000) % frame_s; //(int) (time)%(frame_s*1000)/1000;

                glBindVertexArray(m_objet[i * j + j][(k_frame % 23)].vao);
                glDrawArrays(GL_TRIANGLES, 0, m_objet[i * j + j][(k_frame % 23)].vertex_count);

            }
        }

        std::cout<<" espace "<<std::endl;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlitFramebuffer(
            0, 0, m_framebuffer_width, m_framebuffer_height, // rectangle origine dans READ_FRAMEBUFFER
            0, 0, m_framebuffer_width, m_framebuffer_height, // rectangle destination dans DRAW_FRAMEBUFFER
            GL_COLOR_BUFFER_BIT, GL_LINEAR);                 // ne copier que la couleur (+ interpoler)



    }


    void render_redect_and_cube(){

        /***********************************************************************************************
        *       Render avec 2 passes et affichage de la première phase sur les faces d'un cube         *
        ************************************************************************************************/

        float my_time = 2*global_time(); 

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
        //glClearColor(1, 1, 0, 1);
        glClearColor(0.1, 0.1, 0.1, 1);

        // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer
        int location;
        glUseProgram(m_program2);


        for (int i = 0; i < l; i++)
        {
            for (int j = 0; j < w; j++)
            {


                m_model[i * j + j] = Translation(8 * i, 0, 8 * j);
                //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
                Transform view = m_framebuffer_camera.view();
                Transform projection = m_framebuffer_camera.projection(window_width(), window_height(), 45);
                Transform mv = view * m_model[i * j + j];
                Transform mvp = projection * view * m_model[i * j + j];

                //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
                location = glGetUniformLocation(m_program2, "mvpMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
                //program_uniform(m_program, "mvMatrix", mv.buffer());
                location = glGetUniformLocation(m_program2, "mvMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

                //program_uniform(m_program2, "normalMatrix", mv.normal());
                location = glGetUniformLocation(m_program2, "view");
                glUniformMatrix4fv(location, 1, GL_TRUE, view.buffer());

                location = glGetUniformLocation(m_program2, "model");
                glUniformMatrix4fv(location, 1, GL_TRUE, m_model[i * j + j].buffer());

                location = glGetUniformLocation(m_program2, "projection");
                glUniformMatrix4fv(location, 1, GL_TRUE, projection.buffer());

                //program_uniform(m_program, "temps", );
                location = glGetUniformLocation(m_program2, "temps");

                location = glGetUniformLocation(m_program2, "materials");
                glUniform4fv(location, m_colors.size(), &m_colors[0].r);

                int k_frame = (int)(frame_s * my_time / 1000) % frame_s; //(int) (time)%(frame_s*1000)/1000;
                //std::cout<<"k_frame first"<<k_frame<<std::endl;

                //k_frame = 3;(i * j + j)
                glBindVertexArray(m_objet[i * j + j][(k_frame % 23)].vao);
                glDrawArrays(GL_TRIANGLES, 0, m_objet[i * j + j][(k_frame  % 23)].vertex_count);

                //m_objet[i*j+j].draw(m_groups[k].first, m_groups[k].n, m_program, /* use position */ true, /* use texcoord */ true, /* use normal */ false, /* use color */ false, /* use material index*/ false);
            }
        }

        std::cout<<" rien "<<std::endl;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());

        glClearColor(0.2f, 0.2f, 0.2f, 1.f); // couleur par defaut de la fenetre

        // on commence par effacer la fenetre avant de dessiner quelquechose
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // on efface aussi le zbuffer

        glUseProgram(m_program);

        for (int i = 0; i < l_2; i++)
        {
            for (int j = 0; j < w_2; j++)
            {

                m_model_2_cube[i * j + j] = Translation(2 * i, 0, 2 * j);
                //draw(m_objet[i*j+j], m_model[i*j+j], m_camera, texture);
                Transform view = m_camera.view();
                Transform projection = m_camera.projection(window_width(), window_height(), 45);
                Transform mv = view * m_model_2_cube[i * j + j];
                Transform mvp = projection * view * m_model_2_cube[i * j + j];

                //  . transformation : la matrice declaree dans le vertex shader s'appelle mvpMatrix
                location = glGetUniformLocation(m_program, "mvpMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
                //program_uniform(m_program, "mvMatrix", mv.buffer());
                location = glGetUniformLocation(m_program, "mvMatrix");
                glUniformMatrix4fv(location, 1, GL_TRUE, mv.buffer());

                program_uniform(m_program, "normalMatrix", mv.normal());
                location = glGetUniformLocation(m_program, "view");
                glUniformMatrix4fv(location, 1, GL_TRUE, view.buffer());

                location = glGetUniformLocation(m_program, "model");
                glUniformMatrix4fv(location, 1, GL_TRUE, m_model[i * j + j].buffer());

                location = glGetUniformLocation(m_program, "projection");
                glUniformMatrix4fv(location, 1, GL_TRUE, projection.buffer());

                //program_uniform(m_program, "temps", );
                location = glGetUniformLocation(m_program, "temps");

                location = glGetUniformLocation(m_program, "materials");
                glUniform4fv(location, m_colors.size(), &m_colors[0].r);

                program_use_texture(m_program, "color_texture", 0, m_color_buffer, sampler);

                /**ou
                utilise la texture attachee au framebuffer
                program_uniform(m_program, "color_texture", 0); // utilise la texture configuree sur l'unite 0

                // configure l'unite 0
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_color_buffer);
                glBindSampler(0, sampler);
                **/

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, miplevels(m_framebuffer_width, m_framebuffer_height));
                glGenerateMipmap(GL_TEXTURE_2D);

                program_use_texture(m_program, "profondeur_texture", 1, m_depth_buffer, sampler);


                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, miplevels(m_framebuffer_width, m_framebuffer_height));
                glGenerateMipmap(GL_TEXTURE_2D);

                glBindVertexArray(m_objet_2_cube[i * j + j].vao);
                glDrawArrays(GL_TRIANGLES, 0, m_objet_2_cube[i * j + j].vertex_count);
            }
        }


    glBindTexture(GL_TEXTURE_2D, 0);
    glBindSampler(0, 0);
    glUseProgram(0);
    glBindVertexArray(0);




    }

    // dessiner une nouvelle image
    int render()
    {
        // etape 2 : dessiner l'm_objet avec opengl

        // recupere les mouvements de la souris, utilise directement SDL2
        int mx, my;
        unsigned int mb = SDL_GetRelativeMouseState(&mx, &my);

        // deplace la m_camera
        if (mb & SDL_BUTTON(1))
        { // le bouton gauche est enfonce
            // tourne autour de l'm_objet
            m_camera.rotation(mx, my);
            m_framebuffer_camera.rotation(mx, my);
            m_camera_origin.rotation(mx, my);
        }
        else if (mb & SDL_BUTTON(3))
        { // le bouton droit est enfonce
            // approche / eloigne l'm_objet
            m_camera.move(mx);
            m_framebuffer_camera.move(mx);
            m_camera_origin.move(mx);
        }
        else if (mb & SDL_BUTTON(2))
        { // le bouton du milieu est enfonce
            // deplace le point de rotation
            m_camera.translation((float)mx / (float)window_width(), (float)my / (float)window_height());
            m_framebuffer_camera.translation((float)mx / (float)window_width(), (float)my / (float)window_height());
            m_camera_origin.translation((float)mx / (float)window_width(), (float)my / (float)window_height());
        }


        // mesure le temps d'execution du draw pour le gpu
        glBeginQuery(GL_TIME_ELAPSED, m_time_query);

        // mesure le temps d'execution du draw pour le cpu
        // utilise std::chrono pour mesurer le temps cpu
        std::chrono::high_resolution_clock::time_point cpu_start = std::chrono::high_resolution_clock::now();


        static int mode= 0;
        if(key_state(' '))
        {
            clear_key_state(' ');
            mode= (mode + 1) % 4;
        }


        if(mode == 0)
        {
            render_origin();
        }
        else if(mode == 1){
            render_frambuffer();
        }
        else if(mode == 2){
            render_indirect();
        }
        else if(mode == 3){
            render_redect_and_cube();
        }



        std::chrono::high_resolution_clock::time_point cpu_stop = std::chrono::high_resolution_clock::now();
        // conversion des mesures en duree...
        int cpu_time = std::chrono::duration_cast<std::chrono::microseconds>(cpu_stop - cpu_start).count();

        glEndQuery(GL_TIME_ELAPSED);

        /* recuperer le resultat de la requete time_elapsed, il faut attendre que le gpu ait fini de dessiner...
        utilise encore std::chrono pour mesurer le temps d'attente.
        */
        std::chrono::high_resolution_clock::time_point wait_start = std::chrono::high_resolution_clock::now();

        // attendre le resultat de la requete
        GLint64 gpu_time = 0;
        glGetQueryObjecti64v(m_time_query, GL_QUERY_RESULT, &gpu_time);

        std::chrono::high_resolution_clock::time_point wait_stop = std::chrono::high_resolution_clock::now();
        int wait_time = std::chrono::duration_cast<std::chrono::microseconds>(wait_stop - wait_start).count();

        // affiche le temps mesure, et formate les valeurs... c'est un peu plus lisible.
        clear(m_console);
        if(mode == 0) printf(m_console, 0, 0, "mode 0 : Affichage Direct");
        if(mode == 1) printf(m_console, 0, 0, "mode 1 : Affichage Framebuffer");
        if(mode == 2) printf(m_console, 0, 0, "mode 2 : Affichage Indirect");
        if(mode == 3) printf(m_console, 0, 0, "mode 3 : Affichage Direct et cubes");
        printf(m_console, 0, 1, "cpu  %02dms %03dus", int(cpu_time / 1000), int(cpu_time % 1000));
        printf(m_console, 0, 2, "gpu  %02dms %03dus", int(gpu_time / 1000000), int((gpu_time / 1000) % 1000));
        printf(m_console, 0, 3, "wait %02dms %03dus", int(wait_time / 1000), int(wait_time % 1000));

        // affiche le texte dans la fenetre de l'application, utilise console.h
        draw(m_console, window_width(), window_height());

        // affiche le temps dans le terminal
        //printf("cpu  %02dms %03dus  ", int(cpu_time / 1000), int(cpu_time % 1000));
        //printf("gpu  %02dms %03dus\n", int(gpu_time / 1000000), int((gpu_time / 1000) % 1000));

        return 1;
    }

protected:
    GLuint m_time_query;
    Text m_console;



    const static int frame_s = 23;

    //robots
    const static int l = 3;
    const static int w = 3;
    Transform m_model[l * w];
    Buffers m_objet[l * w][frame_s];

    //on texture cube
    const static int l_2 = 3;
    const static int w_2 = 3;
    Transform m_model_2_cube[l_2 * w_2];
    Buffers m_objet_2_cube[l_2 * w_2];


    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint quad_VertexArrayID;
    GLuint quad_vertexbuffer;

    Orbiter m_camera;
    Orbiter m_framebuffer_camera;
    Orbiter m_camera_origin;

    int m_framebuffer_width;
    int m_framebuffer_height;


    GLuint m_position_buffer;
    GLuint m_normal_buffer;
    GLuint m_material_buffer;
    GLuint m_color_buffer;
    GLuint m_depth_buffer;
    GLuint m_framebuffer;

    GLuint m_texture0;
    GLuint m_texture1;

    GLuint sampler;
    std::vector<Color> m_colors;
    GLuint m_program;
    GLuint m_program2;
    GLuint m_program_origin;
    GLuint m_program_quad;
};

int main(int argc, char **argv)
{
    TP tp;
    tp.run();

    return 0;
}
