#define __HIP_PLATFORM_AMD__
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include "mesh.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "texture.h"

#include <GLFW/glfw3.h>

const float wWidth = 1600;
const float wHeight = 900;


float fbVertices[] =
{
    // Coords          // texCoords
     1.0f, -1.0f,      1.0f, 0.0f,
    -1.0f,  1.0f,      0.0f, 1.0f,
    -1.0f, -1.0f,      0.0f, 0.0f,

     1.0f,  1.0f,      1.0f, 1.0f,
    -1.0f,  1.0f,      0.0f, 1.0f,
     1.0f, -1.0f,      1.0f, 0.0f,
};

glm::vec3 camPos(0.0f, 1.0f, 0.0f);
glm::vec3 camRot(-90.0f, 0.0f, 0.0f);

glm::vec3 carPos(0.0f, 0.0f, 0.0f);
glm::vec3 lightPos(1.0f, 1.0f, 0.0f);

extern "C" {
    _declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void processInput(GLFWwindow* window, float dTime, glm::vec3 camFront, glm::vec3 camUp) {
    const float camSpeed = 1.5f * dTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos += camSpeed * camUp;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        camPos -= camSpeed * camUp;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        carPos.z -= camSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        carPos.z += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        carPos.x -= camSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        carPos.x += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        carPos.y += camSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        carPos.y -= camSpeed;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static const float sens = 0.1f;

    float centerX = wWidth / 2.0f;
    float centerY = wHeight / 2.0f;

    float xOffset = xpos - centerX;
    float yOffset = centerY - ypos;

    camRot.y += xOffset * sens;
    camRot.x += yOffset * sens;

    camRot.x = std::clamp(camRot.x, -89.0f, 89.0f);

    glfwSetCursorPos(window, centerX, centerY);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "Optimized Engine", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    
    glfwSwapInterval(0); //vsync (0-off, 1-on)

    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int framebufferTexture;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wWidth, wHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wWidth, wHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer error: " << fboStatus << std::endl;

    unsigned int fbVAO, fbVBO;
    glGenVertexArrays(1, &fbVAO);
    glGenBuffers(1, &fbVBO);
    glBindVertexArray(fbVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fbVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fbVertices), &fbVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    shader fb("src/fb.vert", "src/fb.frag");
    glUniform1i(glGetUniformLocation(fb, "screenTexture"), 0);

    camera camera0(camPos, camRot);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), wWidth / wHeight, 0.001f, 100.0f);

    // model.h is kinda for testing purposes only, it sometimes have weird bugs and other problems
    // can't load model with multiple meshes in it
    model carModel("res/car.glb");
    //also you can create textures from images
    //texture tx0("res/brickwall.png", GL_RGB);
    mesh car(carModel.vertices, carModel.indices);
    shader dShader("src/default.vert", "src/default.frag", proj);

    double delta, frame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //bg
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        delta = glfwGetTime() - frame;
        frame = glfwGetTime();
        glfwSetWindowTitle(window, std::to_string(1/delta).c_str());

        processInput(window, delta, camera0.front, camera0.up);
        camera0.update();
        camera0.pos = camPos;
        camera0.rot = camRot;

        car.draw(dShader, carModel.textures[0], camera0);
        car.matrix = glm::mat4(1.0f);
        car.matrix = glm::translate(car.matrix, carPos);
        //car.matrix = glm::scale(car.matrix, glm::vec3(0.5));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(fb);
        glBindVertexArray(fbVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
