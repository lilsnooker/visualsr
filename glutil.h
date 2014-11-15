#ifndef GLUTIL_H
#define GLUTIL_H
static inline
int
open_shader(char* filename, char **target)
{
    int len;
    SDL_RWops *f = SDL_RWFromFile(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "Error: could not open shader file (%s)\n", filename);
        goto FAIL;
    }
    len = f->seek(f, 0, RW_SEEK_END);
    f->seek(f, 0, RW_SEEK_SET);
    *target = calloc(sizeof(char), len+1);
    if (!*target) {
        fprintf(stderr, "Error: couldnt calloc shader\n");
        goto FAIL;
    }
    if (f->read(f, *target, len, 1) != 1) {
        fprintf(stderr, "Error: Failed reading file\n");
        goto FAIL;
    }

    f->close(f);
    return len;

FAIL:
    if(f)
        f->close(f);
    if (*target);
        free(*target);
    exit(1);
}
static inline
int
compile_shader(GLenum shaderObject, char* src, int *len)
{
    glShaderSourceARB(shaderObject, 1, (const GLchar **)&src, len);
    glCompileShaderARB(shaderObject);

    GLint isCompiled = 0;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &maxLength);
        char * logData = NULL;
        logData = calloc(1, maxLength);
        if (!logData) {
            fprintf(stderr, "Error: could not calloc shader log data\n");
            exit(1);
        }
        glGetShaderInfoLog(shaderObject, maxLength, &maxLength, logData);
        glDeleteShader(shaderObject);
        fprintf(stderr, "Error: Shader could not compile:\n(reason)\n%s\n", logData);
        free(logData);
        return 0;
    }

    return 1;
}
#endif
