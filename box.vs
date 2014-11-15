#version 130
uniform float tick;
uniform vec3 win;
uniform mat4 translateMatrix;
uniform mat4 projectionMatrix;
uniform mat4 rotateMatrix;
uniform mat4 viewMatrix;
uniform mat4 fullMatrix; //model + projection matrices
in vec3 vert;
in vec3 offset; //model + projection matrices

out float vZCoord;
out vec4 vTest;

void main(void) {

    vec4 tex = vec4(vert*vec3(1.0, offset.y*8.0+1.0, 1.0), 1.0);

    mat4 newTrans = mat4(1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    offset.x, 10.0*8.0+1.0, offset.z, 1.0); // offset.y/2.0+0.5

    gl_Position = projectionMatrix * viewMatrix
     * rotateMatrix * newTrans * tex;

    vTest = vec4(vert, 1.0);

    vZCoord = abs(vTest.z);
}
