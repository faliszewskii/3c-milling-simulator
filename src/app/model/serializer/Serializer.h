//
// Created by faliszewskii on 12.05.24.
//

#ifndef OPENGL_SANDBOX_SERIALIZER_H
#define OPENGL_SANDBOX_SERIALIZER_H
#include <string>

struct AppContext;

class Serializer {


public:
    void importScene(AppContext &appContext, const std::string &path);
};


#endif //OPENGL_SANDBOX_SERIALIZER_H
