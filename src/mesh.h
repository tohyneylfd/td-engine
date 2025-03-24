#pragma once
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include "camera.h"

class mesh {
public:
    explicit mesh(const std::vector<float>& vertices, const std::vector<GLuint>& indices) : matrix(glm::mat4(1.0f)), points(indices.size()) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    ~mesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        std::cout << "mesh cleared" << std::endl;
    }

    void draw(GLuint shader, GLuint texture, camera cam) {
        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "matrix"), 1, GL_FALSE, glm::value_ptr(matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader, "cam"), 1, GL_FALSE, glm::value_ptr(cam.view));
        glUniform3f(glGetUniformLocation(shader, "camPos"), cam.pos.x, cam.pos.y, cam.pos.z);

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, points, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    glm::mat4 matrix;

private:
    GLuint VAO, VBO, EBO;
    size_t points;
};
