#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

#include "SkyBox.hpp"

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 1.0f;

GLboolean pressedKeys[1024];

// models
//gps::Model3D solarSystem;
gps::Model3D sun;
gps::Model3D mercury;
gps::Model3D venus;
gps::Model3D earth;
gps::Model3D mars;
gps::Model3D jupiter;
gps::Model3D saturn;
gps::Model3D uranus;
gps::Model3D neptune;
gps::Model3D spaceship1;
gps::Model3D spaceship2;

glm::mat4 sunModel = glm::mat4(1.0f);
glm::mat4 mercuryModel = glm::mat4(1.0f);
glm::mat4 venusModel = glm::mat4(1.0f);
glm::mat4 earthModel = glm::mat4(1.0f);
glm::mat4 marsModel = glm::mat4(1.0f);
glm::mat4 jupiterModel = glm::mat4(1.0f);
glm::mat4 saturnModel = glm::mat4(1.0f);
glm::mat4 uranusModel = glm::mat4(1.0f);
glm::mat4 neptuneModel = glm::mat4(1.0f);
glm::mat4 spaceship1Model = glm::mat4(1.0f);
glm::mat4 spaceship2Model = glm::mat4(1.0f);

const float sunRotationSpeed = 1.0f;
const float mercuryRotationSpeed = 256.0f;
const float venusRotationSpeed = 128.0f;
const float earthRotationSpeed = 64.0f;
const float marsRotationSpeed = 32.0f;
const float jupiterRotationSpeed = 16.0f;
const float saturnRotationSpeed = 8.0f;
const float uranusRotationSpeed = 4.0f;
const float neptuneRotationSpeed = 2.0f;
const float spaceship1Speed = 1.0f;
const float spaceship2Speed = 1.0f;

GLfloat angle;
GLfloat sunAngle = 0.0f;
GLfloat mercuryOrbitAngle = 0.0f;
GLfloat venusOrbitAngle = 0.0f;
GLfloat earthOrbitAngle = 0.0f;
GLfloat marsOrbitAngle = 0.0f;
GLfloat jupiterOrbitAngle = 0.0f;
GLfloat saturnOrbitAngle = 0.0f;
GLfloat uranusOrbitAngle = 0.0f;
GLfloat neptuneOrbitAngle = 0.0f;
GLfloat spaceship1Distance = 0.0f;
GLfloat spaceship2Distance = 0.0f;

// shaders
gps::Shader myBasicShader;
gps::SkyBox mySkyBox;
gps::Shader skyBoxShader;
gps::Shader depthMapShader;

glm::vec3 sunPosition;

// depth map FBO and texture
GLuint shadowMapFBO;
GLuint depthMapTexture;
const GLuint SHADOW_WIDTH = 1024;
const GLuint SHADOW_HEIGHT = 1024;

void initShadowMapping() { // initFBO
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &shadowMapFBO);

    // Create the depth texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
        GL_FLOAT, NULL);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach the texture as the depth buffer of the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

bool firstMouse = true;
float lastX = 300, lastY = 150;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    myCamera.rotate(yoffset, xoffset);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 728, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    sun.LoadModel("models/planets/star.obj");
    mercury.LoadModel("models/planets/mercury.obj");
    venus.LoadModel("models/planets/venus.obj");
    earth.LoadModel("models/planets/earth.obj");
    mars.LoadModel("models/planets/mars.obj");
    jupiter.LoadModel("models/planets/jupiter.obj");
    saturn.LoadModel("models/planets/bakedSaturn.obj");
    uranus.LoadModel("models/planets/uranus.obj");
    neptune.LoadModel("models/planets/neptune.obj");
    spaceship1.LoadModel("models/spaceship1/spaceship1.obj");
    spaceship2.LoadModel("models/spaceship2/spaceship2.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyBoxShader.useShaderProgram();
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
    depthMapShader.useShaderProgram();
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint sunPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "sunPosition");
    glUniform3fv(sunPosLoc, 1, glm::value_ptr(sunPosition));

    //set light color
    lightColor = glm::vec3(20.0f, 20.0f, 20.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void initSkyBox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/starfield_rt.tga");
    faces.push_back("skybox/starfield_lf.tga");
    faces.push_back("skybox/starfield_up.tga");
    faces.push_back("skybox/starfield_dn.tga");
    faces.push_back("skybox/starfield_bk.tga");
    faces.push_back("skybox/starfield_ft.tga");

    mySkyBox.Load(faces);
}

