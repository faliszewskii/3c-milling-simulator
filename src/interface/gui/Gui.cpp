//
// Created by faliszewskii on 16.06.24.
//

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include "Gui.h"

#include <functional>
#include <numeric>

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
void openNfd(const std::function<void(const std::string &)> &func) {
    NFD_Init();

    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[] = { };
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 0;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        std::string s(outPath);
        func(s);
        NFD_FreePath(outPath);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
}

void saveNfd(const std::function<void(const std::string &)> &func) {
    NFD_Init();

    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[0] = { };
    nfdsavedialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 0;
    args.defaultName = "path.___";
    nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        std::string s(outPath);
        func(s);
        NFD_FreePath(outPath);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
}

void Gui::render() {
    ImGui::ShowDemoWindow();
    renderMainMenu();
    ImGui::Begin("3C Milling Simulator");
    ImGui::SeparatorText("Simulation");
    ImGui::BeginDisabled(appContext.mill->getPath().empty() || appContext.mill->pathFinished());
    if(!appContext.mill->isThreadRunning()) {
        if(ImGui::Button("Run")) {
            appContext.mill->startMilling(appContext.heightMap->heightMapData, appContext.baseDimensions);
        }
    } else {
        if(ImGui::Button("Stop")) {
            appContext.mill->signalStop();
        }
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(appContext.mill->getPath().empty() || appContext.mill->pathFinished() || appContext.mill->isThreadRunning());
    ImGui::SameLine();
    if(ImGui::Button("Instant")) {
        appContext.mill->startInstant(appContext.heightMap->heightMapData, appContext.baseDimensions);
//        auto e = appContext.mill->instant(appContext.heightMap->heightMapData, appContext.baseDimensions);
//        if(!e) {
//            appContext.running = false;
//            appContext.errorMessages.push_back(e.error());
//        }
//        appContext.heightMap->update();
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(appContext.mill->isThreadRunning());
    ImGui::SameLine();
    if(ImGui::Button("Reset Block##block")) {
        appContext.heightMap->resizeHeightMap(appContext.heightMap->heightMapSize, appContext.baseDimensions.y);
        appContext.running = false;
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset Mill##Mill")) {
        appContext.running = false;
        appContext.mill->setPosition(glm::vec3(0,  appContext.baseDimensions.y+10, 0));
        appContext.mill->reset();
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
    ImGui::EndDisabled();

    ImGui::SeparatorText("Visualization");
    ImGui::Checkbox("Draw Mill Path", &appContext.drawPath);
    ImGui::Checkbox("Draw Meshes", &appContext.drawMeshes);
    ImGui::Checkbox("Draw Mill", &appContext.drawMill);
    ImGui::Checkbox("Use color map", &appContext.useColorMap);

    ImGui::SeparatorText("Error Messages");
    char *errors = vectorToCharPointer(appContext.errorMessages);
    static ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
    ImGui::InputTextMultiline("##source", errors, totalLength(appContext.errorMessages), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
    delete(errors);

    ImGui::End();

    ImGui::Begin("Path Generation");
    ImGui::BeginDisabled(appContext.patchesC0.empty() && appContext.patchesC2.empty());

    ImGui::DragInt("Every nth path point", &appContext.everyNthPathPoint, 1, 1, 100);
    if(ImGui::Button("Generate K16 path")) {
        appContext.pathGenerator->generatePathK16();
    }
    if(ImGui::Button("Generate F10 path")) {
        appContext.pathGenerator->generatePathF10();
    }
    if(ImGui::Button("Generate Analytical F10 path")) {
        appContext.pathGenerator->generatePathAnalyticalF10();
    }
    if(ImGui::Button("Generate Analytical K08 path eye")) {
        appContext.pathGenerator->generatePathAnalyticalK08Eye();
    }
    if(ImGui::Button("Generate Analytical K08 path")) {
        appContext.pathGenerator->generatePathAnalyticalK08();
    }
    if(ImGui::Button("Generate Intersections K08 path")) {
        appContext.pathGenerator->generatePathAnalyticalK08Inter();
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(appContext.mill->getPath().empty());
    if(ImGui::Button("Save current path")) {
        saveNfd([&](const std::string &path) {
            GCodeExporter::parse(path, appContext.mill->getPath());
        });

    }
    ImGui::EndDisabled();

    pathManipulationUI();

    ImGui::End();

}

void Gui::pathManipulationUI() {
    ImGui::BeginDisabled(appContext.mill->getPath().empty());
    bool changed = false;

    if(ImGui::Button("Add mill rest position")) {
        auto path = appContext.mill->getPath();
        path.insert(path.begin(), {path.begin()->x, 66, path.begin()->z});
        path.insert(path.begin(), {0, 66, 0});
        path.emplace_back(path.rbegin()->x, 66, path.rbegin()->z);
        path.emplace_back(0, 66, 0);
        appContext.mill->setPath(path);
        std::vector<PositionVertex> vertices;
        std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                       [](glm::vec3 v){ return PositionVertex(v);});
        appContext.pathModel->update(std::move(vertices), std::nullopt);
        appContext.running = false;
    }


    glm::vec3 oldOffset = appContext.pathOffset;
    if(ImGui::DragFloat3("Path offset", glm::value_ptr(appContext.pathOffset), 0.1)) {
        changed = true;
        auto diff = appContext.pathOffset - oldOffset;
        auto path = appContext.mill->getPath();
        std::ranges::transform(path, path.begin(), [&](auto& elem) { return elem + diff;});
        appContext.mill->setPath(path);
        std::vector<PositionVertex> vertices;
        std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                       [](glm::vec3 v){ return PositionVertex(v);});
        appContext.pathModel->update(std::move(vertices), std::nullopt);
        appContext.running = false;
    }

    float oldScale = appContext.pathScale;
    if(ImGui::DragFloat("Path scale", &appContext.pathScale, 0.005, 0.02, 2)) {
        changed = true;
        auto diff = appContext.pathScale / oldScale;
        auto path = appContext.mill->getPath();
        glm::vec3 avg = std::accumulate(path.begin(), path.end(), glm::vec3{0.0}) / static_cast<float>(path.size());
        std::ranges::transform(path, path.begin(), [&](auto& elem) {
            return glm::vec3{(elem.x-avg.x)*diff+avg.x, elem.y, (elem.z-avg.z)*diff+avg.z};
        });
        appContext.mill->setPath(path);
        std::vector<PositionVertex> vertices;
        std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                       [](glm::vec3 v){ return PositionVertex(v);});
        appContext.pathModel->update(std::move(vertices), std::nullopt);
        appContext.running = false;
    }

    float oldRotation = appContext.pathRotation;
    if(ImGui::DragFloat("Path rotation", &appContext.pathRotation, 0.005, -std::numbers::pi, std::numbers::pi)) {
        changed = true;
        auto diff = appContext.pathRotation - oldRotation;
        auto path = appContext.mill->getPath();
        glm::vec3 avg = std::accumulate(path.begin(), path.end(), glm::vec3{0.0}) / static_cast<float>(path.size());
        std::cout<<"Diff: "<<diff<< ", Sin: " << std::sin(diff) << ", Cos: " << std::cos(diff) << std::endl;
        std::ranges::transform(path, path.begin(), [&](auto& elem) {
            return glm::vec3{
                ((elem.x-avg.x)*std::cos(diff) - (elem.z-avg.z)*std::sin(diff))+avg.x,
                elem.y,
                ((elem.x-avg.x)*std::sin(diff) + (elem.z-avg.z)*std::cos(diff))+avg.z
            };
        });
        appContext.mill->setPath(path);
        std::vector<PositionVertex> vertices;
        std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                       [](glm::vec3 v){ return PositionVertex(v);});
        appContext.pathModel->update(std::move(vertices), std::nullopt);
        appContext.running = false;
    }

    if(changed) {
        auto path = appContext.mill->getPath();
        path.erase(path.begin(), path.begin() +2);
        path.erase(path.end() - 2, path.end());
        path.insert(path.begin(), {path.begin()->x, 66, path.begin()->z});
        path.insert(path.begin(), {0, 66, 0});
        path.emplace_back(path.rbegin()->x, 66, path.rbegin()->z);
        path.emplace_back(0, 66, 0);
        appContext.mill->setPath(path);
        std::vector<PositionVertex> vertices;
        std::transform(path.begin(), path.end(), std::back_inserter(vertices),
                       [](glm::vec3 v){ return PositionVertex(v);});
        appContext.pathModel->update(std::move(vertices), std::nullopt);
        appContext.running = false;
    }

    ImGui::EndDisabled();
}

void Gui::renderLightUI(PointLight &light) {
    ImGui::ColorPicker3("Light Color", glm::value_ptr(light.color));
    ImGui::DragFloat3("Light Position", glm::value_ptr(light.position), 0.001f);
}

void Gui::renderMainMenu() {
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Import Path")) {
                openNfd([&](const std::string &path) {
                    setupPath(path);
                });
            }
            if(ImGui::MenuItem("Import Model")) {
                openNfd([&](const std::string &path) {
                    appContext.modelSerializer->importModel(appContext, path);
                });
            }
            if(ImGui::MenuItem("Import Helper")) {
                openNfd([&](const std::string &path) {
                    appContext.modelSerializer->importHelper(appContext, path, {});
                });
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
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
    appContext.mill->setHeight(millRadius*6);

    auto points = appContext.gCodeParser->parse(outPath);
    appContext.mill->setPath(points);
    appContext.lastFrameTime = glfwGetTime();

    std::vector<PositionVertex> vertices;
    std::transform(points.begin(), points.end(), std::back_inserter(vertices),
                   [](glm::vec3 v){ return PositionVertex(v);});
    appContext.pathModel->update(std::move(vertices), std::nullopt);

    appContext.pathOffset = {};
    appContext.pathScale = 1;
    appContext.pathRotation = 0;

    appContext.running = false;
}
