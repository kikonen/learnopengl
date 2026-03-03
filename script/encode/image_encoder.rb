# frozen_string_literal: true

require_relative "util"
require_relative "file_encoder"
require_relative "texture_info"

module Encode
  ############################################################
  # image encode
  ############################################################
  class ImageEncoder < FileEncoder
    attr_reader :tex_info

    def initialize(
      src_dir:,
      dst_dir:,
      target_size:,
      force:,
      dry_run:,
      tex_info:
    )
      super(src_dir:, dst_dir:, target_size:, force:, dry_run:)

      @tex_info = tex_info
    end

    def encode(tid:)
      @tid = tid

      encode_image(
        src_dir:,
        dst_dir:,
        tex_info:)
    end

    private

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

      dst_digest = TextureDigest.new(
        dst_path,
        [src_path],
        meta: {
          target: File.basename(dst_path),
          type: tex_info[:type],
          target_channel: tex_info.target_channel,
          srgb: tex_info.srgb,
        },
        salt: {
          version: IMAGE_VERSION,
          size: target_size,
          type: tex_info[:type],
          depth: target_depth,
          parts: [
            {
              name: tex_info.name,
              source_channel: tex_info.source_channel,
              target_channel: tex_info.target_channel,
              srgb: tex_info.srgb,
            }
          ]
        },
        force:,
        tid:)

      unless dst_digest.changed?
        return dst_digest.update_if_needed
      end

      info "ENCODE: [#{group}] [size=#{target_size}] [depth=#{target_depth}] [#{tex_info.target_channel}=#{tex_info.source_channel}] #{dst_path}"

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
      src_img = Util.scale_image(src_img, target_size)

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

        info "MAP:  [#{group}] #{dst_channel} = #{src_channel} #{channel_img.inspect}"

        if dst_channel == Magick::AlphaChannel
          alpha_img = channel_img
          next
        end

        img_list << channel_img
      end

      # NOTE KI workaround segmentation fault, which happens
      # if running without pause
      #GC.start
      #sleep 0.2

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

        # info dst_img.image_type
        # info dst_img.colorspace
        # info dst_img.gray?

        # https://unix.stackexchange.com/questions/689906/imagemagick-not-converting-grayscale-to-rgb
        file_format = "";

        if img_list.size == 3
          if (alpha_img)
            file_format = dst_img.quantum_depth == 16 ? "PNG64:" : "PNG32:"
          else
            file_format = dst_img.quantum_depth == 16 ? "PNG48:" : "PNG24:"
          end
        end

        info "WRITE: [#{group}] #{file_format + dst_path}"
        dst_img.write(file_format + dst_path)

        # dst_img.write(dst_path) do |info|
        #   p info.image_type
        #   #info.image_type = Magick::TrueColorType
        # end

        dst_digest.write_digest

        info "DONE: [#{group}] #{dst_path}"
      end
    end
  end
end
