#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <signal.h>

#define PI 3.141592653589793
#define FOV (45.0*PI/180.0)
#define TIMER_SLEEP 10
#define FFT_TEXTURE_WIDTH (CHUNK/4)
#define FFT_TEXTURE_HEIGHT 1024
#define numCubes MAX_SAMPLES
#define dist -20.0

#include "linmath.h"
#include "glutil.h"
#include "pamon.h"

GLuint textureID;

GLuint cubeVertexBuffer;
GLuint cubeIndexBuffer;
GLuint skyVertexBuffer;
GLuint skyIndexBuffer;

GLuint cubeOffsetBuffer;

GLenum pBackground; // main program for background shaders
GLenum pCube; // main program for background shaders
GLenum pSkybox; // main program for background shaders

GLuint currentAudioTextureOffset = 0;
GLfloat pixels[FFT_TEXTURE_WIDTH*FFT_TEXTURE_HEIGHT]; // global storage for texture data
vec3 window = {640.01f, 480.01f, 0.0f}; // x,y there is no vec2
float tick = 0.0; // msec/1000.0
SDL_Window *screen;
SDL_Event event;
GLfloat offsets[numCubes*3] = {0};

#include "camera.h"

void updateAudioTexture(void)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Fill a texture with audio wave data
    im_getSnapshot();
    GLfloat* mfft = magnitudeFFT;
    GLfloat* mraw = magnitudeRaw;
    int i;
    float ghostDiff = 0.025F;

    currentAudioTextureOffset++;
    currentAudioTextureOffset = currentAudioTextureOffset % FFT_TEXTURE_HEIGHT;
    for (i=0;i<FFT_TEXTURE_WIDTH;i++) {
        if (i<MAX_SAMPLES) {
            // first row fill with a filtered fft
            GLint offset = currentAudioTextureOffset*FFT_TEXTURE_WIDTH+i;
            pixels[offset] = mfft[i];
                // pixels[offset]*(GLfloat)0.75 
                //     + mfft[i]*(GLfloat)0.25;
            // // second row holds max fft values that decay by ghostDiff
            // if (pixels[i] > pixels[i+FFT_TEXTURE_WIDTH]-ghostDiff)
            //     pixels[i+FFT_TEXTURE_WIDTH] = pixels[i];
            // else
            //     pixels[i+FFT_TEXTURE_WIDTH] -= ghostDiff;
        }
        // // third row holds raw wave
        // pixels[i+FFT_TEXTURE_WIDTH*2] = mraw[i];
    }
            
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FFT_TEXTURE_WIDTH, FFT_TEXTURE_HEIGHT, GL_RED, GL_FLOAT, pixels);
    glActiveTexture(GL_TEXTURE0);
}

