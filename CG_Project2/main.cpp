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

bool startBook = false;
bool startReaper = false;
bool startGhost = false;
bool BookToogle = false;
bool reaperToogle = false;
bool ghostToogle = false;
bool runFOV = false;


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
    void Lights(float light_d, float light_linear, Vector3f spot_color);

private:

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;

    BasicMesh* Room = NULL;
    BasicMesh* Book = NULL;
    BasicMesh* Reaper = NULL;
    BasicMesh* Ghost = NULL;

    PersProjInfo persProjInfo;
    LightingTechnique* pLightingTech = NULL;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[LightingTechnique::MAX_SPOT_LIGHTS];

    float FOV = 90.f;
    float zNear = 0.01f;
    float zFar = 100.0f;
    float runningFOV = 0.0f;

    float bookmoveY = 0.0f;

    float reaperMove = 0.0f;
    float reaperX = 0.0f;
    float ghostmoveX = 0.0f;
    
};


GGProject2::GGProject2()
{
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_DEPTH_TEST);

    pointLights[0].DiffuseIntensity = 0.5f;
    pointLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[0].Attenuation.Linear = 0.1f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 0.5f;
    pointLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    pointLights[1].Attenuation.Linear = 0.1f;
    pointLights[1].Attenuation.Exp = 0.0f;

    spotLights[0].DiffuseIntensity = 2.5f;
    spotLights[0].Color = Vector3f(1.0f, 1.0f, 1.0f);
    spotLights[0].Attenuation.Linear = 0.01f;
    spotLights[0].Cutoff = 25.0f;

    spotLights[1].DiffuseIntensity = 0.0f;
    spotLights[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
    spotLights[1].Attenuation.Linear = 0.01f;
    spotLights[1].Cutoff = 25.0f;
}


GGProject2::~GGProject2()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (Room) {
        delete Room;
    }

    if (Book) {
        delete Book;
    }

    if (Reaper) {
        delete Reaper;
    }

    if (Ghost) {
        delete Ghost;
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

    Book = new BasicMesh();
    if (!Book->LoadMesh("../Models/book/book.obj")) {
        return false;
    }

    Reaper = new BasicMesh();
    if (!Reaper->LoadMesh("../Models/reaper/reaper.obj")) {
        return false;
    }

    Ghost = new BasicMesh();
    if (!Ghost->LoadMesh("../Models/ghost/ghost.obj")) {
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

void GGProject2::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };
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

    pointLights[1].WorldPosition.x = 15.0f;
    pointLights[1].WorldPosition.y = 3;
    pointLights[1].WorldPosition.z = 2.0f;
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

    WorldTrans& meshWorldTransform = Book->GetWorldTransform();
    meshWorldTransform.SetScale(-4.0f, 4.0f, 4.0f);
    //meshWorldTransform.SetRotation(0.0f, 180.0f, 0.0f);
    if (startBook && BookToogle) {
        
        static float stopY = 3.0f;
        bookmoveY += 0.005;

        if (bookmoveY < stopY) {
            meshWorldTransform.SetPosition(2.5f, 3.6f + bookmoveY, 17.0f);
            meshWorldTransform.Rotate(1.0f, 0.0f, 0.0f);
        }
        else if (bookmoveY > stopY) {
            meshWorldTransform.SetPosition(2.5f, 3.6f + stopY, 17.0f);
            meshWorldTransform.Rotate(0.0f, 1.0f, 0.0f);
        }
    }
    else if (!startBook && !BookToogle) {
        meshWorldTransform.SetPosition(2.5f, 3.3f, 17.0f);
        bookmoveY = 0.0f;
    }

    World = meshWorldTransform.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform);
    pointLights[1].CalcLocalPosition(meshWorldTransform);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(Book->GetMaterial());
    CameraLocalPos3f = meshWorldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);
    Book->Render();
    
    WorldTrans& meshWorldTransform2 = Reaper->GetWorldTransform();
    meshWorldTransform2.SetRotation(0.0f, -90.0f, 0.0f);
    meshWorldTransform2.SetScale(2.0f, 2.0f, 2.0f);
    if (startReaper && reaperToogle) {
        reaperMove += 0.05f;
        reaperX += 0.05f;

        if (reaperMove > 0.0f && reaperMove < 5.0f) {
            meshWorldTransform2.SetPosition(-8.00f + reaperX, 0.0f, 4.0f);
        }
        else if (reaperMove > 5.0f && reaperMove < 10.0f) {
            meshWorldTransform2.SetPosition(-5.0f, 0.0f, 4.0f);
        }

        else if (reaperMove > 10.0f && reaperMove < 15.0f) {
            meshWorldTransform2.SetPosition(-8.0f, 0.0f, 13.0f);
        }

        else if (reaperMove > 20.0f && reaperMove < 30.0f) {
            meshWorldTransform2.SetPosition(0.0f , 0.0f, 9.0f);
            meshWorldTransform2.SetRotation(0.0f, -100.0f, 0.0f);
            
        }

        else if (reaperMove > 30.0f) {
            meshWorldTransform2.SetPosition(-13.5f, 0.0f, 4.0f);
            Lights(3.0f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));
            reaperMove = 0.0f;
            startReaper = false;
            reaperToogle = false;
        }

        
    }
    else if (!startReaper && !reaperToogle) {
        meshWorldTransform2.SetPosition(-13.5f, 0.0f, 4.0f);
        reaperMove = 0.0f;
    } 
       
    World = meshWorldTransform2.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform2);
    pointLights[1].CalcLocalPosition(meshWorldTransform2);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform2);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform2);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(Reaper->GetMaterial());
    CameraLocalPos3f = meshWorldTransform2.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);
    Reaper->Render();

    WorldTrans& meshWorldTransform3 = Ghost->GetWorldTransform();
    meshWorldTransform3.SetRotation(0.0f, 90.0f, 0.0f);
    meshWorldTransform3.SetScale(4.0f, 4.0f, 4.0f);
    if (startGhost && ghostToogle && runFOV) {
        static float stopX = 15.0f;
        ghostmoveX += 0.5;

        if (ghostmoveX < stopX) {
            meshWorldTransform3.SetPosition(30.0f - ghostmoveX, 0.0f, 2.0f);
        }
        else if (ghostmoveX > stopX) {
            meshWorldTransform3.SetPosition(30.0f - stopX, 0.0f, 2.0f);
        }

    }
    else if (!startGhost && !ghostToogle && !runFOV) {
        meshWorldTransform3.SetPosition(30.0f, 0.0f, 2.0f);
        ghostmoveX = 0.0f;
    }

    World = meshWorldTransform3.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(meshWorldTransform3);
    pointLights[1].CalcLocalPosition(meshWorldTransform3);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(meshWorldTransform3);
    spotLights[1].CalcLocalDirectionAndPosition(meshWorldTransform3);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(Ghost->GetMaterial());
    CameraLocalPos3f = meshWorldTransform3.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);
    Ghost->Render();


    glutPostRedisplay();
    glutSwapBuffers();
}


