//
// Created by faliszewskii on 16.06.24.
//

#ifndef OPENGL_TEMPLATE_GUI_H
#define OPENGL_TEMPLATE_GUI_H


#include "../../app/AppContext.h"

class Gui {
    AppContext &appContext;

public:
    Gui(AppContext &appContext);

    void render();

    void renderLightUI(PointLight &light);

    void renderMainMenu();

    void renderMenuItemLoadModel();

    void setupPath(const std::string& outPath);
};


#endif //OPENGL_TEMPLATE_GUI_H