glm::mat4 computeLightSpaceTrMatrix() {
    // Adjust these parameters based on your scene's size and the light's position
    const GLfloat near_plane = 1.0f, far_plane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(sunPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void renderSun(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    // update rotation angle
    sunAngle += sunRotationSpeed * deltaTime;
    sunModel = glm::rotate(glm::mat4(1.0f), glm::radians(sunAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(sunModel));
    sun.Draw(shader);
}

void renderMercury(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    mercuryOrbitAngle += mercuryRotationSpeed * deltaTime;
    // Resetting the model matrix
    mercuryModel = glm::mat4(1.0f);
    // Rotate around the origin (orbit rotation)
    mercuryModel = glm::rotate(mercuryModel, glm::radians(mercuryOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    // Translate to the orbit radius
    mercuryModel = glm::translate(mercuryModel, glm::vec3(50.0f, 0.0f, 0.0f));
    mercuryModel = glm::rotate(mercuryModel, glm::radians(mercuryOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mercuryModel));
    mercury.Draw(shader);
}

void renderVenus(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    venusOrbitAngle += venusRotationSpeed * deltaTime;

    // Resetting the model matrix
    venusModel = glm::mat4(1.0f);

    // Rotate around the origin (orbital rotation)
    venusModel = glm::rotate(venusModel, glm::radians(venusOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    venusModel = glm::translate(venusModel, glm::vec3(60.0f, 0.0f, 0.0f));

    // Rotate around its own axis (if needed)
    venusModel = glm::rotate(venusModel, glm::radians(venusOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Scale Venus to make it 2 times bigger than Mercury
    venusModel = glm::scale(venusModel, glm::vec3(2.0f, 2.0f, 2.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(venusModel));
    venus.Draw(shader);
}

void renderEarth(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    earthOrbitAngle += earthRotationSpeed * deltaTime;

    // Resetting the model matrix
    earthModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    earthModel = glm::rotate(earthModel, glm::radians(earthOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    earthModel = glm::translate(earthModel, glm::vec3(70.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    earthModel = glm::rotate(earthModel, glm::radians(earthOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Apply scaling transformation to make Venus 2 times bigger than Mercury
    earthModel = glm::scale(earthModel, glm::vec3(4.0f, 4.0f, 4.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earthModel));
    earth.Draw(shader);
}

void renderMars(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    marsOrbitAngle += marsRotationSpeed * deltaTime;

    // Resetting the model matrix
    marsModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    marsModel = glm::rotate(marsModel, glm::radians(marsOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    marsModel = glm::translate(marsModel, glm::vec3(80.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    marsModel = glm::rotate(marsModel, glm::radians(marsOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    marsModel = glm::scale(marsModel, glm::vec3(3.0f, 3.0f, 3.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(marsModel));
    mars.Draw(shader);
}

void renderJupiter(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    jupiterOrbitAngle += jupiterRotationSpeed * deltaTime;

    // Resetting the model matrix
    jupiterModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    jupiterModel = glm::rotate(jupiterModel, glm::radians(jupiterOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    jupiterModel = glm::translate(jupiterModel, glm::vec3(90.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    jupiterModel = glm::rotate(jupiterModel, glm::radians(jupiterOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    jupiterModel = glm::scale(jupiterModel, glm::vec3(10.0f, 10.0f, 10.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(jupiterModel));
    jupiter.Draw(shader);
}

void renderSaturn(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    saturnOrbitAngle += saturnRotationSpeed * deltaTime;

    // Resetting the model matrix
    saturnModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    saturnModel = glm::rotate(saturnModel, glm::radians(saturnOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    saturnModel = glm::translate(saturnModel, glm::vec3(100.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    saturnModel = glm::rotate(saturnModel, glm::radians(saturnOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    saturnModel = glm::scale(saturnModel, glm::vec3(8.0f, 8.0f, 8.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(saturnModel));
    saturn.Draw(shader);
}

void renderUranus(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    uranusOrbitAngle += uranusRotationSpeed * deltaTime;

    // Resetting the model matrix
    uranusModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    uranusModel = glm::rotate(uranusModel, glm::radians(uranusOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    uranusModel = glm::translate(uranusModel, glm::vec3(110.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    uranusModel = glm::rotate(uranusModel, glm::radians(uranusOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    uranusModel = glm::scale(uranusModel, glm::vec3(6.0f, 6.0f, 6.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(uranusModel));
    uranus.Draw(shader);
}

void renderNeptune(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    neptuneOrbitAngle += neptuneRotationSpeed * deltaTime;

    // Resetting the model matrix
    neptuneModel = glm::mat4(1.0f);

    // Rotate around the origin (orbit rotation)
    neptuneModel = glm::rotate(neptuneModel, glm::radians(neptuneOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Translate to the orbit radius
    neptuneModel = glm::translate(neptuneModel, glm::vec3(120.0f, 0.0f, 0.0f));

    // Rotate around its own axis if needed
    neptuneModel = glm::rotate(neptuneModel, glm::radians(neptuneOrbitAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    neptuneModel = glm::scale(neptuneModel, glm::vec3(6.0f, 6.0f, 6.0f));
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(neptuneModel));
    neptune.Draw(shader);
}

void renderSpaceShip1(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    spaceship1Distance += spaceship1Speed * deltaTime;

    spaceship1Model = glm::mat4(1.0f);
    
    // spaceship1 is moving
    spaceship1Model = glm::translate(spaceship1Model, glm::vec3(-spaceship1Distance, 0.0f, 0.0f));

    // init position of spaceship1
    spaceship1Model = glm::translate(spaceship1Model, glm::vec3(120.0f, 0.0f, 120.0f));

    spaceship1Model = glm::scale(spaceship1Model, glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(spaceship1Model));
    spaceship1.Draw(shader);
}

void renderSpaceShip2(gps::Shader shader, float deltaTime) {
    shader.useShaderProgram();
    spaceship2Distance += spaceship2Speed * deltaTime;

    spaceship2Model = glm::mat4(1.0f);

    // spaceship2 is moving
    spaceship2Model = glm::translate(spaceship2Model, glm::vec3(0.0f, 0.0f, -spaceship2Distance));

    // init position of spaceship2
    spaceship2Model = glm::translate(spaceship2Model, glm::vec3(-120.0f, 0.0f, 120.0f));
    spaceship2Model = glm::rotate(spaceship2Model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    spaceship2Model = glm::scale(spaceship2Model, glm::vec3(5.0f, 5.0f, 5.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(spaceship2Model));
    spaceship2.Draw(shader);
}

void renderToShadowMap() {
    // Set the viewport to the size of the depth map
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    // Bind the framebuffer for the shadow map
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use the shader for rendering the depth map
    depthMapShader.useShaderProgram();

    // Calculate and set the light space transformation matrix
    glm::mat4 lightSpaceMatrix = computeLightSpaceTrMatrix();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Render the scene (or parts of it) from the light's perspective
    renderSun(depthMapShader, 0.0f); // Similarly render other objects
    renderMercury(depthMapShader, 0.0f);
    renderVenus(depthMapShader, 0.0f);
    renderEarth(depthMapShader, 0.0f);
    renderMars(depthMapShader, 0.0f);
    renderJupiter(depthMapShader, 0.0f);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore the original viewport size
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
}


void renderScene(float deltaTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderToShadowMap();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    //render the scene
    renderSun(myBasicShader, deltaTime);
    renderMercury(myBasicShader, deltaTime);
    renderVenus(myBasicShader, deltaTime);
    renderEarth(myBasicShader, deltaTime);
    renderMars(myBasicShader, deltaTime);
    renderJupiter(myBasicShader, deltaTime);
    renderSaturn(myBasicShader, deltaTime);
    renderUranus(myBasicShader, deltaTime);
    renderNeptune(myBasicShader, deltaTime);
    renderSpaceShip1(myBasicShader, deltaTime);
    renderSpaceShip2(myBasicShader, deltaTime);
    mySkyBox.Draw(skyBoxShader, view, projection);
}


void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}



int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initSkyBox();
    initShaders();
    initUniforms();
    initShadowMapping();
    setWindowCallbacks();

    glCheckError();
    // application loop
   
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        
        // Compute delta time
        static float lastTime = glfwGetTime();
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processMovement();
        renderToShadowMap();
        renderScene(deltaTime);
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());
        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}