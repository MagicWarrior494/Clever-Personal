#include "ObjectManager.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

std::pair<std::vector<Vertex>, std::vector<uint16_t>> loadModel(std::string modelFilePath)
{
    std::string inputfile = modelFilePath;
    tinyobj::ObjReaderConfig reader_config;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    std::vector<Vertex> verticies;
    std::vector<uint16_t> indicies;

    float largestMagnitude = 0;

    for (size_t shape = 0; shape < shapes.size(); shape++)
    {
        size_t index_offset = 0;
        for (size_t face = 0; face < shapes[shape].mesh.num_face_vertices.size(); face++)
        {
            size_t fv = size_t(shapes[shape].mesh.num_face_vertices[face]);

            for (size_t vertex = 0; vertex < fv; vertex++)
            {
                tinyobj::index_t index = shapes[shape].mesh.indices[index_offset + vertex];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(index.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(index.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(index.vertex_index) + 2];

                glm::vec3 pos = { vx, vy, vz };
                float mag = glm::distance({ 0,0,0 }, pos);
                if (mag > largestMagnitude)
                    largestMagnitude = mag;

                glm::vec3 normal = { 1, 1, 1 };

                if (index.normal_index >= 0) {
                    normal.x = attrib.normals[3 * size_t(index.normal_index) + 0];
                    normal.y = attrib.normals[3 * size_t(index.normal_index) + 1];
                    normal.z = attrib.normals[3 * size_t(index.normal_index) + 2];
                    normal += 1;
                    normal /= 2;
                }

                verticies.push_back({ { vx, vy, vz }, normal });
                indicies.push_back(verticies.size() - 1);
            }
            index_offset += fv;
        }
    }

    for (auto& v : verticies)
    {
        v.pos /= largestMagnitude;
    }

    return {verticies, indicies};
}
