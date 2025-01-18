require 'open3'

require 'thor'
require 'fileutils'

class Converter < Thor
  def self.exit_on_failure?
    true
  end

  desc "run", "build KTX assets"
  method_option :src, default: 'resources/assets'
  method_option :dst, default: 'resources/build'
  def call
    src_dir = options[:src]
    dst_dir = options[:src]
    extensions = ["png", "jpg"]

    process_dir(src_dir:, dst_dir:, extensions:)
  end

  private

  def process_dir(
    src_dir:,
    dst_dir:,
    extensions:)

    puts "DIR: #{src_dir} => #{dst_dir}"

    files = Dir["#{src_dir}/*"]

    sub_dirs = []

    files.sort_by(&:downcase).each do |f|
      basename = File.basename(f, ".*")

      if File.directory?(f)
        sub_dirs << basename
      end

      next unless extensions.any? { |ext| f.end_with?(ext) }

      case basename.downcase
      when /_col_/, /_color_/, /_color\z/, /[-_]basecolor\z/
        encode_srgba(f, dst_dir:)
      when /_nrm_/, /_normalgl_/, /_normalgl\z/, /[-_]normal\z/, /_nrm\z/
        encode_normal(f, dst_dir:)
      when /_specular_/, /[-_]specular\z/
        encode_normal(f, dst_dir:)
      when /_opacity_/, /_opacity\z/
        encode_r(f, dst_dir:)
      when /_disp_/, /_metalness_/, /_metalness\z/, /-metallic\z/
        puts "CHANNEL_R: #{f}"
        #encode_r(f)
      when /_disp_/, /_displacement_/, /[-_]displacement\z/
        puts "CHANNEL_B: #{f}"
        #encode_r(f)
      when /_rgh_/, /_roughness_/, /[-_]roughness\z/
        puts "CHANNEL_G: #{f}"
        #encode_r(f)
      when /_ao_/, /_occlusion_/, /_ambientocclusion\z/, /[-_]ao\z/
        puts "CHANNEL_A: #{f}"
        #encode_r(f)
      else
        #puts "SKIP: #{f}"
        encode_srgba(f, dst_dir:)
      end
    end

    sub_dirs.each do |sub_dir|
      process_dir(
        src_dir: [src_dir, sub_dir].join('/'),
        dst_dir: [dst_dir, sub_dir].join('/'),
        extensions:)
    end
  end

  def encode_srgba(src, dst_dir:)
    encode(src, dst_dir:, mode: :srgba, target_type: "RGBA", srgb: true)
  end

  def encode_srgb(src, dst_dir:)
    encode(src, dst_dir:, mode: :srgb, target_type: "RGB", srgb: true)
  end

  def encode_rgb(src, dst_dir:)
    encode(src, dst_dir:, mode: :rgb, target_type: "RGB", srgb: false)
  end

  def encode_normal(src, dst_dir:)
    encode(src, dst_dir:, mode: :normal, target_type: "RGB", srgb: false, normal_mode: true)
  end

  def encode_r(src, dst_dir:)
    encode(src, dst_dir:, mode: :r, target_type: "R", srgb: false)
  end

  def encode(
    src,
    dst_dir:,
    mode:,
    target_type: "RGB",
    srgb: true,
    normal_mode: false
  )
    basename = File.basename(src, ".*")
    dst = "#{dst_dir}/#{basename}.ktx"
    dst_tmp = "#{dst}.tmp"

    if File.exist?(dst)
      src_file = File.new(src)
      dst_file = File.new(dst)
      if src_file.mtime <= dst_file.mtime
        puts "SKIP  [#{mode.to_s.upcase}]: not_changed #{src}"
        return
      end
    end

    puts "ENCODE[#{mode.to_s.upcase}]: #{src}"

    FileUtils.mkdir_p(dst_dir)

    if File.exist?(dst_tmp)
      FileUtils.rm(dst_tmp)
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
    puts system(*cmd)

    if File.exist?(dst_tmp)
      FileUtils.cp(dst_tmp, dst)
      FileUtils.rm_f(dst_tmp)
    end

    unless File.exist?(dst)
      puts "FAIL: #{src}"
      return
    end

    #puts "OK: #{src}"
    #puts system("ktxinfo.exe", dst)
  end
end
Converter.start(ARGV)
