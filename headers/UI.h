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
        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Always);
        ImGui::Text("%s", content.c_str());
    }
};


class Slider : UIElement{
    float& value;
    float minValue;
    float maxValue;
    std::string label;

    void draw() override;


};
#endif