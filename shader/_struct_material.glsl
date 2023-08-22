// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Material {
  vec4 diffuse;
  vec4 emission;

  // specular + shininess
  vec4 specular;

  uvec2 diffuseTex;
  uvec2 emissionTex;
  uvec2 specularTex;
  uvec2 normalMapTex;

  uvec2 dudvMapTex;
  uvec2 heightMapTex;
  uvec2 noiseMapTex;

  uvec2 roughessMapTex;
  uvec2 metalnessMapTex;
  uvec2 opacityMapTex;

  float ambient;

  int pattern;

  float reflection;
  float refraction;
  float refractionRatio;

  float tilingX;
  float tilingY;
  int layers;
  float depth;
};
