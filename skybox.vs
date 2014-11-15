#version 130
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform sampler2D texUnit;

in vec3 verts;

out vec4 pos;
out float dist;
out vec4 tex;

void main (void)
{
    float scale = 1024.0;
    vec3 scaleVec = vec3(scale, scale/2.0, scale);
    tex = vec4(verts*scaleVec, 1.0);

    mat4 newTrans = mat4(1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, scaleVec.y/2.0, 0.0, 1.0);
    gl_Position = projectionMatrix * viewMatrix * newTrans * tex;


    mat4 ident = mat4(1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 0.0
        );
    pos = vec4(verts, 1.0);
    dist = vec4(ident*viewMatrix*newTrans * tex).xyz;
}