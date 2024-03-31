require 'open3'

class Convert
  def run
    ["png", "jpg"].each do |ext|
      encode_ext(ext)
    end
  end

  def encode_ext(ext)
    files = Dir["*.#{ext}"]

    files.each do |f|
      basename = File.basename(f, ".*")

      case basename.downcase
      when /_col_/, /_color_/, /_color\z/, /[-_]basecolor\z/
        encode_srgb(f)
      when /_nrm_/, /_normalgl_/, /_normalgl\z/, /[-_]normal\z/
        encode_normal(f)
      when /_opacity_/, /_opacity\z/
        encode_r(f)
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
        puts "SKIP: #{f}"
      end
    end
  end

  def encode_srgb(src)
    encode(src, target_type: "RGB", srgb: true)
  end

  def encode_rgb(src)
    encode(src, target_type: "RGB", srgb: false)
  end

  def encode_normal(src)
    encode(src, target_type: "RGB", srgb: false, normal_mode: true)
  end

  def encode_r(src)
    encode(src, target_type: "R", srgb: false)
  end

  def encode(
    src,
    target_type: "RGB",
    srgb: true,
    normal_mode: false
  )
    puts "ENCODE: #{src}"

    basename = File.basename(src, ".*")
    dst = "#{basename}.ktx"

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
      false && normal_mode ? "--normal_mode" : nil,
    ].compact

    cmd << dst
    cmd << src

    puts "CMD: #{cmd.join(" ")}"
    puts system(*cmd)

    unless File.exist?(dst)
      puts "FAIL: #{src}"
      return
    end

    #puts "OK: #{src}"
    #puts system("ktxinfo.exe", dst)
  end
end
Convert.new.run
