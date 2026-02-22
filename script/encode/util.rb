# frozen_string_literal: true

require_relative "shared"

#
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
  ############################################################
  # UTILS
  ############################################################
  module Util
    def initialize(
      recursive:
    )
      @recursive = recursive
    end

    def self.info(msg)
      puts msg
    end

    ####################
    # metadata
    ####################
    def self.read_metadata(
      src_dir:)
      metadata_path = "#{src_dir}/_assets.meta"
      return Metadata.new_empty unless File.exist?(metadata_path)

      info "READ: #{metadata_path}"

      data = JSON.parse(File.read(metadata_path), symbolize_names: true)

      data[:textures]&.each do |tex_info|
        tex_info.delete :target_type
        tex_info.delete :normal_map
      end

      Metadata.new(**data)
    end

    def self.write_metadata(
      src_dir:,
      data:,
      dry_run:
    )
      metadata_path = "#{src_dir}/_assets.meta"
      info "WRITE: #{metadata_path}"

      if dry_run
        info JSON.pretty_generate(data)
        return
      end

      File.write(metadata_path, JSON.pretty_generate(data) + "\n")
    end

    def self.list_files(src_dir)
      return [] unless File.directory?(src_dir)

       old_dir = Dir.pwd
      Dir.chdir(src_dir)
      files = Dir["*"]
      Dir.chdir(old_dir)

      files
    end

    def self.clean_dir_path(path)
      return path unless path.end_with?('/')
      path.gsub(/[\/]*\z/, "")
    end

    def self.scale_image(img, target_size)
      need_scale, target_w, target_h = resolve_size(img, target_size)
      return img unless need_scale

      img.resize(target_w, target_h)
    end

    #
    # Resolve scaled size
    #
    # NOTE KI iamge sizes may differ in width/height
    # => only scale down, not up
    #
    # @return [bool, w, h]
    #
    def self.resolve_size(img, target_size)
      # NOTE KI iamge sizes may differ in width/height
      # => only scale down, not up
      target_w = img.columns
      target_h = img.rows

      need_scale = false
      min_size = [target_w, target_h].min
      if min_size > target_size
        need_scale = true
        scale = target_size.to_f / min_size.to_f

        target_w = (target_w * scale).ceil
        target_h = (target_h * scale).ceil
      end

      [need_scale, target_w, target_h]
    end
  end
end