void render (void)
{
    // You need to clear the color buffer and depth
    glClear(GL_DEPTH_BUFFER_BIT); // GL_COLOR_BUFFER_BIT | 

    tick = SDL_GetTicks()/1000.0;
    
    updateAudioTexture(); // audio data converted to a texture

    mat4 a = {0};
    mat4 projectionMatrix = {0};
    mat4 rotateMatrix = {0};
    mat4 viewMatrix = {0};

    mouse_rot[1] += cos(tick/2.8)*0.0001;
    mouse_rot[0] += sin((tick/10.0)*sin(tick/100.0))*0.00008;
    cam_look_pos[0] += cos(tick/15.0);
    cam_look_pos[1] += tan(tick/60.0);
    updateCamera();

    GLfloat vertices[] = {
        +0.5f, +0.5f, +0.5f,
        -0.5f, +0.5f, +0.5f,
        -0.5f, -0.5f, +0.5f,
        +0.5f, -0.5f, +0.5f,

        +0.5f, -0.5f, -0.5f,
        +0.5f, +0.5f, -0.5f,
        -0.5f, +0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f
    };
    GLushort indices[] = {
        0, 1, 2,  2, 3, 0,
        0, 3, 4,  4, 5, 0,
        0, 5, 6,  6, 1, 0,
        1, 6, 7,  7, 2, 1,
        7, 4, 3,  3, 2, 7,
        4, 7, 6,  6, 5, 4
    };

    GLushort indicesSky[] = {
        2, 1, 0,  0, 3, 2,
        4, 3, 0,  0, 5, 4,
        6, 5, 0,  0, 1, 6,
        7, 6, 1,  1, 2, 7,
        3, 4, 7,  7, 2, 3,
        6, 7, 4,  4, 5, 6
    };


    // mat4_identity(a);
    mat4_identity(rotateMatrix);
    mat4_perspective(projectionMatrix, FOV, window[0]/window[1], 1.0, 2000.0);
    // mat4_rotate(rotateMatrix, a, 0.0, 1.0, 0.0, sin(tick/60.0)*0.1);
    mat4_look_at(viewMatrix, cam_position, cam_look_at, cam_up);

    // // // Draw a Visualizer
    // glUseProgramObjectARB(pBackground);
    // glUniform1fARB(glGetUniformLocationARB(pBackground, "tick"), tick);
    // glUniform3fvARB(glGetUniformLocationARB(pBackground, "win"), 1, &window[0]);
    // glProgramUniform1i(pBackground, glGetUniformLocationARB(pCube, "texUnit"), 0); // 0 means GL_TEXTURE0
    // glBegin(GL_TRIANGLES);
    //     glVertex3f(+1.0f, +1.0f, +1.0f);
    //     glVertex3f(-1.0f, +1.0f, +1.0f);
    //     glVertex3f(-1.0f, -1.0f, +1.0f);
    //     glVertex3f(-1.0f, -1.0f, +1.0f);
    //     glVertex3f(+1.0f, -1.0f, +1.0f);
    //     glVertex3f(+1.0f, +1.0f, +1.0f);
    // glEnd();

    glBindBufferARB(GL_ARRAY_BUFFER, skyVertexBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glProgramUniform1i(pCube, glGetUniformLocationARB(pSkybox, "texUnit"), 0); // 0 means GL_TEXTURE0
    
    glVertexAttribPointerARB(glGetAttribLocationARB(pSkybox, "verts"), 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, 0);
    glEnableVertexAttribArrayARB(glGetAttribLocationARB(pSkybox, "verts"));
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, skyIndexBuffer);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesSky), indicesSky, GL_STATIC_DRAW);
    // draw Skybox
    glUseProgramObjectARB(pSkybox);
    glUniform1iARB(glGetUniformLocationARB(pSkybox, "texOffset"), currentAudioTextureOffset);
    glUniform1fARB(glGetUniformLocationARB(pSkybox, "tick"), tick);
    glUniformMatrix4fvARB(glGetUniformLocationARB(pSkybox, "viewMatrix"), 1, 
                 GL_FALSE, (const GLfloat *)&viewMatrix[0]);
    glUniformMatrix4fvARB(glGetUniformLocationARB(pSkybox, "projectionMatrix"), 1, 
                 GL_FALSE, (const GLfloat *)&projectionMatrix[0]);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

    glBindBufferARB(GL_ARRAY_BUFFER, 0);

    glBindBufferARB(GL_ARRAY_BUFFER, cubeVertexBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointerARB(glGetAttribLocationARB(pCube, "vert"), 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, 0);
    glEnableVertexAttribArrayARB(glGetAttribLocationARB(pCube, "vert"));

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // draw a cube
    glUseProgramObjectARB(pCube);
    glUniform3fvARB(glGetUniformLocationARB(pCube, "win"), 1, &window[0]);
    glUniform1fARB(glGetUniformLocationARB(pCube, "tick"), tick);
    glProgramUniform1i(pCube, glGetUniformLocationARB(pCube, "texUnit"), 0); // 0 means GL_TEXTURE0
    glUniformMatrix4fvARB(glGetUniformLocationARB(pCube, "rotateMatrix"), 1, 
                GL_FALSE, (const GLfloat *)&rotateMatrix[0]);
    glUniformMatrix4fvARB(glGetUniformLocationARB(pCube, "viewMatrix"), 1, 
                 GL_FALSE, (const GLfloat *)&viewMatrix[0]);
    glUniformMatrix4fvARB(glGetUniformLocationARB(pCube, "projectionMatrix"), 1, 
                 GL_FALSE, (const GLfloat *)&projectionMatrix[0]);



    int i;
    for(i=0;i<numCubes;i++) {
        offsets[i*3+0] = (i%MAX_SAMPLES)-(MAX_SAMPLES/2.0)*1.0;
        offsets[i*3+1] = offsets[i*3+1]*0.85 + 0.15*pixels[i+currentAudioTextureOffset*FFT_TEXTURE_WIDTH]*20.0;
        offsets[i*3+2] = 0.0;
    }

    glBindBufferARB(GL_ARRAY_BUFFER, cubeOffsetBuffer);
    glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GLfloat)*numCubes*3, offsets, GL_STREAM_DRAW);

    GLuint attrib = glGetAttribLocationARB(pCube, "offset");
    glVertexAttribPointerARB(attrib, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (void*)(sizeof(float)*0));
    glEnableVertexAttribArrayARB(attrib);
    glVertexAttribDivisorARB(attrib,1);

    glDrawElementsInstancedARB(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0, numCubes);
    glBindBufferARB(GL_ARRAY_BUFFER, 0);

    SDL_GL_SwapWindow(screen);
}

