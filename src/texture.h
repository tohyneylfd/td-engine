#pragma once
#include <string>

class texture {
public:
    explicit texture(const std::string& path, GLenum format) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        int width, height, nrChannels;
        //stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    ~texture() {
        glDeleteTextures(1, &id);
        std::cout << "texture cleared" << std::endl;
    }

    operator unsigned int() const { return id; }

private:
    unsigned int id;
};
