vec3 cam_position = {0.0f, 1.5f, 70.0};
vec3 cam_look_at = {0.0, 0.0f, 0.0};
vec3 cam_look_pos = {0.0, 1.5, 0.0};
vec3 cam_look_dir = {0.0, 0.0, -1.0};
vec3 cam_up = {0.0f, 1.0f, 0.0f};
vec3 cam_norm = {0.0f, 1.0f, 0.0f};

vec3 mouse_rot = {-PI/12.0f, PI, 0.0f};
int oldx=0, oldy=0;
float lastMove = 0.0;
vec3 cam_velocity = {0.0f};

#define GRAVITY 9.8
#define GRAVITY_MUL 4.0
#define MAX_KEYS 283
#define MOUSE_MAX_CHANGE 30.0
#define MOUSE_SENSITIVITY 400.0
#define JUMP_AMOUNT GRAVITY*1.2
#define TERMINAL_VEL GRAVITY*100.0


float FLOOR_Y = 2.0;

int keys[MAX_KEYS];

void keyboardInput(int code, int state)
{
    if (code > MAX_KEYS) {
        printf("Error: Key exceeded max keys value: %i\n", code);
        return;
    }
    if (state == SDL_KEYUP)
        keys[code] = 0;
    if (state == SDL_KEYDOWN)
        keys[code] = 1;
}

void doPhysics(float msecSinceLastRun)
{
    cam_position[1] += cam_velocity[1]*msecSinceLastRun;
    cam_velocity[1] -= GRAVITY*GRAVITY_MUL*msecSinceLastRun;

    if(cam_velocity[1] < -TERMINAL_VEL) {
        cam_velocity[1] = -TERMINAL_VEL;
    }
    if(cam_position[1] <= FLOOR_Y && cam_velocity[1] < 0.0) {
        cam_position[1] = FLOOR_Y;
        cam_velocity[1] = 0.0;
    }
}

void moveCamera()
{
    const float t = SDL_GetTicks()/1000.0;
    float SPEED_MOD = (17.0+(!keys[SDL_SCANCODE_LSHIFT])*12.0) * (t-lastMove);
    if ((keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) && (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]))
        SPEED_MOD /= 2.0;

    if (keys[SDL_SCANCODE_W]) {
        cam_position[0] += sin(mouse_rot[1]) * SPEED_MOD;
        cam_position[2] += cos(mouse_rot[1]) * SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_S]) {
        cam_position[0] -= sin(mouse_rot[1]) * SPEED_MOD;
        cam_position[2] -= cos(mouse_rot[1]) * SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_A]) {
        cam_position[0] += cos(mouse_rot[1]) * SPEED_MOD;
        cam_position[2] += -sin(mouse_rot[1]) * SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_D]) {
        cam_position[0] -= cos(mouse_rot[1]) * SPEED_MOD;
        cam_position[2] -= -sin(mouse_rot[1]) * SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_E]) {
        FLOOR_Y += SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_Q]) {
        FLOOR_Y -= SPEED_MOD;
    }
    if (keys[SDL_SCANCODE_R]) {
        FLOOR_Y = 2.0;
    }

    // if (keys[SDL_SCANCODE_P]) {
    //     keys[SDL_SCANCODE_P] = 0;
    //     save_pamon_config();
    // }
    if (keys[SDL_SCANCODE_L]) {
        keys[SDL_SCANCODE_L] = 0;
        load_pamon_config();
    }

    if (keys[SDL_SCANCODE_SPACE]) {
        if (cam_position[1] <= FLOOR_Y+0.00001)
            cam_velocity[1] = JUMP_AMOUNT;
    }

    doPhysics(t-lastMove);

    lastMove = t;
}

void updateCamera()
{
    cam_look_dir[0] = sin(mouse_rot[1]);
    cam_look_dir[1] = -tan(mouse_rot[0]);
    cam_look_dir[2] = cos(mouse_rot[1]);

    moveCamera();
    vec3_norm(cam_up, cam_norm);
    vec3_add(cam_look_at, cam_look_dir, cam_position);
}

void mouseMotion(int deltaX, int deltaY)
{
    if(abs(deltaX) < MOUSE_MAX_CHANGE) {
        mouse_rot[1] -= deltaX/MOUSE_SENSITIVITY;
    }
    if(abs(deltaY) < MOUSE_MAX_CHANGE) {
        mouse_rot[0] += deltaY/MOUSE_SENSITIVITY;
        float maxAngle = 89.9*PI/180.0;
        if (mouse_rot[0] > maxAngle)
            mouse_rot[0] = maxAngle;
        if (mouse_rot[0] < -maxAngle)
            mouse_rot[0] = -maxAngle;
    }
    
    updateCamera();
}