void createTextures(void)
{
    int width = FFT_TEXTURE_WIDTH;
    int height = FFT_TEXTURE_HEIGHT;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height); // 32bit float only red channel
}

void createBuffers(void)
{
    glGenBuffersARB(1, &cubeVertexBuffer);
    glGenBuffersARB(1, &cubeIndexBuffer);
    glGenBuffersARB(1, &skyIndexBuffer);
    glGenBuffersARB(1, &skyVertexBuffer);
    glGenBuffersARB(1, &cubeOffsetBuffer);
}

void createShaders(void)
{
    char *fragmentSrc, *vertexSrc;

    printf("Compiling shaders..\n");

    // Skybox
    pSkybox = glCreateProgramObjectARB();
    int len = 0;
    assert((len = open_shader("skybox.frs", &fragmentSrc)));
    GLenum skyShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    if (!compile_shader(skyShaderObject, fragmentSrc, &len)) {
        fprintf(stderr, "Could not compile skybox fragment shader\n");
        free(fragmentSrc);
        exit(1);
    }
    // fprintf(stdout, "Compiled skybox fragment shader\n");
    glAttachObjectARB(pSkybox, skyShaderObject);
    len = 0;
    assert((len = open_shader("skybox.vs", &vertexSrc)));
    GLenum skyVertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    if (!compile_shader(skyVertexShaderObject, vertexSrc, &len)) {
        fprintf(stderr, "Could not compile skybox vertex shader\n");
        free(vertexSrc);
        exit(1);
    }
    // fprintf(stdout, "Compiled skybox vertex shader\n");
    glAttachObjectARB(pSkybox, skyVertexShaderObject);
    glLinkProgramARB(pSkybox);

    free(vertexSrc);
    free(fragmentSrc);
    glDeleteShader(skyVertexShaderObject);
    glDeleteShader(skyShaderObject);

    // Background
    pBackground = glCreateProgramObjectARB();
    len = 0;
    assert((len = open_shader("visualizer.frs", &fragmentSrc)));
    GLenum fragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    if (!compile_shader(fragmentShaderObject, fragmentSrc, &len)) {
        fprintf(stderr, "Could not compile background fragment shader\n");
        free(fragmentSrc);
        exit(1);
    }
    // fprintf(stdout, "Compiled bg fragment shader\n");
    glAttachObjectARB(pBackground, fragmentShaderObject);
    glLinkProgramARB(pBackground);

    free(fragmentSrc);
    glDeleteShader(fragmentShaderObject);

    // Cube
    pCube = glCreateProgramObjectARB();
    fragmentSrc = NULL;
    len = 0;
    assert((len = open_shader("box.frs", &fragmentSrc)));
    GLenum boxShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    if (!compile_shader(boxShaderObject, fragmentSrc, &len)) {
        fprintf(stderr, "Could not compile cube fragment shader\n");
        free(fragmentSrc);
        exit(1);
    }
    // fprintf(stdout, "Compiled cube fragment shader\n");
    glAttachObjectARB(pCube, boxShaderObject);
    len = 0;
    assert((len = open_shader("box.vs", &vertexSrc)));
    GLenum boxVShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    if (!compile_shader(boxVShaderObject, vertexSrc, &len)) {
        fprintf(stderr, "Could not compile cube vertex shader\n");
        free(vertexSrc);
        exit(1);
    }
    // fprintf(stdout, "Compiled cube vertex shader\n");
    glAttachObjectARB(pCube, boxVShaderObject);

    glLinkProgramARB(pCube);

    free(vertexSrc);
    free(fragmentSrc);
    glDeleteShader(boxShaderObject);
    glDeleteShader(boxVShaderObject);
}

