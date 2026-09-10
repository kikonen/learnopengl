# frozen_string_literal: true

require "amazing_print"

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
      info "SAVE: #{metadata_path}"

      if dry_run
        info JSON.pretty_generate(data)
        return
      end

      data = JSON.parse(data.to_json, symbolize_names: true)
      data[:textures] = data[:textures].map(&:compact)

      File.write(metadata_path, JSON.pretty_generate(data) + "\n")
    end

    def self.list_files(src_dir, pattern = "*")
      return [] unless File.directory?(src_dir)

      Dir.glob(pattern, base: src_dir).sort
    end

    def self.clean_dir_path(path)
      return path unless path.end_with?('/')
      path.gsub(/[\/]*\z/, "")
    end

    #
    # Extent image canvas to target_size
    #
    #
    # Extent image canvas to target_size
    #
    def self.extent_image(img, type, target_size)
      # ==========================================
      # RESOLVE NEUTRAL BACKGROUND COLOR FOR EXTENT
      # ==========================================

      return img unless img.columns != target_size || img.rows != target_size

      # Max quantum intensity value for fully opaque channel pixel mapping (16-bit)
      opaque_alpha = 65535

      if type == :normal || type == :dudv
        # Flat Normal vector (128, 128, 255) with fully opaque alpha layout.
        # This keeps the image strictly at 3 channels (RGB) without adding alpha clutter!
        bg_pixel = Magick::Pixel.new(128 * 257, 128 * 257, 255 * 257, opaque_alpha)

      elsif type == :mras
        # Neutral PBR factor state (0, 255, 255) with fully opaque alpha container setup.
        # This keeps the MRAS data tight at 3 (or 4) channels depending on your structural needs.
        bg_pixel = Magick::Pixel.new(0, 255 * 257, 255 * 257, opaque_alpha)

      elsif type == :displacement || type == :noise || type == :height
        # Pure opaque black placeholder fill layer for raw single channel mathematical maps
        bg_pixel = Magick::Pixel.new(0, 0, 0, opaque_alpha)

      else
        # Diffuse, Emission, and Spritesheets require a true transparent black canvas padding area
        bg_pixel = Magick::Pixel.new(0, 0, 0, 0)
      end

      # Assign the strictly typed explicit background pixel mapping template
      img.background_color = bg_pixel

      # Pad the boundaries safely without destroying preexisting embedded alpha data
      img = img.extent(target_size, target_size, 0, 0)

      # Force final colorspace target conversions ONLY after the extent pass is fully completed
      if type == :diffuse || type == :emission
        img.colorspace = Magick::SRGBColorspace
      else
        img.colorspace = Magick::RGBColorspace
      end

      img
    end

    def self.scale_diffuse_image(img, target_size, max)
      resolved = resolve_size(img, target_size, max)
      return img unless resolved[:need_scale]
      target_w = resolved[:w]
      target_h = resolved[:h]

      # save colorspace
      orig_colorspace = img.colorspace
      img.colorspace = Magick::RGBColorspace

      # NOTE KI use Lanczos-filter (should be better for diffuse textures)
      resized_img = img.resize(target_w, target_h, Magick::LanczosFilter)

      # restore colorspace
      img.colorspace = orig_colorspace
      resized_img.colorspace = orig_colorspace

      resized_img
    end

    # Skaalausmetodi datakartoille (Normal, MRA, DuDv, Displacement, jne.)
    def self.scale_data_image(img, target_size, max)
      resolved = resolve_size(img, target_size, max)
      return img unless resolved[:need_scale]
      target_w = resolved[:w]
      target_h = resolved[:h]

      # NOTE don't touch color space; (raw data / RGBColorspace)

      # Use Cubic-filter, reduces raw data corruption
      resized_img = img.resize(target_w, target_h, Magick::CubicFilter)

      resized_img
    end

    # scaling noise data
    def self.scale_nearest_image(img, target_size, max)
      resolved = resolve_size(img, target_size, max)
      return img unless resolved[:need_scale]
      target_w = resolved[:w]
      target_h = resolved[:h]

      # NOTE don't touch color space; (raw data / RGBColorspace)

      # Point filter for "noise"
      resized_img = img.resize(target_w, target_h, Magick::PointFilter)

      resized_img
    end

    def self.resolve_size(img, target_size, max)
      if max
        resolve_max_size(img, target_size)
      else
        resolve_min_size(img, target_size)
      end
    end

    #
    # Resolve scaled size based into min size
    #
    # NOTE KI iamge sizes may differ in width/height
    # => scaling done BOTH ways for sampler2dArray support
    #
    # @return [bool, w, h]
    #
    #
    # Resolve scaled size
    #
    # NOTE KI image sizes may differ in width/height
    # => normalized strictly into target_size (supports both scale down AND up)
    #
    # @return [bool, w, h]
    #
    def self.resolve_min_size(img, target_size)
      img_w = img.columns
      img_h = img.rows

      min_size = [img_w, img_h].min

      # FIX: Check if the size differs from target_size in EITHER direction
      if min_size != target_size
        scale = target_size.to_f / min_size.to_f

        target_w = (img_w * scale).ceil
        target_h = (img_h * scale).ceil

        { w: target_w, h: target_h, need_scale: true, scale: }
      else
        { w: img_w, h: img_h, need_scale: false, scale: 1.0 }
      end
    end

    #
    # Resolve scaled based into max size
    #
    # NOTE KI iamge sizes may differ in width/height
    # => scaling done BOTH ways for sampler2dArray support
    #
    # @return [bool, w, h]
    #
    #
    # Resolve scaled size
    #
    # NOTE KI image sizes may differ in width/height
    # => normalized strictly into target_size (supports both scale down AND up)
    #
    # @return [bool, w, h]
    #
    def self.resolve_max_size(img, target_size)
      img_w = img.columns
      img_h = img.rows

      max_size = [img_w, img_h].max

      # FIX: Check if the size differs from target_size in EITHER direction
      if max_size != target_size
        scale = target_size.to_f / max_size.to_f

        target_w = (img_w * scale).ceil
        target_h = (img_h * scale).ceil

        { w: target_w, h: target_h, need_scale: true, scale: }
      else
        { w: img_w, h: img_h, need_scale: false, scale: 1.0 }
      end
    end
  end
end
