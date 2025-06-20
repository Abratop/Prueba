#ifndef GLAD_H
#define GLAD_H

// Minimal GLAD stub
typedef void (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc loader);

// Define some common OpenGL constants/types to satisfy includes if they are used directly
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FALSE 0
#define GL_TRUE 1
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef float GLfloat;

// Minimal function definition to avoid linking errors if called directly by ImGui backends in a stub context
#ifdef __cplusplus
extern "C" {
#endif
void glClear(GLenum mask);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glViewport(int x, int y, int width, int height);
const char* glGetString(GLenum name); // ImGui might call this
#ifdef __cplusplus
}
#endif


#endif // GLAD_H
