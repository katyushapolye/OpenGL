//mega header for UI elements that wraps IMGUI
#ifndef UI_H
#define UI_H
#include "Definitions.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>



class UIElement{
public:
    virtual ~UIElement() = default;  
    virtual void draw() = 0;
};

class Text : public UIElement{  
public:
    Text(){
        content = "Default Text";
        pos = vec2(0.0f, 0.0f);
    }
    Text(const std::string& text, const vec2& position){
        content = text;
        pos = position;
    }
    std::string content;
    vec2 pos;
    
    void draw() override{
       // ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
        ImGui::Text("%s", content.c_str());
    }
};


class Slider : public UIElement{
    public:
    float* value;
    vec2 pos;
    float minValue;
    float maxValue;
    std::string label;

    Slider(){
        label = "DefaultSlider";
        minValue = 0.0f;
        maxValue = 1.0f;
        pos = vec2(0.0f, 0.0f);
    }
    

    Slider(const std::string& label, const vec2& position, float* val, float minVal, float maxVal){
        this->label = label;
        this->value = val;
        this->minValue = minVal;
        this->maxValue = maxVal;
        pos = position;
    }

    void draw() override{

        //ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);

    ImGui::SliderFloat(label.c_str(), value, minValue, maxValue, "%.4f", ImGuiSliderFlags_Logarithmic);

    };


};
#endif