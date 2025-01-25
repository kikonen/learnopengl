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
#require 'vips'

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

class Converter < Thor
  EXTENSIONS = ["png", "jpg", "jpeg", "tga"].freeze

  MRAO_MAP = 'mrao_build'
  DISPLACEMENT_MAP = 'displacement_build'

  RED = 'R'
  GREEN = 'G'
  BLUE = 'B'
  ALPHA = 'A'
  RGB = 'RGB'
  RGBA = 'RGBA'

  MAGICK_CHANNELS = {
    RED => Magick::RedChannel,
    GREEN => Magick::GreenChannel,
    BLUE => Magick::BlueChannel,
    ALPHA => Magick::AlphaChannel,
  }.freeze

  COMBINE_VERSION = 1
  KTX_VERSION = 1

  REG_EX = [
    # Level 1: most accurate
    {
      color: [
        /_color_/,
        /_color\z/,
        /[-_]basecolor\z/,
        /diffuse/,
        /albedo/,
      ],
      opacity: [
        /_opacity_/,
        /_opacity\z/,
      ],
      normal: [
        /_normalgl_/,
        /_normalgl\z/,
        /[-_]normal\z/,
        /\Anormal\z/,
        /_bump\z/,
      ],
      metal: [
        /_metalness_/,
        /_metalness\z/,
        /-metallic\z/,
      ],
      roughness: [
        /_roughness_/,
        /[-_]roughness\z/,
        /\Aroughness\z/,
      ],
      occlusion: [
        /_occlusion_/,
        /_ambientocclusion\z/,
      ],
      displacement: [
        /_displacement_/,
        /[-_]displacement\z/,
      ],
      specular: [
        /_specular_/,
        /[-_]specular\z/,
        /\Aspecular\z/,
      ],
      gloss: [
        /gloss/,
      ],
      preview: [
        /preview/,
      ],
    },
    # Level 2; less accurate
    {
      color: [
        /_col_/,
      ],
      opacity: [
      ],
      normal: [
        /_nrm_/,
        /_nrm\z/,
      ],
      metal: [
      ],
      roughness: [
        /_rgh_/,
        /_rough_/,
      ],
      occlusion: [
        /_ao_/,
        /[-_]ao\z/,
        /\Aao\z/,
      ],
      displacement: [
        /_disp_/,
        /_disp\z/,
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
    :combine,
    :encode_ktx,
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

    @target_size = options[:target_size]
    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]

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
    @combine = options[:combine]
    @encode_ktx = options[:ktx]

    if options[:all]
      @combine = true
      @encode_ktx = true
    end

    unless @combine || @encode_ktx
      puts "--combine or --ktx required"
      exit
    end

    puts "SRC_DIR:     #{src_dir}"
    puts "ASSETS_DIR:  #{assets_root_dir}"
    puts "BUILD_DIR:   #{build_root_dir}"
    puts "TYPE:        #{types}"
    puts "TARGET_SIZE: #{target_size}"
    puts "COMBINE:     #{combine}"
    puts "ENCODE_KTX:  #{encode_ktx}"
    puts "EXT:         #{extensions}"
    puts "FORCE:       #{force}"
    puts "RECURSIVE:   #{recursive}"
    puts "DRY_RUN:     #{dry_run}"

    build_dir(
      src_dir:,
      extensions:)
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
      basename = File.basename(f, ".*")

      old_info = manual_textures[name]

      src_path = "#{src_dir}/#{name}"
      if File.directory?(src_path)
        sub_dirs << name
        next
      end

      img = nil
      info = nil

      file_ext = File.extname(f).downcase[1, 5]
      if extensions.include?(file_ext)
        img = Magick::Image.ping(src_path).first
        #debugger

        type = detect_type(basename)

        case type
        when :preview
          info = {
            type: :preview,
            action: :skip,
          }
        when :color
          info = {
            type: :color,
            action: :encode,
          }
        when :normal
          info = {
            type: :normal,
            action: :encode,
            mode: :normal,
            # NOTE KI only 3 channels in normal
            target_channel: RGB,
          }
        when :specular
          info = {
            type: :specular,
            action: :skip,
          }
        when :opacity
          info = {
            type: :opacity,
            action: :copy,
            source_channel: RED,
            target_channel: RED,
          }
        when :metal
          info = {
            type: :metalness,
            action: :combine,
            mode: :mrao,
            target_name: MRAO_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :roughness
          info = {
            type: :roughness,
            action: :combine,
            mode: :mrao,
            target_name: MRAO_MAP,
            source_channel: RED,
            target_channel: GREEN,
          }
        when :occlusion
          info = {
            type: :occlusion,
            action: :combine,
            mode: :mrao,
            target_name: MRAO_MAP,
            source_channel: RED,
            target_channel: BLUE,
          }
        when :displacement
          info = {
            type: :displacement,
            action: :combine,
            mode: :displacement,
            target_name: DISPLACEMENT_MAP,
            source_channel: RED,
            target_channel: RED,
          }
        when :gloss
          info = {
            type: :gloss,
            action: :skip,
          }
        else
          info = {
            type: :unknown,
            action: :encode,
          }
        end

        if info
          if img.colorspace == Magick::GRAYColorspace
            channels = RED
          else
            channels = img.alpha? ? RGBA : RGB
          end

          base = {
            name:,
            target_name: name,
            type: :unknown,
            action: :skip,
            mode: 'copy',
            detected_channels: channels,
            source_channel: channels,
            target_channel: channels,
            source_depth: img.quantum_depth,
            target_depth: img.quantum_depth,
            srgb: img.colorspace == Magick::SRGBColorspace,
            manual: false,
          }
          info = base.merge(info)
        end
      end

      if info
        info = info.merge(old_info) if old_info
      else
        # NOTE KI keep old dead manual entries
        info = old_info
      end

      textures << info if info
    end

    need_process = textures.any? do |info|
      [:combine, :encode].include?(info[:action])
    end

    if need_process
      metadata[:dir] = src_dir
      metadata[:textures] = textures.sort_by { |e| e[:name] }
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

  def detect_type(filename)
    filename = filename.downcase

    REG_EX.each_with_index do |level, idx|
      level.each do |type, expressions|
        return type if expressions.any? { |e| e.match?(filename) }
      end
    end

    :unknown
  end

  ############################################################
  #
  ############################################################
  def build_dir(
    src_dir:,
    extensions:)

    plain_dir = src_dir[assets_root_dir.length + 1, src_dir.length]
    dst_dir = "#{build_root_dir}/#{target_size}/#{plain_dir}"
    puts "DIR: #{src_dir} => #{dst_dir}"

    metadata = read_metadata(src_dir:)

    combine_textures = {}

    metadata[:textures]&.each do |info|
      name = info[:name].downcase
      next unless name
      next unless extensions.any? { |ext| name.downcase.end_with?(ext) }

      src_path = "#{src_dir}/#{name}"

      action = info[:action]&.to_sym

      if action == :combine
        (combine_textures[info[:target_name]] ||= []) << info
        next
      end

      next unless action == :encode

      if encode_ktx
        encode(
          src_path,
          dst_dir:,
          type: info[:type],
          target_type: info[:target_type] || RGB,
          srgb: info[:srgb] || false,
          normal_mode: info[:mode]-to_sym == :normal
        )
      end
    end

    if combine
      combine_textures.each do |target_name, parts|
        target_mode = parts.first[:mode].to_sym
        create_combound_texture(src_dir, dst_dir, target_name, target_mode, parts)
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
          extensions:)
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
    case target_mode
    when :mrao
      create_mrao_texture(
        src_dir,
        dst_dir,
        target_name,
        parts
      )
    when :displacement
      create_displacement_texture(
        src_dir,
        dst_dir,
        target_name,
        parts
      )
    else
      raise "ERROR: unknown mode: #{{src_dir:, target_name:, target_mode:, parts:}}"
    end
  end

  ########################################
  # MRAO
  ########################################
  def create_mrao_texture(
    src_dir,
    dst_dir,
    target_name,
    parts
  )
    if parts.size > 3
      raise "ERROR: too many parts: #{{src_dir:, target_name:, parts:}}"
    end

    black = black_image
    white = white_image

    dst_path = "#{dst_dir}/#{target_name}.png"

    sorted_parts = parts.sort_by { |e| e[:name] }

    source_paths = sorted_parts.map do |info|
      "#{src_dir}/#{info[:name]}"
    end

    salt = {
      version: COMBINE_VERSION,
      size: target_size,
      parts: sorted_parts.map do |info|
        {
          name: info[:name],
          source_channel: info[:source_channel],
          target_channel: info[:target_channel],
        }
      end.sort_by { |e| e[:name] }
    }

    sha_digest = sha_changed?(dst_path, source_paths, salt)
    return unless sha_digest

    puts "MRAO: #{dst_path}"

    # channel: [ metalness, roughness, ambient-occlusion ]
    # DEFAULTS = glm::vec3 mrao{ 0.f, 1.f, 1.f };
    target_channels = {
      Magick::RedChannel => nil,
      Magick::GreenChannel => nil,
      Magick::BlueChannel => nil,
    }

    target_placeholders = {
      Magick::RedChannel => black,
      Magick::GreenChannel => white,
      Magick::BlueChannel => white,
    }

    #debugger

    parts.each do |info|
      src_channel = select_channel(info[:source_channel])
      dst_channel = select_channel(info[:target_channel])

      next unless src_channel && dst_channel

      # https://imagemagick.org/script/command-line-options.php#separate
      src_path = "#{src_dir}/#{info[:name]}"
      src_img = Magick::Image.read(src_path)
        .first
        .separate(src_channel)[0]
        .set_channel_depth(Magick::AllChannels, 8)
        .resize(target_size, target_size)

      target_channels[dst_channel] = {
        image: src_img,
        channel: src_channel,
      }
    end

    img_list = Magick::ImageList.new

    #debugger

    target_channels.each do |dst_channel, info|
      src_img = target_placeholders[dst_channel]

      if info
        src_img = info[:image]
      end

      puts "#{dst_channel} = #{src_img.inspect}"

      img_list << src_img
    end

    #debugger

    # https://imagemagick.org/script/command-line-options.php#combine
    dst_img = img_list.combine(Magick::RGBColorspace)

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      puts "WRITE: #{dst_path}"
      dst_img.write(dst_path)

      write_digest(dst_path, sha_digest, source_paths, salt)

      puts "DONE:  #{dst_path}"
    end
  end

  ########################################
  # DISPLACEMENT
  ########################################
  def create_displacement_texture(
    src_dir,
    dst_dir,
    target_name,
    parts
  )
    if parts.size > 1
      raise "ERROR: too many parts: #{{src_dir:, target_name:, parts:}}"
    end

    dst_path = "#{dst_dir}/#{target_name}.png"

    part = parts.first
    src_path = "#{src_dir}/#{part[:name]}"

    salt = {
      version: COMBINE_VERSION,
      size: target_size,
      parts: [
        {
          name: part[:name],
          source_channel: part[:source_channel],
          target_channel: part[:target_channel],
        }
      ]
    }

    sha_digest = sha_changed?(dst_path, [src_path], salt)
    return unless sha_digest

    puts "DISPLACEMENT: #{dst_path}"

    #debugger

    src_channel = select_channel(part[:source_channel]) || select_channel(RED)
    dst_channel = select_channel(part[:target_channel]) || select_channel(RED)

    # https://imagemagick.org/script/command-line-options.php#separate
    src_path = "#{src_dir}/#{part[:name]}"
    src_img = Magick::Image.read(src_path)
      .first
      .separate(src_channel)[0]
      .set_channel_depth(Magick::AllChannels, 8)
      .resize(target_size, target_size)

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

  def select_channel(ch)
    MAGICK_CHANNELS[ch&.upcase]
  end

  def black_image
    @black_iamge ||=
      if true
        w = target_size
        Magick::Image
          .new(w, w) { |opt|
            opt.background_color = 'black'
            opt.depth = 8
            opt.image_type = Magick::TrueColorAlphaType
            opt.colorspace = Magick::RGBColorspace
            opt.filename = "black"
          }
      else
        Magick::Image
          .read("#{assets_root_dir}/textures/placeholder/black.png")
          .first
          .separate(Magick::RedChannel)[0]
          .set_channel_depth(Magick::AllChannels, 8)
          .resize(target_size, target_size)
      end
  end

  def white_image
    @white_image ||=
      if true
        w = target_size
        Magick::Image
          .new(w, w) { |opt|
            opt.background_color = 'white'
            opt.depth = 8
            opt.image_type = Magick::TrueColorAlphaType
            opt.colorspace = Magick::RGBColorspace
            opt.filename = "white"
          }
      else
        Magick::Image
          .read("#{assets_root_dir}/textures/placeholder/white.png")
          .first
          .separate(Magick::RedChannel)[0]
          .set_channel_depth(Magick::AllChannels, 8)
          .resize(target_size, target_size)
      end
  end

  ############################################################
  # KTX
  ############################################################
  def encode(
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
  def read_metadata(
    src_dir:)
    metadata_path = "#{src_dir}/_assets.meta"
    return {} unless File.exist?(metadata_path)

    puts "READ: #{metadata_path}"
    JSON.parse(File.read(metadata_path), symbolize_names: true)
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

  def write_digest(dst_path, sha_digest, source_paths, salt)
    digest_path = "#{dst_path}.digest"
    data = {
      sha_digest:,
      salt:,
      sources: source_paths.sort
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
        while chunk = f.read(256)
          sha << chunk
        end
      end
    end

    sha.hexdigest
  end
end
Converter.start(ARGV)
