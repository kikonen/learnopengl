require 'open3'

require 'thor'
require 'fileutils'
require 'json'

class Converter < Thor
  attr_reader :assets_dir,
    :build_dir,
    :recursive,
    :force,
    :dry_run,
    :types

  def self.exit_on_failure?
    true
  end

  desc "meta", "Generate texture meta info"
  method_option :src, default: 'resources/assets'
  method_option :assets_dir, default: 'resources/assets'
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
    extensions = ["png", "jpg", "jpeg", "tga"]

    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]

    puts "SRC_DIR: #{src_dir}"
    puts "EXT:     #{extensions}"
    puts "FORCE:   #{force}"
    puts "DRY_RUN: #{dry_run}"

    generate_metadata(
      src_dir:,
      extensions:)
  end

  desc "ktx", "build KTX assets"
  method_option :src, default: 'resources/assets'
  method_option :assets_dir, default: 'resources/assets'
  method_option :build_dir, default: 'resources/build'
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
    extensions = ["png", "jpg"]

    @assets_dir = options[:assets_dir]
    @build_dir = options[:build_dir]
    @recursive = options[:recursive]
    @force = options[:force]
    @dry_run = options[:dry_run]
    @types = options[:type].map(&:to_sym)

    puts "SRC_DIR:   #{src_dir}"
    puts "BUILD_DIR: #{build_dir}"
    puts "TYPE:      #{types}"
    puts "EXT:       #{extensions}"
    puts "FORCE:     #{force}"
    puts "DRY_RUN:   #{dry_run}"

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
          target_type: 'RGB',
          srgb: true
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
      when /_disp_/, /_metalness_/, /_metalness\z/, /-metallic\z/
        info = {
          type: :metalness,
          action: :combine,
          target_type: "R",
          srgb: false,
        }
      when /_disp_/, /_displacement_/, /[-_]displacement\z/, /_disp\z/
        info = {
          type: :displacement,
          action: :combine,
          target_type: "B",
          srgb: false,
        }
      when /_rgh_/, /_roughness_/, /[-_]roughness\z/, /\Aroughness\z/
        info = {
          type: :roughness,
          action: :combine,
          target_type: "G",
          srgb: false,
        }
      when /_ao_/, /_occlusion_/, /_ambientocclusion\z/, /[-_]ao\z/, /\Aao\z/
        info = {
          type: :occlusion,
          action: :combine,
          target_type: "A",
          srgb: false,
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

    plain_dir = src_dir[assets_dir.length + 1, src_dir.length]
    dst_dir = "#{build_dir}/#{plain_dir}"
    puts "DIR: #{src_dir} => #{dst_dir}"

    metadata = read_metadata(src_dir:)

    metadata[:textures]&.each do |info|
      name = info[:name].downcase
      next unless name
      next unless extensions.any? { |ext| name.downcase.end_with?(ext) }

      filepath = "#{src_dir}/#{name}"

      action = info[:action]&.to_sym
      next unless action == :encode

      encode(
        filepath,
        dst_dir:,
        type: info[:type],
        target_type: info[:target_type] || 'RGB',
        srgb: info[:srgb] || false,
        normal_mode: info[:normal_mode] || false
      )
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
