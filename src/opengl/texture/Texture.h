//
// Created by faliszewskii on 06.05.24.
//

#ifndef PUMAROBOT_TEXTURE_H
#define PUMAROBOT_TEXTURE_H


#include <string>

class Texture {


    unsigned int id{};
    int nrChannels;
    int target, format, internalFormat;

public:
    Texture(int x, int y, int nrChannels, int internalFormat, int format, int type, int target, void* data);

    void bind(int slot) const;
    void bindImage(int slot) const;
    void update2D(void* data) const;
    void updatePixel(int x, int y, void* data) const;
    int width;
    int height;
    int depth;
    int type;
};


#endif //PUMAROBOT_TEXTURE_H
