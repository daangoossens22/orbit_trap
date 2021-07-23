#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "camera.h"

Camera::Camera(float screen_width, float screen_heigth)
{
    // for the mandelbrot starting range is; x: [-2.5, 1.0], y: [-1.0, 1.0]
    glm::mat3 scale_matrix = glm::scale(glm::mat3(1.0f), glm::vec2(3.5f / screen_width, 2.0f / screen_heigth));
    glm::mat3 translate_matrix = glm::translate(glm::mat3(1.0f), glm::vec2(-2.5f, -1.0f));
    pixel_to_mandel = translate_matrix * scale_matrix * glm::mat3(1.0f);
}

glm::vec2 Camera::convert_pixel_to_mandel(double xpos, double ypos)
{
    glm::vec3 mouse_pixel = glm::vec3(xpos, ypos, 1.0f);
    glm::vec3 mouse_mandel = pixel_to_mandel * mouse_pixel;
    mouse_mandel /= mouse_mandel.z;
    return glm::vec2(mouse_mandel.x, mouse_mandel.y);
}

void Camera::move_camera_matrix_to_gpu(unsigned int shader_id)
{
    unsigned int uniform_buf = glGetUniformLocation(shader_id, "pixel_to_mandel");
    glUniformMatrix3fv(uniform_buf, 1, GL_FALSE, glm::value_ptr(pixel_to_mandel));
}

void Camera::drag_start(double xpos, double ypos)
{
    prev_cursor_x = xpos;
    prev_cursor_y = ypos;

    drag_started = true;
}

void Camera::drag_end()
{
    drag_started = false;
}

void Camera::drag_update(double xpos, double ypos)
{
    if (drag_started)
    {
        double xdiff = prev_cursor_x - xpos;
        double ydiff = prev_cursor_y - ypos;
        // only works because only scaling and translations are used
        xdiff *= pixel_to_mandel[0][0];
        ydiff *= pixel_to_mandel[1][1];

        prev_cursor_x = xpos;
        prev_cursor_y = ypos;

        glm::mat3 trans = glm::translate(glm::mat3(1.0f), glm::vec2(xdiff, ydiff));
        pixel_to_mandel = trans * pixel_to_mandel;
    }
}

void Camera::zoom_at_point(double xpos, double ypos, double yoffset)
{
    glm::vec2 mouse_mandel = convert_pixel_to_mandel(xpos, ypos);
    
    float temp_scale = (yoffset > 0) ? scale_factor : 1.0 / scale_factor;

    // move mouse to origin
    glm::mat3 trans = glm::translate(glm::mat3(1.0f), glm::vec2(-mouse_mandel.x, -mouse_mandel.y));
    // scale
    glm::mat3 scale = glm::scale(glm::mat3(1.0f), glm::vec2(temp_scale));
    // move back
    glm::mat3 trans_back = glm::translate(glm::mat3(1.0f), glm::vec2(mouse_mandel.x, mouse_mandel.y));
    // apply to pixel_to_mandel
    pixel_to_mandel = trans_back * scale * trans * pixel_to_mandel;
}
