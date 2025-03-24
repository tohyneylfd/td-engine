#define CGLTF_IMPLEMENTATION

#include <cgltf.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

class model {
public:
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    std::vector<GLuint> textures;

    // .glb loading
    explicit model(const std::string& path) : data(nullptr) {
        cgltf_options options = {};
        cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

        if (result != cgltf_result_success) {
            throw std::runtime_error("Failed to load GLB file: " + path);
        }

        result = cgltf_load_buffers(&options, data, path.c_str());
        if (result != cgltf_result_success) {
            cgltf_free(data);
            throw std::runtime_error("Failed to load buffers for GLB file: " + path);
        }

        for (cgltf_size i = 0; i < data->meshes_count; ++i) {
            const cgltf_mesh& mesh = data->meshes[i];
            for (cgltf_size j = 0; j < mesh.primitives_count; ++j) {
                const cgltf_primitive& primitive = mesh.primitives[j];
                extractVertices(primitive);
                extractIndices(primitive);
            }
        }

        if (data->materials_count == 0) {
            std::cout << "No materials found in the GLTF file." << std::endl;
            return;
        }

        const cgltf_material& material = data->materials[0];

        // Base Color
        if (material.pbr_metallic_roughness.base_color_texture.texture) {
            const cgltf_image* image = material.pbr_metallic_roughness.base_color_texture.texture->image;
            extractTexture(image);
        }

        // Normal Map
        if (material.normal_texture.texture) {
            const cgltf_image* image = material.normal_texture.texture->image;
            extractTexture(image);
        }

        // Metallic-Roughness Map
        if (material.pbr_metallic_roughness.metallic_roughness_texture.texture) {
            const cgltf_image* image = material.pbr_metallic_roughness.metallic_roughness_texture.texture->image;
            extractTexture(image);
        }

        // Occlusion Map
        if (material.occlusion_texture.texture) {
            const cgltf_image* image = material.occlusion_texture.texture->image;
            extractTexture(image);
        }

        // Emissive Map
        if (material.emissive_texture.texture) {
            const cgltf_image* image = material.emissive_texture.texture->image;
            extractTexture(image);
        }
    }

    ~model() {
        if (data) {
            cgltf_free(data);
            std::cout << "gltf cleared" << std::endl;
        }
        if (textures.size() != 0) {
            for (GLuint id : textures) {
                glDeleteTextures(1, &id);
            }
            std::cout << "textures deleted" << std::endl;
        }
    }

private:
    cgltf_data* data;

    void extractVertices(const cgltf_primitive& primitive) {
        const cgltf_accessor* positionAccessor = nullptr;
        const cgltf_accessor* colorAccessor = nullptr;
        const cgltf_accessor* uvAccessor = nullptr;
        const cgltf_accessor* normalAccessor = nullptr;

        // POSITION, COLOR, UV
        for (cgltf_size i = 0; i < primitive.attributes_count; ++i) {
            const cgltf_attribute& attribute = primitive.attributes[i];
            if (attribute.type == cgltf_attribute_type_position) {
                positionAccessor = attribute.data;
            }
            else if (attribute.type == cgltf_attribute_type_color) {
                colorAccessor = attribute.data;
            }
            else if (attribute.type == cgltf_attribute_type_texcoord) {
                uvAccessor = attribute.data;
            }
            else if (attribute.type == cgltf_attribute_type_normal) {
                normalAccessor = attribute.data;
            }
        }

        if (!positionAccessor) {
            throw std::runtime_error("position attribute not found in mesh.");
        }

        const uint8_t* positionData = static_cast<const uint8_t*>(
            positionAccessor->buffer_view->buffer->data);
        const float* positions = reinterpret_cast<const float*>(
            positionData + positionAccessor->buffer_view->offset + positionAccessor->offset);

        const uint8_t* colorData = colorAccessor ? static_cast<const uint8_t*>(
            colorAccessor->buffer_view->buffer->data) : nullptr;
        const float* colors = colorData ? reinterpret_cast<const float*>(
            colorData + colorAccessor->buffer_view->offset + colorAccessor->offset) : nullptr;

        const uint8_t* uvData = uvAccessor ? static_cast<const uint8_t*>(
            uvAccessor->buffer_view->buffer->data) : nullptr;
        const float* uvs = uvData ? reinterpret_cast<const float*>(
            uvData + uvAccessor->buffer_view->offset + uvAccessor->offset) : nullptr;

        const uint8_t* normalData = normalAccessor ? static_cast<const uint8_t*>(
            normalAccessor->buffer_view->buffer->data) : nullptr;
        const float* normals = normalData ? reinterpret_cast<const float*>(
            normalData + normalAccessor->buffer_view->offset + normalAccessor->offset) : nullptr;

        for (size_t i = 0; i < positionAccessor->count; ++i) {
            vertices.push_back(positions[i * 3 + 0]);
            vertices.push_back(positions[i * 3 + 1]);
            vertices.push_back(positions[i * 3 + 2]);

            if (colors) {
                vertices.push_back(colors[i * 3 + 0]);
                vertices.push_back(colors[i * 3 + 1]);
                vertices.push_back(colors[i * 3 + 2]);
            }
            else {
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
            }

            if (uvs) {
                vertices.push_back(uvs[i * 2 + 0]);
                vertices.push_back(uvs[i * 2 + 1]);
            }
            else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
    }

    void extractIndices(const cgltf_primitive& primitive) {
        const cgltf_accessor* accessor = primitive.indices;

        if (!accessor) {
            throw std::runtime_error("indices not found in mesh.");
        }

        const uint8_t* indexData = static_cast<const uint8_t*>(
            accessor->buffer_view->buffer->data);

        if (accessor->component_type == cgltf_component_type_r_16u) {
            const uint16_t* indicesData = reinterpret_cast<const uint16_t*>(
                indexData + accessor->buffer_view->offset + accessor->offset);
            for (cgltf_size i = 0; i < accessor->count; ++i) {
                indices.push_back(static_cast<uint32_t>(indicesData[i]));
            }
        }
        else if (accessor->component_type == cgltf_component_type_r_32u) {
            const uint32_t* indicesData = reinterpret_cast<const uint32_t*>(
                indexData + accessor->buffer_view->offset + accessor->offset);
            for (cgltf_size i = 0; i < accessor->count; ++i) {
                indices.push_back(indicesData[i]);
            }
        }
        else {
            throw std::runtime_error("unsupported index component type.");
        }
    }

    void extractTexture(const cgltf_image* image) {
        if (image->buffer_view) {
            const cgltf_buffer_view* bufferView = image->buffer_view;
            const uint8_t* bufferData = static_cast<const uint8_t*>(bufferView->buffer->data);
            const uint8_t* imageData = bufferData + bufferView->offset;
            GLuint id;
            int width, height, channels;
            unsigned char* pixels = stbi_load_from_memory(imageData, bufferView->size, &width, &height, &channels, 4);
            if (!pixels) {
                throw std::runtime_error("Failed to load embedded texture.");
            }

            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(pixels);
            glBindTexture(GL_TEXTURE_2D, 0);
            textures.push_back(id);
            std::cout << "texture loaded successfully." << std::endl;
        }
        else {
            throw std::runtime_error("Embedded texture buffer view not found.");
        }
    }
};
