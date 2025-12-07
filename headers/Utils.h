// util.h
#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H
#include <memory>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "Drawable.h"
#include "Model.h"
#include "Volumetric.h"
#include "Log.h"

namespace Utils {



    inline std::vector<float> readGrid_DEBUG(const char* path, int Ny, int Nx) {
        std::vector<float> grid(Ny * Nx);
        std::ifstream file(path);

        for (int i = 0; i < Ny; i++) {
            for (int j = 0; j < Nx; j++) {
                file >> grid[i * Nx + j];
                //std::cout << grid[i * Nx + j] << std::endl;
            }
        }

        file.close();
        return grid;
    }

inline std::vector<float> readGrid3D(const char* path, 
                                         int& Nx, int& Ny, int& Nz, float& dh)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return {};
    }

    // Header: [Nx, Ny, Nz, dh]
    float header[4];
    file.read(reinterpret_cast<char*>(header), sizeof(header));

    if (!file) {
        std::cerr << "Error: Failed to read header from " << path << std::endl;
        return {};
    }

    // Assign outputs (note: header stores floats, so cast)
    Nx = static_cast<int>(header[0]);
    Ny = static_cast<int>(header[1]);
    Nz = static_cast<int>(header[2]);
    dh = header[3];

    const size_t count = static_cast<size_t>(Nx) * Ny * Nz;

    if (count == 0) {
        std::cerr << "Error: Invalid grid dimensions in " << path << std::endl;
        return {};
    }

    // Allocate grid
    std::vector<float> grid(count);

    // Read the grid values
    file.read(reinterpret_cast<char*>(grid.data()), count * sizeof(float));

    if (!file) {
        std::cerr << "Error: Could not read full grid data from " << path << std::endl;
        std::cerr << "Expected " << count * sizeof(float) 
                  << " bytes of grid data." << std::endl;
        return {};
    }

    return grid;
}

    

    inline float getDrawableDistance2(const std::shared_ptr<Drawable>& drawable, const glm::vec3& camPos) {
        if (drawable->getType() == DrawableType::MODEL) {
            Model* model = static_cast<Model*>(drawable.get());
            return glm::length2(camPos - model->transform.getPosition());
        }
        else if (drawable->getType() == DrawableType::VOLUMETRIC) {
            Volumetric* vol = static_cast<Volumetric*>(drawable.get());
            return vol->distance2To(camPos);
        }
        else {
            Log::write("[RenderUtils::getDrawableDistance2] - WARNING! Transparent object without a transform encountered!");
            return 0.0f; // fallback distance
        }
    }


    inline bool compareDrawablesFarthestFirst(const std::shared_ptr<Drawable>& a, const std::shared_ptr<Drawable>& b, const glm::vec3& camPos) {
        float distA = getDrawableDistance2(a, camPos);
        float distB = getDrawableDistance2(b, camPos);
        return distA > distB; // farthest first
    }

} 

#endif