void reshape(int w, int h)
{
    // vec3 window (float[3])
    window[0] = w+0.001;
    window[1] = h+0.001; // professional float handling
    window[2] = 0.0f;
    SDL_SetWindowSize(screen, w, h);
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void initWindow (int *argc, char **argv)
{
    printf("Initializing window..\n");
    if(SDL_Init(SDL_INIT_VIDEO)<0) {
        fprintf(stderr, "Could not initialize SDL\n");
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // enable multisample anti-aliasing. This depends on video cards abilties
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // low quality AA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16); // high quality AA

    screen = SDL_CreateWindow("visualsr", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!screen) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(screen);

    if (!glContext) {
        fprintf(stderr, "Failed to create Opengl context\n");
        exit(1);
    }

    GLenum err = glewInit();
    if (err)
        fprintf(stderr, "Error initializing glew\n");
    fprintf(stdout, "Using GLEW ver %s\n", glewGetString(GLEW_VERSION));

    if (SDL_GL_SetSwapInterval(1)<0) {
        printf("Unable to set VSync\n");
    }

    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH); 

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Enable back face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    glEnable(GL_MULTISAMPLE_ARB); // anti aliasing

    glClearColor(1.0, 1.0, 1.0, 1.0);

    updateCamera();
}

void cleanup(void)
{
    quit_pamon(0);
    glDeleteProgram(pCube);
    glDeleteProgram(pBackground);
    glDeleteTextures(1, &textureID); // not necessary
    glDeleteBuffers(1, &cubeOffsetBuffer);
    glDeleteBuffers(1, &cubeIndexBuffer);
    glDeleteBuffers(1, &skyVertexBuffer);
    glDeleteBuffers(1, &skyIndexBuffer);
    glDeleteBuffers(1, &cubeVertexBuffer);
    SDL_Quit();
    fprintf(stderr, "Successfully cleaned up!\n");
}

void sighandle(int sig)
{
    cleanup();
    exit(0);
}

void process_events()
{
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_KEYUP:
                keyboardInput(event.key.keysym.scancode, SDL_KEYUP);
                break;
            case SDL_TEXTINPUT:
                break;
            case SDL_KEYDOWN:
                keyboardInput(event.key.keysym.scancode, SDL_KEYDOWN);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    reshape(event.window.data1, event.window.data2);
                break;
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_MOUSEWHEEL:
                break;
            case SDL_MOUSEBUTTONDOWN:
                // if (event.button.button & SDL_BUTTON_LMASK) {
                    SDL_ShowCursor(SDL_FALSE);
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    oldx = event.button.x;
                    oldy = event.button.y;
                // }
                break;
            case SDL_MOUSEBUTTONUP:
                // if (event.button.button & SDL_BUTTON_LMASK) {
                    SDL_ShowCursor(SDL_TRUE);
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    // warp the mouse somewhere nice
                    SDL_WarpMouseInWindow(screen, oldx, oldy);
                // }
                break;
            case SDL_MOUSEMOTION:
                if(event.motion.state) { // & SDL_BUTTON_LMASK
                    mouseMotion(event.motion.xrel, event.motion.yrel);
                }
                break;
            default:
                printf("Unhandled event: %i\n", event.type);
                break;
        }
    }
}

int main(int argc, char **argv)
{
    atexit(cleanup);
    signal(SIGINT, sighandle);
    signal(SIGTERM, sighandle);

    initWindow(&argc, argv);
    createShaders();
    createTextures();
    createBuffers();
    initPAMon();

    for(;;) {
        process_events();
        updateCamera();
        render();
    }

    cleanup();
}
