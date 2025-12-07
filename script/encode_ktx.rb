# frozen_string_literal: true

require 'open3'
require 'debug'
require 'yaml'
require 'digest'
require 'pathname'
require 'thor'
require 'fileutils'
require 'json'
require 'rmagick'
require 'hashie'

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

# NOTE KI using Hashie
class TextureInfo < Hashie::Dash
  include Hashie::Extensions::MergeInitializer
  include Hashie::Extensions::MethodAccess
  include Hashie::Extensions::Dash::PropertyTranslation

  property :name
  property :target_name
  property :group
  property :type
  property :action
  property :mode
  property :detected_channels
  property :source_channel
  property :target_channel
  property :source_depth
  property :target_depth
  property :source_size
  property :srgb
  property :manual

  def action_sym
    self.action&.to_sym
  end

  def src_path(src_dir)
    "#{src_dir}/#{self.name}"
  end
end

class Metadata < Hashie::Dash
  include Hashie::Extensions::Coercion
  include Hashie::Extensions::MergeInitializer
  include Hashie::Extensions::MethodAccess

  coerce_key :textures, Array[TextureInfo]

  property :dir
  property :textures

  def self.new_empty
    self.new(textures: [])
  end
end

class Converter < Thor
  EXTENSIONS = ["png", "jpg", "jpeg", "tga"].freeze

  MRAS_MAP = 'mras'
  DISPLACEMENT_MAP = 'displacement'
  CAVITY_MAP = 'cavity'

  BUILD_SUFFIX = '_build'

  RED = 'R'
  GREEN = 'G'
  BLUE = 'B'
  ALPHA = 'A'
  RGB = 'RGB'
  RGBA = 'RGBA'

  RED_GREEN = 'RG'
  GREEN_BLUE = 'GB'
  RED_GREEN_BLUE = 'RGB'
  GREEN_RED_BLUE = 'GBB'
  GREEN_BLUE_RED = 'GBR'
  BLUE_GREEN_RED = 'BGR'
  BLUE_RED_GREEN = 'BRG'
  RED_BLUE_GREEN = 'RBG'

  MAGICK_CHANNELS = {
    RED => Magick::RedChannel,
    GREEN => Magick::GreenChannel,
    BLUE => Magick::BlueChannel,
    ALPHA => Magick::AlphaChannel,
  }.freeze

  # MRA + KHR_materials_specular
  # MRAS: [ambient-occlusion, metalness, roughness, specular]
  # - metalness: 0 = dielectric, 1 = metal
  # - roughness: 0 = smooth/shiny, 1 = rough/matte
  # - occlusion: 0 = fully occluded, 1 = no occlusion
  # - specular:  0 = no reflection, 1 = strong reflection
  MODE_MRAS = :mras
  MODE_DISPLACEMENT = :displacement

  COMBINE_VERSION = 1
  KTX_VERSION = 1

  # NOTE KI match against *basename" without *extension*
  # => this \z instead of \.
  REG_EX = [
    # Level 1: most accurate
    {
      color: [
        /\Acolor\z/,
        /\Acolor[-_ ]/,
        /[-_ ]color[-_ ]/,
        /[-_ ]color\z/,
        ###
        /\Abasecolor\z/,
        /\Abasecolor[-_ ]/,
        /[-_ ]basecolor[-_ ]/,
        /[-_ ]basecolor/,
        /[-_ ]basecolor\z/,
        ###
        /\Adiffuse\z/,
        /\Adiffuse[-_ ]/,
        /[-_ ]diffuse[-_ ]/,
        /[-_ ]diffuse\z/,
        ###
        /\Aalbedo\z/,
        /\Aalbedo[-_ ]/,
        /[-_ ]albedo/,
        /[-_ ]albedo\z/,
        ###
      ],
      emission: [
        /\Aemission\z/,
        /\Aemission[-_ ]/,
        /[-_ ]emission[-_ ]/,
        /[-_ ]emission\z/,
        ###
        /\Aemissive\z/,
        /\Aemissive[-_ ]/,
        /[-_ ]emissive[-_ ]/,
        /[-_ ]emissive\z/,
        ###
        /\Aglow\z/,
        /\Aglow[-_ ]/,
        /[-_ ]glow[-_ ]/,
        /[-_ ]glow\z/,
        ###
      ],
      opacity: [
        /\Aopacity\z/,
        /\Aopacity[-_ ]/,
        /[-_ ]opacity[-_ ]/,
        /[-_ ]opacity\z/,
        ###
        /\Aalpha\z/,
        /\Aalpha[-_ ]/,
        /[-_ ]alpha[-_ ]/,
        /[-_ ]alpha\z/,
        ###
      ],
      normal: [
        /[-_ ]normalgl[-_ ]/,
        /[-_ ]normalgl\z/,
        ###
        /\Anormal\z/,
        /\Anormal[-_ ]/,
        /[-_ ]normal[-_ ]/,
        /[-_ ]normal\z/,
        ###
        /\Anormals\z/,
        /\Anormals[-_ ]/,
        /[-_ ]normals[-_ ]/,
        /[-_ ]normals\z/,
        ###
      ],
      bump: [
        /\Abump\z/,
        /\Abump[-_ ]/,
        /[-_ ]bump[-_ ]/,
        /[-_ ]bump\z/,
        ###
      ],
      metal: [
        /\Ametalness\z/,
        /\Ametalness[-_ ]/,
        /[-_ ]metalness[-_ ]/,
        /[-_ ]metalness\z/,
        ###
        /\Ametallic\z/,
        /\Ametallic[-_ ]/,
        /[-_ ]metallic[-_ ]/,
        /[-_ ]metallic\z/,
        ###
        /[-_ ]metallicsmoothness/,
        ###
        /\Ametalness\z/,
        /\Ametalness[-_ ]/,
        /[-_ ]metalness[-_ ]/,
        /[-_ ]metalness\z/,
      ],
      roughness: [
        /\Aroughness\z/,
        /\Aroughness[-_ ]/,
        /[-_ ]roughness[-_ ]/,
        /[-_ ]roughness\z/,
      ],
      occlusion: [
        /\Aambientocclusion\z/,
        /\Aambientocclusion[-_ ]/,
        /[-_ ]ambientocclusion[-_ ]/,
        /[-_ ]ambientocclusion\z/,
        ###
        /\Aocclusion\z/,
        /\Aocclusion[-_ ]/,
        /[-_ ]occlusion[-_ ]/,
        /[-_ ]occlusion\z/,
      ],
      metal_roughness: [
        /metallicroughness/,
      ],
      metal_roughness_occlusion: [
        /metallicroughnessocclusion/,
      ],
      roughness_metal_occlusion: [
        /roughnessmetallicocclusion/,
      ],
      roughness_occlusion_metal: [
        /roughnessocclusionmetal/,
      ],
      occlusion_roughness_metal: [
        /occlusionroughnessmetal/,
      ],
      displacement: [
        /\Adisplacement\z/,
        /\Adisplacement[-_ ]/,
        /[-_ ]displacement[-_ ]/,
        /[-_ ]displacement\z/,
        ###
        /\Adepth\z/,
        /\Adepth[-_ ]/,
        /[-_ ]depth[-_ ]/,
        /[-_ ]depth\z/,
        ###
      ],
      cavity: [
        /\Acavity\z/,
        /\Acavity[-_ ]/,
        /[-_ ]cavity[-_ ]/,
        /[-_ ]cavity\z/,
      ],
      specular: [
        /\Aspecular\z/,
        /\Aspecular[-_ ]/,
        /[-_ ]specular[-_ ]/,
        /[-_ ]specular\z/,
      ],
      gloss: [
        /\Agloss\z/,
        /\Agloss[-_ ]/,
        /[-_ ]gloss[-_ ]/,
        /[-_ ]gloss\z/,
      ],
      noise: [
        /\Anoise\z/,
        /\Anoise[-_ ]/,
        /[-_ ]noise[-_ ]/,
        /[-_ ]noise\z/,
      ],
      preview: [
        /preview/,
      ],
    },
    # Level 2; less accurate
    {
      color: [
        /\Acol\z/,
        /\Acol[-_ ]/,
        /[-_ ]col[-_ ]/,
        /[-_ ]col\z/,
        ###
        /\Aalb\z/,
        /\Aalb[-_ ]/,
        /[-_ ]alb[-_ ]/,
        /[-_ ]alb\z/,
        ###
        /\Adiff\z/,
        /\Adiff[-_ ]/,
        /[-_ ]diff[-_ ]/,
        /[-_ ]diff\z/,
        ###
      ],
      emission: [
        /\Aemi\z/,
        /\Aemi[-_ ]/,
        /[-_ ]emi[-_ ]/,
        /[-_ ]emi\z/,
        ###
      ],
      opacity: [
      ],
      normal: [
        /\Anrm\z/,
        /\Anrm[-_ ]/,
        /[-_ ]nrm[-_ ]/,
        /[-_ ]nrm\z/,
        ###
        /\Anml\z/,
        /\Anml[-_ ]/,
        /[-_ ]nml[-_ ]/,
        /[-_ ]nml\z/,
        ###
        /\Anor\z/,
        /\Anor[-_ ]/,
        /[-_ ]nor[-_ ]/,
        /[-_ ]nor\z/,
        ###
      ],
      metal: [
        /\Amet\z/,
        /\Amet[-_ ]/,
        /[-_ ]met[-_ ]/,
        /[-_ ]met\z/,
        ###
      ],
      roughness: [
        /\Argh\z/,
        /\Argh[-_ ]/,
        /[-_ ]rgh[-_ ]/,
        /[-_ ]rgh\z/,
        ###
        /\Arough\z/,
        #/\Arough[-_ ]/,
        /[-_ ]rough[-_ ]/,
        /[-_ ]rough\z/,
        ###
      ],
      occlusion: [
      ],
      metal_roughness_occlusion: [
        /\Amra\z/,
        /\Amra[-_ ]/,
        /[-_ ]mra[-_ ]/,
        /[-_ ]mra\z/,
      ],
      roughness_metal_occlusion: [
        /\Arma\z/,
        /\Arma[-_ ]/,
        /[-_ ]rma[-_ ]/,
        /[-_ ]rma\z/,
      ],
      roughness_occlusion_metal: [
        /\Arom\z/,
        /\Arom[-_ ]/,
        /[-_ ]rom[-_ ]/,
        /[-_ ]rom\z/,
      ],
      occlusion_roughness_metal: [
        /\Aorm\z/,
        /\Aorm[-_ ]/,
        /[-_ ]orm[-_ ]/,
        /[-_ ]orm\z/,
      ],
      displacement: [
        /\Adisp\z/,
        /\Adisp[-_ ]/,
        /[-_ ]disp[-_ ]/,
        /[-_ ]disp\z/,
      ],
      specular: [
        /\Aspec\z/,
        /\Aspec[-_ ]/,
        /[-_ ]spec[-_ ]/,
        /[-_ ]spec\z/,
      ],
      gloss: [
      ],
      preview: [
      ],
    },
    # Level 3; even less accurate (TLA, OLA)
    {
      color: [
        /\Abc\z/,
        /\Abc[-_ ]/,
        /[-_ ]bc[-_ ]/,
        /[-_ ]bc\z/,
        ###
        /\Ac\z/,
        /[-_ ]c\z/,
        ###
        /\Aa\z/,
        /[-_ ]a[-_ ]\z/,
        /[-_ ]a\z/,
        ###
        /\Aa_m\z/,
        /[-_ ]a_m[-_ ]\z/,
        /[-_ ]a_m\z/,
        ###
        /\Ab\z/,
        /[-_ ]b\z/,
        ###
        /\Ad\z/,
        /[-_ ]d\z/,
        ###
        /[-_ ]tx\z/,
        ###
      ],
      opacity: [
      ],
      normal: [
        /[-_ ]tn\z/,
        ###
        /[-_ ]n\z/,
        ###
      ],
      metal: [
      ],
      roughness: [
        /\Ar\z/,
        /[-_ ]r\z/,
        ###
      ],
      occlusion: [
        /\Aao\z/,
        /\Aao[-_ ]/,
        /[-_ ]ao[-_ ]/,
        /[-_]ao\z/,
      ],
      metal_roughness_occlusion: [
      ],
      roughness_metal_occlusion: [
      ],
      roughness_occlusion_metal: [
      ],
      displacement: [
      ],
      specular: [
      ],
      gloss: [
      ],
      preview: [
      ],
    },
  ]

  attr_reader :assets_root_dir,
    :build_root_dir,
    :target_size,
    :use_combine,
    :use_encode,
    :use_encode_ktx,
    :recursive,
    :force,
    :dry_run,
    :types

  def self.exit_on_failure?
    true
  end

  ############################################################
  #
  ############################################################
  desc "meta", "Generate texture meta info"
  method_option :src, default: 'resources/assets'
  method_option :assets_root_dir, default: 'resources/assets'
  method_option :target_size,
    type: :numeric,
    required: false,
    default: 2048
  method_option :ext,
    type: :array,
    default: nil
  method_option :recursive,
    type: :boolean,
    default: false
  method_option :force,
    type: :boolean,
    default: false
  method_option :dry_run,
    type: :boolean,
    default: true
  def meta
    src_dir = options[:src]
    extensions = options[:ext] || EXTENSIONS
    extensions = extensions.map(&:downcase)

    @assets_root_dir = options[:assets_root_dir]
    @target_size = options[:target_size]
    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]

    @assets_root_dir = clean_dir_path(@assets_root_dir)
    src_dir = clean_dir_path(src_dir)

    puts "SRC_DIR:     #{src_dir}"
    puts "EXT:         #{extensions}"
    puts "TARGET_SIZE: #{target_size}"
    puts "FORCE:       #{force}"
    puts "RECURSIVE:   #{recursive}"
    puts "DRY_RUN:     #{dry_run}"

    generate_metadata(
      src_dir:,
      extensions:)
  end

  ############################################################
  #
  ############################################################
  desc "build", "build KTX assets"
  method_option :src, default: 'resources/assets'
  method_option :assets_root_dir, default: 'resources/assets'
  method_option :build_root_dir, default: 'resources/build'
  method_option :target_size,
    type: :numeric,
    required: true
  method_option :ext,
    type: :array,
    default: nil
  method_option :all,
    type: :boolean,
    default: false
  method_option :combine,
    type: :boolean,
    default: false
  method_option :encode,
    type: :boolean,
    default: false
  method_option :ktx,
    type: :boolean,
    default: false
  method_option :recursive,
    type: :boolean,
    default: false
  method_option :force,
    type: :boolean,
    default: false
  method_option :dry_run,
    type: :boolean,
    default: true
  method_option :type,
    type: :array,
    default: ['all']
  def build
    src_dir = options[:src]
    extensions = options[:ext] || EXTENSIONS
    extensions = extensions.map(&:downcase)

    @assets_root_dir = options[:assets_root_dir]
    @build_root_dir = options[:build_root_dir]
    @target_size = options[:target_size]
    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]
    @types = options[:type].map(&:to_sym)
    @use_combine = options[:combine]
    @use_encode = options[:encode]
    @use_encode_ktx = options[:ktx]

    if options[:all]
      @use_combine = true
      @use_encode = true
      @use_encode_ktx = true
    end

    unless @use_combine || @use_encode || @use_encode_ktx
      puts "--combine or --encode or --ktx required"
      exit
    end

    @assets_root_dir = clean_dir_path(@assets_root_dir)
    @build_root_dir = clean_dir_path(@build_root_dir)
    src_dir = clean_dir_path(src_dir)

    puts "SRC_DIR:     #{src_dir}"
    puts "ASSETS_DIR:  #{assets_root_dir}"
    puts "BUILD_DIR:   #{build_root_dir}"
    puts "TYPE:        #{types}"
    puts "TARGET_SIZE: #{target_size}"
    puts "COMBINE:     #{use_combine}"
    puts "ENCODE:      #{use_encode}"
    puts "ENCODE_KTX:  #{use_encode_ktx}"
    puts "EXT:         #{extensions}"
    puts "FORCE:       #{force}"
    puts "RECURSIVE:   #{recursive}"
    puts "DRY_RUN:     #{dry_run}"

    if use_combine
      puts "[COMBINE]"
      build_dir(
        src_dir:,
        extensions:,
        combine_pass: true,
        encode_pass: false,
        encode_ktx_pass: false
      )
    end

    if use_encode
      puts "[ENCODE]"
      build_dir(
        src_dir:,
        extensions:,
        combine_pass: false,
        encode_pass: true,
        encode_ktx_pass: false)
    end

    if use_encode_ktx
      puts "[ENCODE_KTX]"
      build_dir(
        src_dir:,
        extensions:,
        combine_pass: false,
        encode_pass: false,
        encode_ktx_pass: true)
    end
  end

  private

  ############################################################
  #
  ############################################################
  def generate_metadata(
    src_dir:,
    extensions:)

    metadata = read_metadata(src_dir:)

    manual_textures = {}
    textures = []
    sub_dirs = []

    metadata[:textures]&.each do |info|
      next unless info[:manual]
      manual_textures[info[:name]] = info
    end

    # TODO KI ktx options
    #
    # --resize <width>x<height>
    #              Resize images to @e width X @e height. This should not be used
    #              with @b--mipmap as it would resize all the images to the same
    #              size. Resampler options can be set via --filter and --fscale.
    #
    # --input_swizzle <swizzle>
    #              Swizzle the input components according to swizzle which is an
    #              alhpanumeric sequence matching the regular expression
    #              ^[rgba01]{4}$.

    files = list_files(src_dir)
    files.sort_by(&:downcase).each do |f|
      name = File.basename(f)
      plain_name = File.basename(f, ".*")

      old_tex_info = manual_textures[name]

      src_path = "#{src_dir}/#{name}"
      if File.directory?(src_path)
        sub_dirs << name
        next
      end

      img = nil
      tex_info = nil

      file_ext = File.extname(f).downcase[1, 5]
      if extensions.include?(file_ext)
        img = Magick::Image.ping(src_path).first

        type = detect_type(name)

        case type
        when :color
          tex_info = {
            type: :color,
            action: :encode,
          }
        when :emission
          tex_info = {
            type: :emission,
            action: :encode,
          }
        when :normal
          tex_info = {
            type: :normal,
            action: :encode,
            mode: :normal,
            # NOTE KI only 3 channels in normal
            # => DROP alpha as redundant
            target_channel: RGB,
          }
        when :bump
          tex_info = {
            type: :bump,
            action: :encode,
            mode: :bump,
            # NOTE KI only 1 channel in bump
            # i.e. not same as normal
            target_channel: RED,
          }
        when :specular
          tex_info = {
            type: :specular,
            action: :skip,
          }
        when :opacity
          tex_info = {
            type: :opacity,
            action: :encode,
            mode: MODE_OPACITY,
            target_name: OPACITY_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :metal
          tex_info = {
            group: 'default',
            type: :metalness,
            action: :combine,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED,
            target_channel: GREEN,
          }
        when :roughness
          tex_info = {
            group: 'default',
            type: :roughness,
            action: :combine,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED,
            target_channel: BLUE,
          }
        when :occlusion
          tex_info = {
            group: 'default',
            type: :occlusion,
            action: :combine,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :metal_roughness
          tex_info = {
            group: 'default',
            type: :metal_roughness,
            action: :skip,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED_GREEN,
            target_channel: GREEN_BLUE
          }
        when :metal_roughness_occlusion
          tex_info = {
            group: 'default',
            type: :metal_roughness_occlusion,
            action: :skip,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED_GREEN_BLUE,
            target_channel: GREEN_BLUE_RED,
          }
        when :roughness_metal_occlusion
          tex_info = {
            group: 'default',
            type: :roughness_metal_occlusion,
            action: :skip,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED_GREEN_BLUE,
            target_channel: BLUE_GREEN_RED,
          }
        when :roughness_occlusion_metal
          tex_info = {
            group: 'default',
            type: :roughness_occlusion_metal,
            action: :skip,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED_GREEN_BLUE,
            target_channel: BLUE_RED_GREEN,
          }
        when :occlusion_roughness_metal
          tex_info = {
            group: 'default',
            type: :occlusion_roughness_metal,
            action: :skip,
            mode: MODE_MRAS,
            target_name: MRAS_MAP,
            source_channel: RED_GREEN_BLUE,
            target_channel: RED_BLUE_GREEN,
          }
        when :displacement
          tex_info = {
            group: 'default',
            type: :displacement,
            action: :combine,
            mode: MODE_DISPLACEMENT,
            target_name: DISPLACEMENT_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :cavity
          tex_info = {
            group: 'default',
            type: :cavity,
            action: :skip,
            mode: :cavity,
            target_name: CAVITY_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :gloss
          tex_info = {
            type: :gloss,
            action: :skip,
          }
        when :noise
          tex_info = {
            type: :noise,
            action: :copy,
          }
        when :preview
          tex_info = {
            type: :preview,
            action: :skip,
          }
        else
          tex_info = {
            type: :unknown,
            action: :copy,
          }
        end

        if tex_info
          if img.colorspace == Magick::GRAYColorspace
            channels = RED
          else
            channels = img.alpha? ? RGBA : RGB
          end

          base = {
            name:,
            target_name: plain_name,
            group: nil,
            type: :unknown,
            action: :skip,
            mode: 'copy',
            detected_channels: channels,
            source_channel: channels,
            target_channel: channels,
            source_depth: img.quantum_depth,
            target_depth: img.quantum_depth,
            source_size: "#{img.columns}x#{img.rows}",
            srgb: img.colorspace == Magick::SRGBColorspace,
            manual: false,
          }
          tex_info = base.merge(tex_info)
        end
      end

      if tex_info
        tex_info = tex_info.merge(old_tex_info) if old_tex_info
      else
        # NOTE KI keep old dead manual entries
        tex_info = old_tex_info
      end

      if tex_info
        if tex_info[:type] == :unknown
          puts "**WARN** unknown type: #{src_dir}  #{tex_info[:name]} **"
        end

        textures << tex_info
      end
    end

    need_process = !textures.empty?

    begin
      grouped_textures = group_by_prefix(textures)

      texture_to_groups = {}

      grouped_textures.each do |group, group_textures|
        group_textures.each do |tex_info|
          texture_to_groups[tex_info[:name]] ||= []
          texture_to_groups[tex_info[:name]] << group
        end
      end

      textures.each do |tex_info|
        groups = texture_to_groups[tex_info[:name]]
        next unless groups

        # NOTE KI ignore group with single entry if better ones exist
        # => at extreme there is no groups
        group_sizes = groups.map { |e| [e, grouped_textures[e].size] }.to_h
        if groups.size > 1
          groups.delete_if { |e| group_sizes[e] == 1 }
        end

        groups = groups.sort_by { |e| e.length }
        tex_info[:group] = groups.last unless groups.empty?
      end
    end

    textures.each do |tex_info|
      next unless tex_info[:group]
      next if tex_info[:manual]
      next if tex_info[:action] != :combine

      tex_info[:target_name] = "#{tex_info[:group]}_#{tex_info[:target_name]}"
    end

    if need_process
      metadata[:dir] = src_dir
      metadata[:textures] = textures.sort_by { |e| e[:name].downcase }

      write_metadata(src_dir:, data: metadata)
    end

    if recursive
      sub_dirs.each do |sub_dir|
        generate_metadata(
          src_dir: [src_dir, sub_dir].join('/'),
          extensions:)
      end
    end
  end

  def detect_type(f)
    basename = File.basename(f, ".*").downcase

    REG_EX.each_with_index do |level, idx|
      level.each do |type, expressions|
        return type if expressions.any? { |e| e.match?(basename) }
      end
    end

    :unknown
  end

  def group_by_prefix(textures)
    groups = {}

    textures.each do |tex_info|
      #next unless tex_info[:action] == :combine

      parts = tex_info[:name].split('_')
      parts.size.times do |idx|
        group = parts[0, idx + 1].join('_')
        next if group == tex_info[:name]

        type = detect_type(group)
        next if type != :unknown

        (groups[group] ||= []) << tex_info
      end
    end

    groups
  end

  ############################################################
  #
  ############################################################
  def build_dir(
    src_dir:,
    extensions:,
    combine_pass:,
    encode_pass:,
    encode_ktx_pass:)

    plain_dir = src_dir[assets_root_dir.length + 1, src_dir.length]
    dst_dir = "#{build_root_dir}/#{target_size}/#{plain_dir}"
    puts "DIR: #{src_dir} => #{dst_dir}"

    metadata = read_metadata(src_dir:)

    combine_textures = {}
    encode_textures = []

    metadata.textures&.each do |tex_info|
      name = tex_info.name.downcase
      next unless name

      action = tex_info.action.to_sym
      type = tex_info.type.to_sym

      unless extensions.any? { |ext| name.downcase.end_with?(ext) }
        puts "IGNORE: wrong ext - #{type} #{src_dir} #{tex_info.name}"
      end

      if action == :combine
        (combine_textures[tex_info.target_name] ||= []) << tex_info
        next
      end

      if action == :encode
        encode_textures << tex_info
        next
      end

      puts "IGNORE: #{type} #{src_dir} #{tex_info.name}"
    end

    if combine_pass
      combine_textures.each do |target_name, parts|
        target_mode = parts.first.mode.to_sym
        create_combound_texture(src_dir, dst_dir, target_name, target_mode, parts)
      end
    end

    if encode_pass
      encode_textures.each do |tex_info|
        encode_image(
          src_dir:,
          dst_dir:,
          tex_info:
        )
      end
    end

    if encode_ktx_pass
      encode_textures.each do |tex_info|
        src_path = "#{src_dir}/#{name}"

        encode_ktx_image(
          src_path,
          dst_dir:,
          type: tex_info.type,
          target_type: tex_info.target_type || RGB,
          srgb: tex_info.srgb || false,
          normal_mode: tex_info.mode-to_sym == :normal
        )
      end
    end

    if recursive
      sub_dirs = []

      files = list_files(src_dir)
      files.sort_by(&:downcase).each do |f|
        if File.directory?("#{src_dir}/#{f}")
          sub_dirs << f
        end
      end

      sub_dirs.each do |sub_dir|
        build_dir(
          src_dir: [src_dir, sub_dir].join('/'),
          extensions:,
          combine_pass:,
          encode_pass:,
          encode_ktx_pass:)
      end
    end
  end

  ############################################################
  # combine
  ############################################################
  def create_combound_texture(
    src_dir,
    dst_dir,
    target_name,
    target_mode,
    parts
  )
    group = parts.first.group

    case target_mode
    when MODE_MRAS
      create_mras_texture(
        src_dir,
        dst_dir,
        parts.first.group,
        target_name,
        parts
      )
    when MODE_DISPLACEMENT
      create_displacement_texture(
        src_dir,
        dst_dir,
        parts.first.group,
        target_name,
        parts
      )
    else
      raise "ERROR: unknown mode: #{{
        src_dir:,
        group:,
        target_name:,
        target_mode:,
        parts: parts.map(&:name),
      }}"
    end

    GC.start
  end

  ########################################
  # MRAS
  ########################################
  def create_mras_texture(
    src_dir,
    dst_dir,
    group,
    target_name,
    parts
  )
    if parts.size > 3
      raise "ERROR: too many parts: #{{
        src_dir:,
        group:,
        target_name:,
        parts: parts.map(&:name),
      }}"
    end

    channel_counts = {}
    parts.each do |tex_info|
      channel_counts[tex_info.target_channel] ||= 0
      channel_counts[tex_info.target_channel] += 1
    end

    if channel_counts.any? { |k, v| v > 1 }
      duplicates = channel_counts
        .select { |k, v| v > 1 }
        .map(&:first)
        .map do |e|
          parts.select { |p| p.target_channel == e }.map(&:name)
        end

      raise "ERROR: duplicate channel mapping: #{{
        src_dir:,
        group:,
        target_name:,
        parts: parts.map(&:name),
        duplicates:}}"
    end

    if parts.map(&:target_depth).uniq.size > 1
      raise "ERROR: target_depth mismatch: #{{
        src_dir:,
        group:,
        target_name:,
        parts: parts.map(&:name),
        depths: parts.map(&:target_depth).uniq.size}}"
    end

    dst_path = "#{dst_dir}/#{target_name}#{BUILD_SUFFIX}.png"

    sorted_parts = parts.sort_by { |e| e.name }
    target_depth = sorted_parts.first.target_depth

    source_paths = sorted_parts.map do |tex_info|
      "#{src_dir}/#{tex_info.name}"
    end

    salt = {
      version: COMBINE_VERSION,
      size: target_size,
      depth: target_depth,
      parts: sorted_parts.map do |tex_info|
        {
          name: tex_info.name,
          source_channel: tex_info.source_channel,
          target_channel: tex_info.target_channel,
        }
      end.sort_by { |e| e[:name] }
    }

    sha_digest = sha_changed?(dst_path, source_paths, salt)
    return unless sha_digest

    puts "MRAS: [#{group}] [size=#{target_size}] [depth=#{target_depth}] #{dst_path}"

    # channel: [ ambient-occlusion, metalness, roughness, specular ]
    # DEFAULTS = glm::vec3 mras{ 1.f, 0.f, 1.f, 0.f };
    target_channels = {
      Magick::RedChannel => nil,
      Magick::GreenChannel => nil,
      Magick::BlueChannel => nil,
      Magick::AlphaChannel => nil,
    }

    target_w = target_size
    target_h = target_size

    parts.each do |tex_info|
      src_channel = select_channel(tex_info.source_channel)
      dst_channel = select_channel(tex_info.target_channel)

      next unless src_channel && dst_channel

      src_path = "#{src_dir}/#{tex_info.name}"

      puts "LOAD: [#{group}] #{dst_channel} = #{src_channel} #{src_path}"

      # https://imagemagick.org/script/command-line-options.php#separate
      # NOTE KI *NOT* supporting non power-of-2 images
      # => should be resonable restriction
      channel_img = Magick::Image.read(src_path)
        .first

      channel_img = channel_img
        .separate(src_channel)[0]
        .set_channel_depth(Magick::AllChannels, target_depth)
      channel_img = scale_image(channel_img, target_size)

      target_w = channel_img.columns
      target_h = channel_img.rows

      target_channels[dst_channel] = {
        image: channel_img,
        channel: src_channel,
      }
    end

    img_list = Magick::ImageList.new
    alpha_img = nil

    black = black_image(target_w, target_h, target_depth)
    white = white_image(target_w, target_h, target_depth)

    target_placeholders = {
      Magick::RedChannel => white,
      Magick::GreenChannel => black,
      Magick::BlueChannel => white,
      Magick::AlphaChannel => black,
    }

    target_channels.each do |dst_channel, image_info|
      channel_img = target_placeholders[dst_channel]
      src_channel = nil

      if image_info
        channel_img = image_info[:image]
        src_channel = image_info[:channel]
      end

      puts "MAP:  [#{group}] #{dst_channel} = #{src_channel} #{channel_img.inspect}"

      if dst_channel == Magick::AlphaChannel
        alpha_img = channel_img
        next
      end

      img_list << channel_img
    end

    # NOTE KI workaround segmentation fault, which happens
    # if running without pause
    GC.start
    sleep 0.25
    #debugger

    # https://imagemagick.org/script/command-line-options.php#combine
    dst_img = img_list.combine(Magick::RGBColorspace)

    if alpha_img
      dst_img.alpha(Magick::SetAlphaChannel)
      dst_img
        .composite_channel!(
          alpha_img,
          0, 0,
          Magick::CopyAlphaCompositeOp,
          Magick::AlphaChannel)
    end

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      # https://unix.stackexchange.com/questions/689906/imagemagick-not-converting-grayscale-to-rgb
      file_format = "";

      if img_list.size == 3
        if (alpha_img)
          file_format = dst_img.quantum_depth == 16 ? "PNG64:" : "PNG32:"
        else
          file_format = dst_img.quantum_depth == 16 ? "PNG48:" : "PNG24:"
        end
      end

      puts "WRITE: [#{group}] #{file_format + dst_path}"
      dst_img.write(file_format + dst_path)

      write_digest(dst_path, sha_digest, source_paths, salt)

      puts "DONE: [#{group}] #{dst_path}"
    end
  end

  ########################################
  # DISPLACEMENT
  ########################################
  def create_displacement_texture(
    src_dir,
    dst_dir,
    group,
    target_name,
    parts
  )
    if parts.size > 1
      raise "ERROR: too many parts: #{{
        src_dir:,
        group:,
        target_name:,
        parts: parts.map(&:name),
      }}"
    end

    dst_path = "#{dst_dir}/#{target_name}#{BUILD_SUFFIX}.png"

    part = parts.first
    src_path = "#{src_dir}/#{part.name}"
    target_depth = part.target_depth

    salt = {
      version: COMBINE_VERSION,
      size: target_size,
      parts: [
        {
          name: part.name,
          source_channel: part.source_channel,
          target_channel: part.target_channel,
        }
      ]
    }

    sha_digest = sha_changed?(dst_path, [src_path], salt)
    return unless sha_digest

    puts "DISPLACEMENT: [#{group}] [size=#{target_size}] [depth=#{target_depth}] ]#{dst_path}"

    src_channel = select_channel(part.source_channel) || select_channel(RED)
    dst_channel = select_channel(part.target_channel) || select_channel(RED)

    # https://imagemagick.org/script/command-line-options.php#separate
    src_path = part.src_path(src_dir)
    src_img = Magick::Image.read(src_path)
      .first
      .separate(src_channel)[0]
      .set_channel_depth(Magick::AllChannels, target_depth)
    src_img = scale_image(src_img, target_size)

    puts "#{dst_channel} = #{src_img.inspect}"

    img_list = Magick::ImageList.new
    img_list << src_img

    #debugger

    # https://imagemagick.org/script/command-line-options.php#combine
    dst_img = img_list.combine(Magick::RGBColorspace)

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      puts "WRITE: #{dst_path}"
      dst_img.write(dst_path)

      write_digest(dst_path, sha_digest, [src_path], salt)

      puts "DONE:  #{dst_path}"
    end
  end

  ############################################################
  # encode
  ############################################################

  def encode_image(
    src_dir:,
    dst_dir:,
    tex_info:
  )
    group = tex_info.group
    target_depth = tex_info.target_depth
    target_name = tex_info.target_name

    src_path = tex_info.src_path(src_dir)
    dst_path = "#{dst_dir}/#{target_name}#{BUILD_SUFFIX}.png"

    salt = {
      version: COMBINE_VERSION,
      size: target_size,
      depth: target_depth,
      parts: [
        {
          name: tex_info.name,
          source_channel: tex_info.source_channel,
          target_channel: tex_info.target_channel,
        }
      ]
    }

    sha_digest = sha_changed?(dst_path, [src_path], salt)
    return unless sha_digest

    puts "ENCODE: [#{group}] [size=#{target_size}] [depth=#{target_depth}] [#{tex_info.target_channel}=#{tex_info.source_channel}] #{dst_path}"

    src_channels = select_channels(tex_info.source_channel)
    dst_channels = select_channels(tex_info.target_channel)

    target_channels = {
      Magick::RedChannel => nil,
      Magick::GreenChannel => nil,
      Magick::BlueChannel => nil,
      Magick::AlphaChannel => nil,
    }

    target_channels.keys.each do |ch|
      target_channels.delete(ch) unless dst_channels.include?(ch)
    end

    src_img = Magick::Image.read(src_path)
      .first
      .set_channel_depth(Magick::AllChannels, target_depth)

    src_img = src_img
      .set_channel_depth(Magick::AllChannels, target_depth)
    src_img = scale_image(src_img, target_size)

    target_w = src_img.columns
    target_h = src_img.rows

    black = black_image(target_w, target_h, target_depth)
    white = white_image(target_w, target_h, target_depth)

    target_placeholders = {
      Magick::RedChannel => black,
      Magick::GreenChannel => black,
      Magick::BlueChannel => black,
      Magick::AlphaChannel => white,
    }

    src_channels.each_with_index do |src_channel, idx|
      dst_channel = dst_channels[idx]

      img = src_img
        .separate(src_channel)[0]

      target_channels[dst_channel] = {
        image: img,
        channel: src_channel,
      }
    end

    img_list = Magick::ImageList.new
    alpha_img = nil

    target_channels.each do |dst_channel, image_info|
      channel_img = target_placeholders[dst_channel]
      src_channel = nil

      if image_info
        channel_img = image_info[:image]
        src_channel = image_info[:channel]
      end

      puts "MAP:  [#{group}] #{dst_channel} = #{src_channel} #{channel_img.inspect}"

      if dst_channel == Magick::AlphaChannel
        alpha_img = channel_img
        next
      end

      img_list << channel_img
    end

    # NOTE KI workaround segmentation fault, which happens
    # if running without pause
    GC.start
    sleep 0.25

    #debugger

    # https://imagemagick.org/script/command-line-options.php#combine
    # Magick::RGBColorspace
    dst_img = img_list.combine(src_img.colorspace)
    # dst_img.colorspace = src_img.colorspace

    # if target_channels.size > 1
    #   dst_img.image_type = Magick::TrueColorType
    # end

    if alpha_img
      # dst_img.image_type = Magick::TrueColorAlphaType
      dst_img.alpha(Magick::SetAlphaChannel)
      dst_img
        .composite_channel!(
          src_img,
          0, 0,
          Magick::CopyAlphaCompositeOp,
          Magick::AlphaChannel)
    end

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      # puts dst_img.image_type
      # puts dst_img.colorspace
      # puts dst_img.gray?

      # https://unix.stackexchange.com/questions/689906/imagemagick-not-converting-grayscale-to-rgb
      file_format = "";

      if img_list.size == 3
        if (alpha_img)
          file_format = dst_img.quantum_depth == 16 ? "PNG64:" : "PNG32:"
        else
          file_format = dst_img.quantum_depth == 16 ? "PNG48:" : "PNG24:"
        end
      end

      puts "WRITE: [#{group}] #{file_format + dst_path}"
      dst_img.write(file_format + dst_path)

      # dst_img.write(dst_path) do |info|
      #   p info.image_type
      #   #info.image_type = Magick::TrueColorType
      # end

      write_digest(dst_path, sha_digest, [src_path], salt)

      puts "DONE: [#{group}] #{dst_path}"
    end
  end

  ############################################################
  # KTX
  ############################################################
  def encode_ktx_image(
    src_path,
    dst_dir:,
    type:,
    target_type: RGB,
    srgb: true,
    normal_mode: false
  )
    return unless types.include?(type) || types.include?(:all)

    basename = File.basename(src_path, ".*")
    dst_path = "#{dst_dir}/#{basename}.ktx"
    dst_tmp_path = "#{dst_path}.tmp"

    salt = {
      version: KTX_VERSION,
      target_size:,
      parts: [
        name: File.basename(src_path)
      ]
    }

    sha_digest = sha_changed?(dst_path, [src_path], salt)
    return unless sha_digest

    puts "ENCODE[#{type.to_s.upcase}]: #{src_path}"

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      if File.exist?(dst_tmp_path)
        FileUtils.rm(dst_tmp_path)
      end
    end

    cmd = [
      "toktx.exe",
      "--verbose",
      "--genmipmap",
      #"--automipmap",
      "--encode",
      normal_mode ? "astc" : "astc",
      "--target_type",
      target_type,
      "--assign_oetf",
      srgb ? "srgb" : "linear",
      "--lower_left_maps_to_s0t0",
    ].compact

    if normal_mode
      cmd << "--normal_mode"
      "--uastc_rdo_l"
    end


    src_pathname = Pathname.new(src_path)
    dst_pathname = Pathname.new(dst_tmp_path)
    #binding.irb

    cmd << %Q["#{dst_pathname.cleanpath}"]
    cmd << %Q["#{src_pathname.cleanpath}"]

    puts "CMD: #{cmd.join(" ")}"

    unless dry_run
      puts "WRITE: #{dst_path}"

      %x{#{cmd.join(" ")}}

      if File.exist?(dst_tmp_path)
        FileUtils.cp(dst_tmp_path, dst_path)
        FileUtils.rm_f(dst_tmp_path)

        puts "DONE:  #{dst_path}"
      end

      unless File.exist?(dst_path)
        puts "FAIL:  #{src_path}"
        return
      end

      write_digest(dst_path, sha_digest, [src_path], salt)
    end
  end

  ############################################################
  # UTILS
  ############################################################

  ####################
  # metadata
  ####################
  def select_channel(ch)
    MAGICK_CHANNELS[ch&.upcase]
  end

  def select_channels(channels)
    channels.chars.map do |ch|
      MAGICK_CHANNELS[ch&.upcase]
    end
  end

  def black_image(target_w, target_h, target_depth)
    @black_iamge ||= {}
    @black_iamge[[target_w, target_h, target_depth]] ||=
      if true
        Magick::Image
          .new(target_w, target_h) { |opt|
            opt.background_color = 'black'
            opt.depth = target_depth
            opt.image_type = Magick::TrueColorAlphaType
            opt.colorspace = Magick::RGBColorspace
            opt.filename = "black"
          }
      else
        Magick::Image
          .read("#{assets_root_dir}/textures/placeholder/black.png")
          .first
          .separate(Magick::RedChannel)[0]
          .set_channel_depth(Magick::AllChannels, target_depth)
          .resize(target_w, target_h)
      end
  end

  def white_image(target_w, target_h, target_depth)
    @white_image ||= {}
    @white_image[[target_w, target_h, target_depth]] ||=
      if true
        Magick::Image
          .new(target_w, target_h) { |opt|
            opt.background_color = 'white'
            opt.depth = target_depth
            opt.image_type = Magick::TrueColorAlphaType
            opt.colorspace = Magick::RGBColorspace
            opt.filename = "white"
          }
      else
        Magick::Image
          .read("#{assets_root_dir}/textures/placeholder/white.png")
          .first
          .separate(Magick::RedChannel)[0]
          .set_channel_depth(Magick::AllChannels, target_depth)
          .resize(target_w, target_h)
      end
  end

  def scale_image(img, target_size)
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
  def resolve_size(img, target_size)
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

  def read_metadata(
    src_dir:)
    metadata_path = "#{src_dir}/_assets.meta"
    return Metadata.new_empty unless File.exist?(metadata_path)

    puts "READ: #{metadata_path}"

    data = JSON.parse(File.read(metadata_path), symbolize_names: true)

    data[:textures]&.each do |tex_info|
      tex_info.delete :target_type
      tex_info.delete :normal_map
    end

    Metadata.new(**data)
  end

  def write_metadata(
    src_dir:,
    data:)
    metadata_path = "#{src_dir}/_assets.meta"
    puts "WRITE: #{metadata_path}"

    if dry_run
      puts JSON.pretty_generate(data)
      return
    end

    File.write(metadata_path, JSON.pretty_generate(data) + "\n")
  end

  def list_files(src_dir)
    return [] unless File.directory?(src_dir)

    old_dir = Dir.pwd
    Dir.chdir(src_dir)
    files = Dir["*"]
    Dir.chdir(old_dir)

    files
  end

  def read_digest(dst_path)
    digest_path = "#{dst_path}.digest"
    return unless File.exist?(digest_path)
    data = YAML.load(File.read(digest_path), symbolize_names: true)
    data[:sha_digest]
  end

  def write_digest(
    dst_path,
    sha_digest,
    source_paths,
    salt
  )
    digest_path = "#{dst_path}.digest"
    data = {
      sha_digest:,
      salt:,
      sources: source_paths.sort.map do |src_path|
        img = Magick::Image.ping(src_path).first
        {
          path: src_path,
          size: "#{img.columns}x#{img.rows}",
          depth: img.quantum_depth,
        }
      end
    }
    File.write(digest_path, YAML.dump(data))
  end

  #
  # @return SHA if sha changed, nil otherwise
  #
  def sha_changed?(dst_path, source_paths, salt)
    old_digest = read_digest(dst_path)
    sha_digest = digest(source_paths, salt)

    if old_digest == sha_digest && File.exist?(dst_path) && !force
      puts "SKIP: SHA_NOT_ChANGED #{dst_path}"
      return
    end

    sha_digest
  end

  # https://stackoverflow.com/questions/64130698/sha256-value-for-large-binaries-when-using-ruby
  def digest(file_paths, salt)
    sha = Digest::SHA2.new

    sha << salt.to_yaml

    file_paths.sort.each do |file_path|
      File.open(file_path) do |f|
        while chunk = f.read(8192)
          sha << chunk
        end
      end
    end

    sha.hexdigest
  end

  def clean_dir_path(path)
    return path unless path.end_with?('/')
    path.gsub(/[\/]*\z/, "")
  end
end
Converter.start(ARGV)
