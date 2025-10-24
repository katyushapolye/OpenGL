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

    inline std::vector<float> readGrid3D_Binary(const char* path, int& Nz, int& Ny, int& Nx, float& dh) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << path << std::endl;
            return std::vector<float>();
        }

        // Read header: [Nx, Ny, Nz, dh]
        float header[4];
        file.read(reinterpret_cast<char*>(header), 4 * sizeof(float));

        if (!file) {
            std::cerr << "Error: Could not read header from " << path << std::endl;
            return std::vector<float>();
        }

        // Extract dimensions and spacing from header
        Nx = static_cast<int>(header[0]);
        Ny = static_cast<int>(header[1]);
        Nz = static_cast<int>(header[2]);
        dh = header[3];

        //std::cout << "Grid dimensions: " << Nx << " x " << Ny << " x " << Nz << std::endl;
        //std::cout << "Grid spacing: " << dh << std::endl;

        // Allocate grid and read data
        std::vector<float> grid(Nz * Ny * Nx);
        file.read(reinterpret_cast<char*>(grid.data()), Nz * Ny * Nx * sizeof(float));

        if (!file) {
            std::cerr << "Error: Could not read complete grid data from " << path << std::endl;
            std::cerr << "Expected " << (Nz * Ny * Nx * sizeof(float)) << " bytes" << std::endl;
            return std::vector<float>();
        }

        file.close();

        //std::cout << "Successfully loaded grid with " << grid.size() << " points" << std::endl;

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