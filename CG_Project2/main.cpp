#include <stdio.h>
#include <string.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "ogldev_math_3d.h"
#include "ogldev_texture.h"
#include "ogldev_world_transform.h"
#include "ogldev_basic_mesh.h"
#include "camera.h"
#include "lighting_technique.h"
#include "ogldev_engine_common.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080



class GGProject2
{
public:
    GGProject2();
    ~GGProject2();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;

    BasicMesh* Room = NULL;
    BasicMesh* Zombie = NULL;


    PersProjInfo persProjInfo;
    LightingTechnique* pLightingTech = NULL;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    float counter = 0;
};


GGProject2::GGProject2()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    /*
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    */

    glEnable(GL_DEPTH_TEST);

    float FOV = 90.0f;
    float zNear = 0.01f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

    pointLights[0].DiffuseIntensity = 1.0f;
    pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[0].Attenuation.Linear = 0.1f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 1.0f;
    pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[1].Attenuation.Linear = 0.1f;
    pointLights[1].Attenuation.Exp = 0.0f;

    spotLights[0].DiffuseIntensity = 1.0f;
    spotLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
    spotLights[0].Attenuation.Linear = 0.01f;
    spotLights[0].Cutoff = 30.0f;

    spotLights[1].DiffuseIntensity = 0.0f;
    spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    spotLights[1].Attenuation.Linear = 0.01f;
    spotLights[1].Cutoff = 30.0f;
}


GGProject2::~GGProject2()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (Room) {
        delete Room;
    }

    if (Zombie) {
        delete Zombie;
    }

    if (pLightingTech) {
        delete pLightingTech;
    }
}


bool GGProject2::Init()
{
    Vector3f CameraPos(21.0f, 5.0f, 2.0f);
    Vector3f CameraTarget(0.0f, 0.0f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

    Room = new BasicMesh();
    if (!Room->LoadMesh("../Models/room/room v2.obj")) {
        return false;
    }

    Zombie = new BasicMesh();
    if (!Zombie->LoadMesh("../Models/zombie/zombie.obj")) {
        return false;
    }

    pLightingTech = new LightingTechnique();
    if (!pLightingTech->Init())
    {
        return false;
    }

    pLightingTech->Enable();

    pLightingTech->SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    pLightingTech->SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    return true;
}

bool Start = false;

void GGProject2::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

    WorldTrans& worldTransform = Room->GetWorldTransform();

    worldTransform.SetRotation(0.0f, 0.0f, 0.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 10.0f);


    Matrix4f World = worldTransform.GetMatrix();
    Matrix4f View = pGameCamera->GetMatrix();
    Matrix4f Projection;
    Projection.InitPersProjTransform(persProjInfo);
    Matrix4f WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].WorldPosition.x = -9.0f;
    pointLights[0].WorldPosition.y = 3;
    pointLights[0].WorldPosition.z = 8.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 6.0f;
    pointLights[1].WorldPosition.y = 3;
    pointLights[1].WorldPosition.z = 8.0f;
    pointLights[1].CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].WorldPosition = pGameCamera->GetPos();
    spotLights[0].WorldDirection = pGameCamera->GetTarget();
    spotLights[0].CalcLocalDirectionAndPosition(worldTransform);

    spotLights[1].WorldPosition = Vector3f(0.0f, 1.0f, 0.0f);
    spotLights[1].WorldDirection = Vector3f(0.0f, -1.0f, 0.0f);
    spotLights[1].CalcLocalDirectionAndPosition(worldTransform);

    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(Room->GetMaterial());

    Vector3f CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    Room->Render();

    WorldTrans& meshWorldTransform = Zombie->GetWorldTransform();
    meshWorldTransform.SetPosition(-15.0f, 0.0f, 3.5f);
    meshWorldTransform.SetRotation(0.0f, -90.0f, 0.0f);
    meshWorldTransform.SetScale(3.0f, 3.0f, 3.0f);
   
    World = meshWorldTransform.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform);
    pointLights[1].CalcLocalPosition(meshWorldTransform);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(Zombie->GetMaterial());

    CameraLocalPos3f = meshWorldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    Zombie->Render();

    glutPostRedisplay();
    glutSwapBuffers();
}


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f

void GGProject2::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case ' ':
        Start = true;
        break;
    case 'q':
    case 27:    // escape key code
        exit(0);

    case 'a':
        pointLights[0].Attenuation.Linear += ATTEN_STEP;
        pointLights[1].Attenuation.Linear += ATTEN_STEP;
        break;

    case 'z':
        pointLights[0].Attenuation.Linear -= ATTEN_STEP;
        pointLights[1].Attenuation.Linear -= ATTEN_STEP;
        break;

    case 's':
        pointLights[0].Attenuation.Exp += ATTEN_STEP;
        pointLights[1].Attenuation.Exp += ATTEN_STEP;
        break;

    case 'x':
        pointLights[0].Attenuation.Exp -= ATTEN_STEP;
        pointLights[1].Attenuation.Exp -= ATTEN_STEP;
        break;

    case 'd':
        spotLights[0].Cutoff += ANGLE_STEP;
        break;

    case 'c':
        spotLights[0].Cutoff -= ANGLE_STEP;
        break;

    case 'g':
        spotLights[1].Cutoff += ANGLE_STEP;
        break;

    case 'b':
        spotLights[1].Cutoff -= ANGLE_STEP;
        break;

    }

    printf("Linear %f Exp %f\n", pointLights[0].Attenuation.Linear, pointLights[0].Attenuation.Exp);

    pGameCamera->OnKeyboard(key);

    //printf("Key Pressed: %c\n", key);
}


void GGProject2::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}


void GGProject2::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


GGProject2* pGGProject2 = NULL;


void RenderSceneCB()
{
    pGGProject2->RenderSceneCB();
}


void KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    pGGProject2->KeyboardCB(key, mouse_x, mouse_y);
}


void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGGProject2->SpecialKeyboardCB(key, mouse_x, mouse_y);
}


void PassiveMouseCB(int x, int y)
{
    pGGProject2->PassiveMouseCB(x, y);
}


void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
}

int main(int argc, char** argv)
{
#ifdef _WIN64
    srand(GetCurrentProcessId());
#else
    srandom(getpid());
#endif

    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("The Strange Place");
    printf("window id: %d\n", win);

    glutSetCursor(GLUT_CURSOR_NONE);
    glutFullScreen();


    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    InitializeGlutCallbacks();

    pGGProject2 = new GGProject2();

    if (!pGGProject2->Init()) {
        return 1;
    }

    glutMainLoop();

    return 0;
}
