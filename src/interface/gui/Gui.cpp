//
// Created by faliszewskii on 16.06.24.
//

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include "Gui.h"
#include "imgui.h"
#include "nfd.h"

Gui::Gui(AppContext &appContext) : appContext(appContext) {}

int totalLength(const std::vector<std::string>& strVec) {
    size_t totalSize = 0;
    for (const std::string& str : strVec) {
        totalSize += str.size() + 1;  // +1 for the '\n'
    }
    return totalSize;
}
char* vectorToCharPointer(const std::vector<std::string>& strVec) {
    // Calculate the total size needed


    // Allocate memory for the char array
    char* result = new char[totalLength(strVec) + 1];  // +1 for the null terminator
    result[0] = '\0';  // Initialize as an empty string

    // Copy each string and append a newline
    for (const std::string& str : strVec) {
        strcat(result, str.c_str());  // Append the string
        strcat(result, "\n");         // Append newline
    }

    return result;
}

void Gui::render() {
    ImGui::ShowDemoWindow();
    renderMainMenu();
    ImGui::Begin("3C Milling Simulator");
    ImGui::SeparatorText("Simulation");
    ImGui::BeginDisabled(appContext.mill->getPath().empty());
    if(ImGui::Button(appContext.running?"Stop":"Run")) {
        appContext.running = !appContext.running;
    }
    ImGui::SameLine();
    if(ImGui::Button("Instant")) {
        auto e = appContext.mill->instant(appContext.heightMap->heightMapData, appContext.baseDimensions);
        if(!e) {
            appContext.running = false;
            appContext.errorMessages.push_back(e.error());
        }
        appContext.heightMap->update();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Reset Block##block")) {
        appContext.heightMap->resizeHeightMap(appContext.heightMap->heightMapSize, appContext.baseDimensions.y);
        appContext.running = false;
    }
    float velocity = appContext.mill->getVelocity();
    if(ImGui::DragFloat("Velocity (mm/s)", &velocity, 1, 1, 1000))
        appContext.mill->setVelocity(velocity);

    ImGui::SeparatorText("Material");
    bool heightMapResChanged = false;
    heightMapResChanged |= ImGui::DragInt("Resolution X", glm::value_ptr(appContext.heightMap->heightMapSize)+0);
    heightMapResChanged |= ImGui::DragInt("Resolution Y", glm::value_ptr(appContext.heightMap->heightMapSize)+1);
    heightMapResChanged |= ImGui::DragFloat("Base size x (mm)", glm::value_ptr(appContext.baseDimensions)+0, 1, 10, 500);
    heightMapResChanged |= ImGui::DragFloat("Base size y (mm)", glm::value_ptr(appContext.baseDimensions)+1, 1, 10, 100);
    heightMapResChanged |= ImGui::DragFloat("Base size z (mm)", glm::value_ptr(appContext.baseDimensions)+2, 1, 10, 500);
    float minheight = appContext.mill->getMinHeight();
    if(ImGui::DragFloat("Base Min Height (mm)", &minheight, 1, 0, appContext.baseDimensions.y))
        appContext.mill->setMinHeight(minheight);

    if(heightMapResChanged) {
        if(appContext.mill->getMinHeight() > appContext.baseDimensions.y)
            appContext.mill->setMinHeight(appContext.baseDimensions.y);
        appContext.heightMap->resizeHeightMap(appContext.heightMap->heightMapSize, appContext.baseDimensions.y);
        appContext.running = false;
        appContext.mill->setPosition(glm::vec3(0,  appContext.baseDimensions.y+10, 0));
        appContext.mill->reset();
    }

    ImGui::SeparatorText("Mill");
    if(ImGui::Button("Reset Mill##Mill")) {
        appContext.running = false;
        appContext.mill->setPosition(glm::vec3(0,  appContext.baseDimensions.y+10, 0));
        appContext.mill->reset();
    }
    ImGui::Text("Type: ");
    ImGui::SameLine();
    if(ImGui::RadioButton("Flat", appContext.mill->getType() == Flat)) {
        appContext.mill->setType(Flat);
    }
    ImGui::SameLine();
    if(ImGui::RadioButton("Spherical", appContext.mill->getType() == Spherical)) {
        appContext.mill->setType(Spherical);
    }

    float radius = appContext.mill->getRadius();
    if(ImGui::DragFloat("Radius (mm)", &radius, 0.5, 0.5, 50)) {
        appContext.mill->setRadius(radius);
        if(appContext.mill->getHeight() < 2*appContext.mill->getRadius())
            appContext.mill->setHeight(2*appContext.mill->getRadius());
    }
    float height = appContext.mill->getHeight();
    if(ImGui::DragFloat("Height (mm)", &height, 0.5, 0.5, 100)) {
        appContext.mill->setHeight(height);
        if(appContext.mill->getRadius() > 0.5f*appContext.mill->getHeight())
            appContext.mill->setRadius(0.5f*appContext.mill->getHeight());
    }

    ImGui::SeparatorText("Visualization");
    ImGui::Checkbox("Draw Mill Path", &appContext.drawPath);

    ImGui::SeparatorText("Error Messages");
    char *errors = vectorToCharPointer(appContext.errorMessages);
    static ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
    ImGui::InputTextMultiline("##source", errors, totalLength(appContext.errorMessages), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
    delete(errors);

    // renderLightUI(*appContext.light);
    ImGui::End();
}

void Gui::renderLightUI(PointLight &light) {
    ImGui::ColorPicker3("Light Color", glm::value_ptr(light.color));
    ImGui::DragFloat3("Light Position", glm::value_ptr(light.position), 0.001f);
}

void Gui::renderMainMenu() {
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Import")) {
                renderMenuItemLoadModel();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void Gui::renderMenuItemLoadModel() {
    NFD_Init();

    nfdchar_t *outPath;
    nfdfilteritem_t filterItem[0]{/*{*//*"G-code", "f..,k.."*//*}*/};
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 0, NULL);
    if (result == NFD_OKAY) {
        try {
            setupPath(outPath);
        } catch (std::exception &ex) {
            // TODO Log error to log window.
            std::cerr << ex.what() << std::endl;
        }
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
//        }
}

void Gui::setupPath(const std::string& outPath) {
    std::string millType = outPath.substr(outPath.size()-3, 1);
    switch(millType[0]) {
        case 'k':
            appContext.mill->setType(Spherical);
            break;
        case 'f':
            appContext.mill->setType(Flat);
            break;
        default:
            throw std::runtime_error("Wrong file extension");
    }

    std::string millDiameter = outPath.substr(outPath.size()-2, 2);
    float millRadius = std::stoi(millDiameter) / 2.f;
    appContext.mill->setRadius(millRadius);
    appContext.mill->setHeight(millRadius*4);

    auto points = appContext.gCodeParser->parse(outPath);
    appContext.mill->setPath(points);
    appContext.lastFrameTime = glfwGetTime();

    std::vector<PositionVertex> vertices;
    std::transform(points.begin(), points.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);

    appContext.running = false;
}
