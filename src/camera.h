#ifndef CAMERA_H
#define CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Camera
{
    public:
        Camera(float screen_width, float screen_heigth);

        glm::vec2 convert_pixel_to_mandel(double xpos, double ypos);

        void move_camera_matrix_to_gpu(unsigned int shader_id);

        void drag_start(double xpos, double ypos);
        void drag_end();
        void drag_update(double xpos, double ypos);

        void zoom_at_point(double xpos, double ypos, double yoffset);
    private:
        glm::mat3 pixel_to_mandel;
        bool drag_started = false;
        double prev_cursor_x, prev_cursor_y;
        const float scale_factor = 0.9;
};


#endif // CAMERA_H
