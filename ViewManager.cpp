///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ===============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Modified for milestone camera navigation, mouse look,
//  scroll speed adjustment, and projection switching
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// declaration of the global variables and defines
namespace
{
    // Variables for window width and height
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    // camera object used for viewing and interacting with
    // the 3D scene
    Camera* g_pCamera = nullptr;

    // these variables are used for mouse movement processing
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // false = perspective, true = orthographic
    bool bOrthographicProjection = false;

    // scroll will control camera travel speed
    const float MIN_MOVEMENT_SPEED = 1.0f;
    const float MAX_MOVEMENT_SPEED = 20.0f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    // initialize the member variables
    m_pShaderManager = pShaderManager;
    m_pWindow = NULL;
    g_pCamera = new Camera();

    // default camera view parameters
    g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.3f, -1.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80.0f;
    g_pCamera->MovementSpeed = 5.0f;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
    // free up allocated memory
    m_pShaderManager = NULL;
    m_pWindow = NULL;

    if (NULL != g_pCamera)
    {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse wheel is scrolled within the active display window.
 *  It adjusts camera movement speed.
 ***********************************************************/
void Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
    if (g_pCamera != nullptr)
    {
        g_pCamera->MovementSpeed += static_cast<float>(yOffset);

        if (g_pCamera->MovementSpeed < MIN_MOVEMENT_SPEED)
        {
            g_pCamera->MovementSpeed = MIN_MOVEMENT_SPEED;
        }
        if (g_pCamera->MovementSpeed > MAX_MOVEMENT_SPEED)
        {
            g_pCamera->MovementSpeed = MAX_MOVEMENT_SPEED;
        }
    }
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = nullptr;

    // try to create the displayed OpenGL window
    window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowTitle,
        NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    // optional: capture mouse inside window for full camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // mouse position callback
    glfwSetCursorPosCallback(window, ViewManager::Mouse_Position_Callback);

    // mouse scroll callback
    glfwSetScrollCallback(window, Mouse_Scroll_Callback);

    // enable blending for supporting transparent rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;

    return window;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    if (gFirstMouse)
    {
        gLastX = static_cast<float>(xMousePos);
        gLastY = static_cast<float>(yMousePos);
        gFirstMouse = false;
    }

    float xOffset = static_cast<float>(xMousePos) - gLastX;
    float yOffset = gLastY - static_cast<float>(yMousePos);

    gLastX = static_cast<float>(xMousePos);
    gLastY = static_cast<float>(yMousePos);

    if (g_pCamera != nullptr && !bOrthographicProjection)
    {
        g_pCamera->ProcessMouseMovement(xOffset, yOffset);
    }
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
    // close the window if the escape key has been pressed
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_pWindow, true);
    }

    // perspective projection
    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
    {
        bOrthographicProjection = false;
    }

    // orthographic projection
    if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
    {
        bOrthographicProjection = true;
    }

    // keyboard camera movement only in perspective mode
    if (!bOrthographicProjection)
    {
        if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
        }

        if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
        }

        if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
        }

        if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
        }

        if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
        }

        if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
        {
            g_pCamera->ProcessKeyboard(UP, gDeltaTime);
        }
    }
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
    glm::mat4 view;
    glm::mat4 projection;

    // per-frame timing
    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    // process any keyboard events that may be waiting
    ProcessKeyboardEvents();

    if (bOrthographicProjection)
    {
        // fixed front-facing orthographic view
        glm::vec3 orthoCamPos = glm::vec3(0.0f, 6.0f, 12.0f);
        glm::vec3 target = glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);

        view = glm::lookAt(orthoCamPos, target, upVec);

        projection = glm::ortho(
            -8.0f, 8.0f,
            -6.0f, 6.0f,
            0.1f, 100.0f);
    }
    else
    {
        // normal free camera perspective view
        view = g_pCamera->GetViewMatrix();

        projection = glm::perspective(
            glm::radians(g_pCamera->Zoom),
            static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT),
            0.1f,
            100.0f);
    }

    // if the shader manager object is valid
    if (NULL != m_pShaderManager)
    {
        // set the view matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ViewName, view);

        // set the projection matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);

        // set the view position of the camera into the shader
        if (bOrthographicProjection)
        {
            m_pShaderManager->setVec3Value("viewPosition", glm::vec3(0.0f, 6.0f, 12.0f));
        }
        else
        {
            m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
        }
    }
}