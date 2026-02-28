# frozen_string_literal: true

# TODO KI PBR matchers oboleted from framework
# => apply here
#
# TODO KI handle premade MRA mapps
# - fbx/scenery/foliage/VOL10_YardPlants/Textures/TX_DesertDandelion_01a_RMA_2048.png
# - fbx/scenery/foliage/VOL10_YardPlants/Textures/TX_Flower_01_RSA.PNG
#
#
# const std::vector<std::regex> ignoreMatchers{
#     std::regex(".*nope.*"),
#     std::regex(".*[\\.]blend"),
#     std::regex(".*[\\.]exr"),
#     std::regex(".*[\\.]txt"),
#     std::regex(".*[\\.]usda"),
#     std::regex(".*preview.*"),
#     std::regex(".*normaldx.*"),
#     std::regex(".*bc_neutral.*"),
#
#     std::regex(".*_micron[\\.].*"),
#     std::regex(".*_micronmask[\\.].*"),
#     std::regex(".*_resourcemap_position[\\.].*"),
#     std::regex(".*_resourcemap_wsnormal[\\.].*"),
#     std::regex(".*_sssmap[\\.].*"),
#     std::regex(".*_transmap[\\.].*"),
# };
#
# const std::vector<std::regex> imageMatchers{
#     std::regex(".*[\\.]hdr"),
#     std::regex(".*[\\.]png"),
#     std::regex(".*[\\.]jpg"),
#     std::regex(".*[\\.]tga"),
# };
#
# const std::vector<std::regex> colorMatchers{
#     std::regex(".*[-_ ]color[-_ \\.].*"),
#     std::regex(".*[-_ ]col[-_ \\.].*"),
#     std::regex(".*[-_ ]basecolor[-_ \\.].*"),
#     std::regex(".*[-_ ]diff[-_ \\.].*"),
#     std::regex(".*[-_ ]alb[-_ \\.].*"),
#     std::regex(".*[-_ ]albedo[-_ \\.].*"),
#     std::regex(".*[-_ ]albedoopacity[-_ \\.].*"),
#     std::regex(".*[-_ ]albedotransparency[-_ \\.].*"),
#     std::regex(".*[-_ ]basecoloralpha[-_ \\.].*"),
#     std::regex(".*[-_ ]a[\\.].*"),
#     std::regex(".*[-_ ]c[\\.].*"),
#     std::regex(".*[-_ ]bc[\\.].*"),
#     std::regex(".*[-_ ]a_m[\\.].*"),
#     std::regex(".*[-_ ]b[\\.].*"),
# };
#
# const std::vector<std::regex> emissionMatchers{
#     std::regex(".*[-_ ]emission[-_ \\.].*"),
#     std::regex(".*[-_ ]emi[-_ \\.].*"),
#     std::regex(".*[-_ ]emissive[-_ \\.].*"),
# };
#
# const std::vector<std::regex> normalMatchers{
#     std::regex(".*[-_ ]normal[-_ \\.].*"),
#     std::regex(".*[-_ ]normals[-_ \\.].*"),
#     std::regex(".*[-_ ]normalgl[-_ \\.].*"),
#     std::regex(".*[-_ ]nrm[-_ \\.].*"),
#     std::regex(".*[-_ ]nor[-_ \\.].*"),
#     std::regex(".*[-_ ]nor[-_ \\.].*"),
#     std::regex(".*[-_ ]nml[-_ \\.].*"),
#     std::regex(".*[-_ ]n[\\.].*"),
# };
#
# const std::vector<std::regex> metalnessMatchers{
#     std::regex(".*[-_ ]metalness[-_ \\.].*"),
#     std::regex(".*[-_ ]met[-_ \\.].*"),
#     std::regex(".*[-_ ]metallic[-_ \\.].*"),
#     // TODO KI logic various random combined texture formats
#     std::regex(".*[-_ ]metallicsmoothness[-_ \\.].*"),
#     //std::regex(".*[-_ ]occlusionroughnessmetallic[-_ \\.].*"),
#     //std::regex(".*[-_ ]aorm[\\.].*"),
#     //std::regex(".*[-_ ]rom[\\.].*"),
# };
#
# const std::vector<std::regex> roughnessMatchers{
#     std::regex(".*[-_ ]roughness[-_ \\.].*"),
#     std::regex(".*[-_ ]rough[-_ \\.].*"),
#     std::regex(".*[-_ ]rgh[-_ \\.].*"),
# };
#
# const std::vector<std::regex> occlusionMatchers{
#     std::regex(".*[-_ ]ambientocclusion[-_ \\.].*"),
#     std::regex(".*[-_ ]occlusion[-_ \\.].*"),
#     std::regex(".*[-_ ]ao[-_ \\.].*"),
# };
#
# const std::vector<std::regex> displacementMatchers{
#     std::regex(".*[-_ ]displacement[-_ \\.].*"),
#     std::regex(".*[-_ ]disp[-_ \\.].*"),
#     std::regex(".*[-_ ]depth[-_ \\.].*"),
# };
#
# const std::vector<std::regex> opacityMatchers{
#     std::regex(".*[-_ ]opacity[-_ \\.].*"),
#     std::regex(".*[-_ ]ops[-_ \\.].*"),
#     std::regex(".*[-_ ]alpha[-_ \\.].*"),
# };
#

module Encode
  EXTENSIONS = ["png", "jpg", "jpeg", "tga"].freeze

  OPACITY_MAP = 'opacity'
  MRAS_MAP = 'mras'
  MADS_MAP = "mads"
  DISPLACEMENT_MAP = 'displacement'
  CAVITY_MAP = 'cavity'

  BUILD_SUFFIX = '_build'

  NONE = '_'
  RED = 'R'
  GREEN = 'G'
  BLUE = 'B'
  ALPHA = 'A'
  RGB = 'RGB'
  RGBA = 'RGBA'

  RED_GREEN = 'RG'
  GREEN_BLUE = 'GB'
  RED_GREEN_BLUE = 'RGB'
  GREEN_RED_BLUE = 'GRB'
  GREEN_BLUE_RED = 'GBR'
  BLUE_GREEN_RED = 'BGR'
  BLUE_RED_GREEN = 'BRG'
  RED_BLUE_GREEN = 'RBG'
  RED_GREEN_ALPHA = 'RGA'
  RED_ALPHA_GREEN = 'RAG'
  RED_GREEN_BLUE_ALPHA = 'RGBA'

  # MRA + KHR_materials_specular
  # MRAS: [metalness, roughness, ambient-occlusion, specular]
  # - metalness (Red): 0 = dielectric, 1 = metal
  # - roughness (Gren):  0 = smooth/shiny, 1 = rough/matte
  # - occlusion (Blue):   0 = fully occluded, 1 = no occlusion
  # - specular  (Alpha): 0 = no reflection, 1 = strong reflection
  MODE_MRAS = :mras

  # MADS is a channel-packed texture format used by KitBash3D where
  # four grayscale maps are packed into the RGBA channels of a single texture:
  #
  # M (Red) — Metallic
  # A (Green) — Ambient Occlusion
  # D (Blue) — Detail / Displacement
  # S (Alpha) — Smoothness (inverse of roughness)
  MODE_MADS = :mads

  MODE_OPACITY = :opacity
  MODE_DISPLACEMENT = :displacement

  IMAGE_VERSION = 8
  MRAS_VERSION = 9
  DISPLACEMENT_VERSION = 8
  KTX_VERSION = 11
end
