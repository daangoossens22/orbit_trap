class Shader
{
    public:
        unsigned int ID;
    
        Shader(const char* vertex_path, const char* fragment_path);
        Shader(const char* vertex_path, const char* geometry_path, const char* fragment_path);
    
        void use();
    
    private:
        unsigned int load_shader(std::string path, GLenum type);
};
