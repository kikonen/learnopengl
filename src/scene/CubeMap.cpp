#include "CubeMap.h"

#include "ki/GL.h"

#include "asset/Image.h"

//
// EMPTY cube map
//
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int CubeMap::createEmpty(int size)
{
    unsigned int textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    glTextureStorage2D(textureID, 1, GL_RGBA8, size, size);

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int CubeMap::createFromImages(std::vector<std::string> faces)
{
    unsigned int textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    int w = 0, h = 0;

    Image* images[6];

    for (unsigned int i = 0; i < 6; i++)
    {
        Image* image = new Image(faces[i]);
        images[i] = image;
        if (image->load(false)) continue;
        w = image->width;
        h = image->height;
    }

    glTextureStorage2D(textureID, 1, GL_RGBA8, w, h);

    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
    for (unsigned int i = 0; i < 6; i++)
    {
        Image* image = images[i];
        if (!image->data) continue;

        KI_GL_CALL(glTextureSubImage3D(textureID, 0, 0, 0, i, image->width, image->height, 1, GL_RGB, GL_UNSIGNED_BYTE, image->data));
    }

    for (unsigned int i = 0; i < 6; i++) {
        delete images[i];
    }

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
