#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader
{
    public:
        unsigned int ID;
    
        Shader(std::string vertex_path, std::string fragment_path);
        Shader(std::string vertex_path, std::string geometry_path, std::string fragment_path);
    
        void use();
    
    private:
        unsigned int load_shader(std::string path, GLenum type);
};

#endif