#define ATTEN_STEP 0.01f
#define ANGLE_STEP 1.0f

void GGProject2::Lights(float light_d, float light_linear, Vector3f spot_color) {
    pointLights[0].DiffuseIntensity = light_d;
    pointLights[0].Attenuation.Linear = light_linear;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = light_d;
    pointLights[1].Attenuation.Linear = light_linear;
    pointLights[1].Attenuation.Exp = 0.0f;

    spotLights[0].Color = spot_color;
}



void GGProject2::KeyboardCB(unsigned char key, int mouse_x, int mouse_y)
{
    switch (key) {
    case '1':
        if (!BookToogle) {
            startBook = true;
            BookToogle = true;
        }
        else if (BookToogle) {
            startBook = false;
            BookToogle = false;
        }
        break;

    case '2':
        if (!reaperToogle) {
            startReaper = true;
            reaperToogle = true;
        }
        else if (reaperToogle) {
            startReaper = false;
            reaperToogle = false;
        }
        Lights(0.5f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));
        break;

    case '3':
        if (!ghostToogle) {
            startGhost = true;
            ghostToogle = true;
            runFOV = true;
        }
        else if (ghostToogle) {
            startGhost = false;
            ghostToogle = false;
            runFOV = false;
        }
        Lights(0.5f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));
        break;

    case '4':
        if (FOV < 45.0f) {
            FOV = 45.0f;
        }
        else if (FOV > 45.0F) {
            FOV -= 5.0f;
        }
        break;

    case '5':
        if (FOV > 90.0f) {
            FOV = 90.0f;
        }
        else if (FOV < 90.0f) {
            FOV += 5.0f;
        }
        break;

    case '7': // lights off
        Lights(0.0001f, 0.00001f, Vector3f(1.0f, 1.0f, 1.0f));
        break;

    case '6': // lights full
        Lights(3.0f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));
        break;

    case '9': // lights Init
        Lights(0.5f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));
        break;

    case '8': // spot red and lights off
        Lights(0.0001f, 0.00001f, Vector3f(1.0f, 0.0f, 0.0f));
        break;

    case '0': //reset
        pGameCamera->SetPosition(21.0f, 5.0f, 2.0f);
        
        startReaper = false;
        reaperToogle = false;

        startBook = false;
        BookToogle = false;

        FOV = 90.0f;

        Lights(0.5f, 0.01f, Vector3f(1.0f, 1.0f, 1.0f));

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

    pGameCamera->OnKeyboard(key);

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA );
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("The Strange Place");
    printf("window id: %d\n", win);

    glutSetCursor(GLUT_CURSOR_NONE);

    char game_mode_string[64];
    snprintf(game_mode_string, sizeof(game_mode_string), "%dx%d@62", WINDOW_WIDTH, WINDOW_HEIGHT);
    glutGameModeString(game_mode_string);
    glutEnterGameMode();
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
