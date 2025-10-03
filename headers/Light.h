// ===== Light.h =====
#ifndef LIGHT_H
#define LIGHT_H
#include "Definitions.h"
#include "Transform.h"

// Base Light class
class Light
{
protected:
    vec3 color;
    float intensity;
    LightType type;
    bool castShadow;
    
public:
    Transform transform;
    


    
    // Make Light abstract - no direct instances allowed
    virtual LightType getLightType() const = 0;

    //shadow mapping stuff
    virtual mat4 getViewMatrix() = 0;
    virtual mat4 getProjectionMatrix() = 0;

    
    // Getters
    const vec3& getColor() const { return color; }
    float getIntensity() const { return intensity; }
    LightType getType() const { return type; }
    
    // Setters
    void setColor(const vec3& c) { color = c; }
    void setIntensity(float i) { intensity = i; }
};

// Point Light class
class PointLight : public Light
{
private:
    float radius;  // Attenuation radius
    
public:
    PointLight(Transform transform, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
               float intensity = 1.0f, float radius = 10.0f);
    
    PointLight(vec3 position, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
               float intensity = 1.0f, float radius = 10.0f);
    
    // Getters/Setters
    float getRadius() const { return radius; }
    void setRadius(float r) { radius = r; }
    
    // Override pure virtual function
    LightType getLightType() const override { return LightType::POINT; }
    mat4 getViewMatrix() override;
    mat4 getProjectionMatrix() override;
};

// Spot Light class
class SpotLight : public Light
{
private:
    float theta;        // Inner cone angle (in degrees)
    float outerTheta;   // Outer cone angle (in degrees)
    float radius;       // Attenuation radius
    
public:
    SpotLight(Transform transform, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
              float intensity = 1.0f, float theta = 12.5f, 
              float outerTheta = 17.5f, float radius = 10.0f);
    
    SpotLight(vec3 position, vec3 direction, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
              float intensity = 1.0f, float theta = 12.5f, 
              float outerTheta = 17.5f, float radius = 10.0f);
    
    // Getters/Setters
    float getTheta() const { return theta; }
    float getOuterTheta() const { return outerTheta; }

    float getRadius() const { return radius; }
    
    void setTheta(float t) { theta = t; }
    void setOuterTheta(float ot) { outerTheta = ot; }
    void setRadius(float r) { radius = r; }
    
    // Override pure virtual function
    LightType getLightType() const override { return LightType::SPOT; }
    mat4 getViewMatrix() override;
    mat4 getProjectionMatrix() override;
};

// Directional Light class
class DirectionalLight : public Light
{
public:
    DirectionalLight(vec3 direction, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
                     float intensity = 1.0f);
    
    DirectionalLight(Transform transform, vec3 color = vec3(1.0f, 1.0f, 1.0f), 
                     float intensity = 1.0f);
    
    // Override pure virtual function
    LightType getLightType() const override { return LightType::DIRECTIONAL; }

    mat4 getViewMatrix() override;
    mat4 getProjectionMatrix() override;
};

#endif