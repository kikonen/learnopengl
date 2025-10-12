#version 460 core

layout (binding = UNIT_EDITOR_CUBE_MAP)uniform samplerCube u_cubeMap;

uniform bool u_equirectangular;

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/54101329/project-cubemap-to-2d-texture
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void flat1() {
  vec4 color = vec4(0);
  vec2 localST = fs_in.texCoord;

  // Scale Tex coordinates such that each quad has local coordinates from 0,0 to 1,1
  localST.t = mod(localST.t * 3, 1);
  localST.s = mod(localST.s * 4, 1);

  // Due to the way my cubemap is rendered, objects to the -x,y,z side is
  // projected to the positive x,y,z side

  vec3 dir;

  if (fs_in.texCoord.s * 4>1 && fs_in.texCoord.s * 4<2) {
    // Inside where tob/bottom is to be drawn?

    // Bottom (-y) quad
    if (fs_in.texCoord.t * 3.f < 1) {
      // Get lower y texture, which is projected to the +y part of my cubemap
      dir = vec3(localST.s * 2-1, 1, localST.t * 2-1);
    } else if (fs_in.texCoord.t * 3.f > 2) {
      //top (+y) quad

      // Due to the (arbitrary) way I choose as up in my viewmatrix,
      // i her emultiply the latter coordinate with -1
      dir = vec3(localST.s * 2-1, -1, -localST.t * 2 + 1);
    } else {
      //Front (-z) quad
      dir = vec3(localST.s * 2-1, -localST.t * 2 + 1, 1);
    }
  } else if (fs_in.texCoord.t * 3.f > 1 && fs_in.texCoord.t * 3 < 2) {
    //If not, only these ranges should be drawn

    if (fs_in.texCoord.x * 4.f < 1) {
      //left (-x) quad
      dir = vec3(-1, -localST.t * 2 + 1, localST.s * 2-1);
    } else if (fs_in.texCoord.x * 4.f < 3) {
      //right (+x) quad (front was done above)
      dir = vec3(1, -localST.t * 2 + 1, -localST.s * 2 + 1);
    } else {
      //back (+z) quad
      dir = vec3(-localST.s * 2 + 1, -localST.t * 2 + 1, -1);
    }
  } else {
    // Tob/bottom, but outside where we need to put something

    // No need to add fancy semi transparant borders for quads,
    // this is just for debugging purpose after all
    discard;
  }

  color = texture(u_cubeMap, dir);
  o_fragColor = color;
}


void flat2() {
  float phi = fs_in.texCoord.s * 3.1415 * 2;
  float theta = (-fs_in.texCoord.t+0.5) * 3.1415;

  vec3 dir = vec3(cos(phi) * cos(theta), sin(theta), sin(phi) * cos(theta));

  // In this example i use a debthmap with only 1 channel,
  // but the projection should work with a colored cubemap to
  vec3 color = texture(u_cubeMap, dir).rgb;

  o_fragColor = vec4(color, 1);
}

void main() {
  if (u_equirectangular) {
    flat2();
  } else {
    flat1();
  }
}
