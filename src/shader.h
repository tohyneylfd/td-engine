#pragma once
#include <filesystem>
#include <fstream>

class shader {
public:
    explicit shader(const std::filesystem::path& vShader, const std::filesystem::path& fShader, glm::mat4 proj) {
        GLuint VS = compileShader(GL_VERTEX_SHADER, readFile(vShader));
        GLuint FS = compileShader(GL_FRAGMENT_SHADER, readFile(fShader));
        id = glCreateProgram();
        glAttachShader(id, VS);
        glAttachShader(id, FS);
        glLinkProgram(id);
        glValidateProgram(id);
        glDeleteShader(VS);
        glDeleteShader(FS);
        glUseProgram(id);
        glUniformMatrix4fv(glGetUniformLocation(id, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    }
    explicit shader(const std::filesystem::path& vShader, const std::filesystem::path& fShader) {
        GLuint VS = compileShader(GL_VERTEX_SHADER, readFile(vShader));
        GLuint FS = compileShader(GL_FRAGMENT_SHADER, readFile(fShader));
        id = glCreateProgram();
        glAttachShader(id, VS);
        glAttachShader(id, FS);
        glLinkProgram(id);
        glValidateProgram(id);
        glDeleteShader(VS);
        glDeleteShader(FS);
        glUseProgram(id);
    }
    ~shader() {
        glDeleteProgram(id);
        std::cout << "shader cleared" << std::endl;
    }

    operator GLuint() const { return id; }

private:
    GLuint id;
    std::string readFile(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        const auto sz = std::filesystem::file_size(path);
        std::string result(sz, '\0');
        in.read(result.data(), sz);
        return result;
    }
    GLuint compileShader(GLuint sType, const std::string& sSource) {
        GLuint shader = glCreateShader(sType);
        const char* raw = sSource.c_str();
        glShaderSource(shader, 1, &raw, nullptr);
        glCompileShader(shader);
        return shader;
    }
};
