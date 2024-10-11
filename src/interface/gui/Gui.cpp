//
// Created by faliszewskii on 16.06.24.
//

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include "Gui.h"
#include "imgui.h"
#include "nfd.h"

Gui::Gui(AppContext &appContext) : appContext(appContext) {}

void Gui::render() {
    ImGui::Begin("3C Milling Simulator");
    renderMainMenu();
    renderLightUI(*appContext.light);
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

    auto points = appContext.gCodeParser->parse(outPath);
    appContext.mill->setPath(points);
    appContext.lastFrameTime = glfwGetTime();

    std::vector<PositionVertex> vertices;
    std::transform(points.begin(), points.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);
}
