# frozen_string_literal: true

require 'open3'

require 'thor'
require 'fileutils'
require 'json'
require 'rmagick'

class Converter < Thor
  EXTENSIONS = ["png", "jpg", "jpeg", "tga"]

  attr_reader :assets_root_dir,
    :build_root_dir,
    :target_size,
    :recursive,
    :force,
    :dry_run,
    :types

  def self.exit_on_failure?
    true
  end

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

    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]

    puts "SRC_DIR:  #{src_dir}"
    puts "EXT:      #{extensions}"
    puts "TARGET_SIZE: #{target_size}"
    puts "FORCE:    #{force}"
    puts "DRY_RUN:  #{dry_run}"

    generate_metadata(
      src_dir:,
      extensions:)
  end

  desc "ktx", "build KTX assets"
  method_option :src, default: 'resources/assets'
  method_option :assets_root_dir, default: 'resources/assets'
  method_option :build_root_dir, default: 'resources/build'
  method_option :target_size,
    type: :numeric,
    required: true
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
  method_option :type,
    type: :array,
    default: ['all']
  def ktx
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

    puts "SRC_DIR:    #{src_dir}"
    puts "ASSETS_DIR: #{assets_root_dir}"
    puts "BUILD_DIR:  #{build_root_dir}"
    puts "TYPE:       #{types}"
    puts "EXT:        #{extensions}"
    puts "FORCE:      #{force}"
    puts "DRY_RUN:    #{dry_run}"

    build_ktx(
      src_dir:,
      extensions:)
  end

  private

  ####################
  #
  ####################
  def generate_metadata(
    src_dir:,
    extensions:)

    metadata = read_metadata(src_dir:)
    textures = []

    files = Dir["#{src_dir}/*"]

    sub_dirs = []

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

    files.sort_by(&:downcase).each do |f|
      name = File.basename(f)
      basename = File.basename(f, ".*")

      if File.directory?(f)
        sub_dirs << name
      end

      next unless extensions.any? { |ext| f.end_with?(ext) }

      info = nil

      case basename.downcase
      when /preview/
        info = {
          type: :preview,
          action: :skip,
        }
      when /_col_/, /_color_/, /_color\z/, /[-_]basecolor\z/, /diffuse/, /albedo/
        info = {
          type: :color,
          action: :encode,
          target_type: "RGBA",
          srgb: true
        }
      when /_nrm_/, /_normalgl_/, /_normalgl\z/, /[-_]normal\z/, /_nrm\z/, /\Anormal\z/, /_bump\z/
        info = {
          type: :normal,
          action: :encode,
          target_type: "RGB",
          srgb: false,
          normal_map: true
        }
      when /_specular_/, /[-_]specular\z/, /\Aspecular\z/
        info = {
          type: :specular,
          action: :skip,
          target_type: "RGB",
          srgb: true,
        }
      when /_opacity_/, /_opacity\z/
        info = {
          type: :opacity,
          action: :encode,
          target_type: "R",
          srgb: false,
        }
      when /_metalness_/, /_metalness\z/, /-metallic\z/
        info = {
          type: :metalness,
          action: :combine,
          target_name: 'mrdao',
          source_channel: 'R',
          target_channel: "R",
        }
      when /_disp_/, /_displacement_/, /[-_]displacement\z/, /_disp\z/
        info = {
          type: :displacement,
          action: :combine,
          target_name: 'mrdao',
          source_channel: 'R',
          target_channel: "B",
        }
      when /_rgh_/, /_roughness_/, /[-_]roughness\z/, /\Aroughness\z/
        info = {
          type: :roughness,
          action: :combine,
          target_name: 'mrdao',
          source_channel: 'R',
          target_channel: "G",
        }
      when /_ao_/, /_occlusion_/, /_ambientocclusion\z/, /[-_]ao\z/, /\Aao\z/
        info = {
          type: :occlusion,
          action: :combine,
          target_name: 'mrdao',
          source_channel: 'R',
          target_channel: "A",
        }
      when /gloss/
        info = {
          type: :gloss,
          action: :skip,
          target_type: 'R',
          srgb: false,
        }
      else
        info = {
          type: :unknown,
          action: :encode,
          target_type: 'RGBA',
          srgb: true,
        }
      end

      if info
        info = { name: }.merge!(info)
        textures << info if info
      end
    end

    metadata[:dir] = src_dir
    metadata[:textures] = textures
    write_metadata(src_dir:, data: metadata)

    if recursive
      sub_dirs.each do |sub_dir|
        generate_metadata(
          src_dir: [src_dir, sub_dir].join('/'),
          extensions:)
      end
    end
  end

  ####################
  #
  ####################
  def build_ktx(
    src_dir:,
    extensions:)

    plain_dir = src_dir[assets_root_dir.length + 1, src_dir.length]
    dst_dir = "#{build_root_dir}/#{plain_dir}"
    puts "DIR: #{src_dir} => #{dst_dir}"

    metadata = read_metadata(src_dir:)

    combine_textures = {}

    metadata[:textures]&.each do |info|
      name = info[:name].downcase
      next unless name
      next unless extensions.any? { |ext| name.downcase.end_with?(ext) }

      filepath = "#{src_dir}/#{name}"

      action = info[:action]&.to_sym

      if action == :combine
        (combine_textures[info[:target_name]] ||= []) << info
        next
      end

      next unless action == :encode

      process_image(filepath)

      encode(
        filepath,
        dst_dir:,
        type: info[:type],
        target_type: info[:target_type] || 'RGB',
        srgb: info[:srgb] || false,
        normal_mode: info[:normal_mode] || false
      )
    end

    combine_textures.each do |target_name, parts|
      create_combound_texture(src_dir, dst_dir, target_name, parts)
    end

    if recursive
      sub_dirs = []

      files = Dir["#{src_dir}/*"]
      files.sort_by(&:downcase).each do |f|
        if File.directory?(f)
          sub_dirs << name
        end
      end

      sub_dirs.each do |sub_dir|
        build_ktx(
          src_dir: [src_dir, sub_dir].join('/'),
          extensions:)
      end
    end
  end

  ####################
  #
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

  def create_combound_texture(
    src_dir,
    dst_dir,
    target_name,
    parts
  )
    dst_filepath = "#{dst_dir}/#{target_name}_#{target_size}.png"

    channels = {}

    parts.each do |info|
      filepath = "#{src_dir}/#{info[:name]}"
      img = Magick::Image.read(filepath).first

      src_channel = select_channel(info[:source_channel])
      dst_channel = select_channel(info[:target_channel])

      next unless src_channel && dst_channel

      channels[dst_channel] = img.channel(src_channel).resize(target_size, target_size)
    end

    black = Magick::Image
      .new(target_size, target_size) { |options| options.background_color = 'black' }

    white = Magick::Image
      .new(target_size, target_size) { |options| options.background_color = 'white' }

    img_list = Magick::ImageList.new

    img_list << (channels[Magick::RedChannel]   || black)
    img_list << (channels[Magick::GreenChannel] || black)
    img_list << (channels[Magick::BlueChannel]  || black)
    img_list << (channels[Magick::AlphaChannel] || white)

    img = img_list.combine
    img.format = 'PNG'

    #binding.irb

    img.write(dst_filepath)
  end

  CHANNELS = {
    'R' => Magick::RedChannel,
    'G' => Magick::GreenChannel,
    'B' => Magick::BlueChannel,
    'A' => Magick::AlphaChannel,
  }

  def select_channel(ch)
    CHANNELS[ch&.upcase]
  end

  def process_image(
    src
  )
    # img = Magick::Image.read(src).first
    # binding.irb
  end

  def encode(
    src,
    dst_dir:,
    type:,
    target_type: "RGB",
    srgb: true,
    normal_mode: false
  )
    return unless types.include?(type) || types.include?(:all)

    basename = File.basename(src, ".*")
    dst = "#{dst_dir}/#{basename}.ktx"
    dst_tmp = "#{dst}.tmp"

    if File.exist?(dst) && !force
      src_file = File.new(src)
      dst_file = File.new(dst)
      if src_file.mtime <= dst_file.mtime
        puts "SKIP  [#{type.to_s.upcase}]: not_changed #{src}"
        return
      end
    end

    puts "ENCODE[#{type.to_s.upcase}]: #{src}"

    unless dry_run
      FileUtils.mkdir_p(dst_dir)

      if File.exist?(dst_tmp)
        FileUtils.rm(dst_tmp)
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
      normal_mode ? "--normal_mode" : nil,
    ].compact

    cmd << dst_tmp
    cmd << src

    puts "CMD: #{cmd.join(" ")}"

    unless dry_run
      puts system(*cmd)

      if File.exist?(dst_tmp)
        FileUtils.cp(dst_tmp, dst)
        FileUtils.rm_f(dst_tmp)
      end
      unless File.exist?(dst)
        puts "FAIL: #{src}"
        return
      end
    end

    #puts "OK: #{src}"
    #puts system("ktxinfo.exe", dst)
  end
end
Converter.start(ARGV)
