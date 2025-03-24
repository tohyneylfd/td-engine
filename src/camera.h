#pragma once
#include <string>

class camera {
public:
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 front;
    glm::vec3 up;
    glm::mat4 view;
    explicit camera(glm::vec3 pos, glm::vec3 rot) : pos(pos), rot(rot), front(0.0f, 0.0f, -1.0f), up(0.0f, 1.0f, 0.0f) {
        update();
    }
    void update() {
        glm::vec3 front;
        front.x = cos(glm::radians(rot.y)) * cos(glm::radians(rot.x));
        front.y = sin(glm::radians(rot.x));
        front.z = sin(glm::radians(rot.y)) * cos(glm::radians(rot.x));
        camera::front = glm::normalize(front);
        view = glm::lookAt(pos, pos + front, up);
    }
};
