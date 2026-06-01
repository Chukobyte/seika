#include "ska_opengl.h"

#include "seika/logger.h"

void ska_opengl_print_errors(const char* context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (context) {
            ska_logger_message("context = %s", context);
        }
        ska_logger_message("err = %d", err);
        switch (err) {
            case GL_NO_ERROR:
                ska_logger_message("GL_NO_ERROR");
                break;
            case GL_INVALID_ENUM:
                ska_logger_message("GL_INVALID_ENUM");
                break;
            case GL_INVALID_VALUE:
                ska_logger_message("GL_INVALID_VALUE");
                break;
            case GL_INVALID_OPERATION:
                ska_logger_message("GL_INVALID_OPERATION");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                ska_logger_message("GL_INVALID_FRAMEBUFFER_OPERATION");
                break;
            default:
                ska_logger_message("default");
                break;
        }
    }
}