# frozen_string_literal: true

require_relative "util"
require_relative "file_encoder"

module Encode
  ############################################################
  # combine
  ############################################################
  class CombineEncoder < FileEncoder
    attr_reader :target_name,
      :target_mode,
      :parts

    def initialize(
      src_dir:,
      dst_dir:,
      target_name:,
      target_mode:,
      parts:,
      target_size:,
      target_depth:,
      force:,
      dry_run:
    )
      super(src_dir:, dst_dir:, target_size:, target_depth:, force:, dry_run:)

      @target_name = target_name
      @target_mode = target_mode

      @parts = parts
    end

    def encode(tid:)
      @tid = tid

      create_combound_texture(
        src_dir,
        dst_dir,
        target_name,
        target_mode,
        parts
      )
    end

    private

    def create_combound_texture(
      src_dir,
      dst_dir,
      target_name,
      target_mode,
      parts
    )
      group = parts.first.group

      case target_mode
      when MODE_DIFFUSE
        create_diffuse_texture(
          src_dir,
          dst_dir,
          parts.first.group,
          target_name,
          parts
        )
      when MODE_MRAS
        create_mras_texture(
          src_dir,
          dst_dir,
          parts.first.group,
          target_name,
          parts
        )
      when MODE_HEIGHT
        create_height_texture(
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
    # DIFFUSE
    ########################################
    def create_diffuse_texture(
      src_dir,
      dst_dir,
      group,
      target_name,
      parts
    )
      # NOTE KI diffuse can have only diffuse and opacity parts
      # => if opacity exists, it WILL override pre-existing alpha
      #    in diffuse alpha channel
      if parts.size > 2
        raise "ERROR: too many parts: #{{
          src_dir:,
          group:,
          target_name:,
          parts: parts.map(&:name),
        }}"
      end

      dst_path = "#{dst_dir}/#{target_name}#{BUILD_SUFFIX}.png"

      # NOTE KI sort opacity last so it WILL override alpha
      sorted_parts = parts.sort_by { |e| [e.type == :opacity ? 1 : 0, e.name] }

      source_paths = sorted_parts.map do |tex_info|
        "#{src_dir}/#{tex_info.name}"
      end

      dst_digest = TextureDigest.new(
        dst_path,
        source_paths,
        meta: {
          target: File.basename(dst_path),
          type: :diffuse,
          target_channel: RGBA,
          srgb: true,
        },
        salt: {
          version: DIFFUSE_VERSION,
          size: target_size,
          type: :diffuse,
          depth: target_depth,
          parts: sorted_parts.map do |tex_info|
            {
              name: tex_info.name,
              source_channel: tex_info.source_channel,
              target_channel: tex_info.target_channel,
              source_depth: tex_info.source_depth,
              srgb: tex_info.srgb,
            }
          end.sort_by { |e| e[:name] }
        },
        force:,
        tid:)

      unless dst_digest.changed?
        return dst_digest.update_if_needed
      end

      info "DIFF: [#{group}] [size=#{target_size}] [depth=#{target_depth}] #{dst_path}"

      target_channels = {
        Magick::RedChannel => nil,
        Magick::GreenChannel => nil,
        Magick::BlueChannel => nil,
        Magick::AlphaChannel => nil,
      }

      target_w = target_size
      target_h = target_size

      parts.each do |tex_info|
        # NOTE KI use detected channels
        src_channel_ids = tex_info.source_channel.chars
        dst_channel_ids = tex_info.target_channel.chars

        src_channel_ids.zip(dst_channel_ids).each do |src_channel_id, dst_channel_id|
          src_channel = select_channel(src_channel_id)
          dst_channel = select_channel(dst_channel_id)

          next unless src_channel && dst_channel

          src_path = tex_info.src_path(src_dir)

          info "LOAD: [#{group}] #{dst_channel} = #{src_channel} #{src_path}"

          # https://imagemagick.org/script/command-line-options.php#separate
          # NOTE KI *NOT* supporting non power-of-2 images
          # => should be resonable restriction
          channel_img = Magick::Image.read(src_path)
            .first
            .separate(src_channel)
            .first

          # NOTE KI enforce RGB space (not grayscale)
          channel_img.colorspace = Magick::SRGBColorspace

          channel_img = Util.scale_diffuse_image(channel_img, target_size)
          channel_img = channel_img.set_channel_depth(Magick::AllChannels, target_depth)

          target_w = channel_img.columns
          target_h = channel_img.rows

          target_channels[dst_channel] = {
            image: channel_img,
            channel: src_channel,
          }
        end
      end

      img_list = Magick::ImageList.new
      alpha_img = nil

      black = black_image(target_w, target_h, target_depth)
      white = white_image(target_w, target_h, target_depth)

      target_placeholders = {
        Magick::RedChannel => black,
        Magick::GreenChannel => black,
        Magick::BlueChannel => black,
        Magick::AlphaChannel => white,
      }

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
      GC.start
      sleep 0.2

      # https://imagemagick.org/script/command-line-options.php#combine
      # => combine as SRGB
      dst_img = img_list.combine(Magick::SRGBColorspace)

      dst_img.alpha(Magick::SetAlphaChannel)
      dst_img
        .composite_channel!(
          alpha_img,
          0, 0,
          Magick::CopyAlphaCompositeOp,
          Magick::AlphaChannel)

      # NOTE KI force SRGB
      dst_img.colorspace = Magick::SRGBColorspace

      unless dry_run
        FileUtils.mkdir_p(dst_dir)

        # https://unix.stackexchange.com/questions/689906/imagemagick-not-converting-grayscale-to-rgb
        file_format = dst_img.quantum_depth == 16 ? "PNG64:" : "PNG32:"

        info "SAVE: [#{group}] #{file_format + dst_path}"
        dst_img.write(file_format + dst_path)

        dst_digest.write_digest

        info "DONE: [#{group}] #{dst_path}"
      end
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

      channel_id_counts = {}
      parts.each do |tex_info|
        tex_info.target_channel.each_char do |channel_id|
          channel_id_counts[channel_id] ||= 0
          channel_id_counts[channel_id] += 1
        end
      end

      if channel_id_counts.any? { |k, v| v > 1 }
        duplicates = channel_id_counts
          .select { |k, v| v > 1 }
          .map(&:first)
          .map do |channel_id|
            parts.select do |part|
              part.target_channel.chars.any?(channel_id)
            end.map(&:name)
          end

        raise "ERROR: duplicate channel mapping: #{{
          src_dir:,
          group:,
          target_name:,
          parts: parts.map(&:name),
          duplicates:}}"
      end

      # if parts.map(&:target_depth).uniq.size > 1
      #   raise "ERROR: target_depth mismatch: #{{
      #     src_dir:,
      #     group:,
      #     target_name:,
      #     parts: parts.map(&:name),
      #     depths: parts.map(&:target_depth).uniq.size}}"
      # end

      dst_path = "#{dst_dir}/#{target_name}#{BUILD_SUFFIX}.png"

      sorted_parts = parts.sort_by { |e| e.name }

      source_paths = sorted_parts.map do |tex_info|
        "#{src_dir}/#{tex_info.name}"
      end

      dst_digest = TextureDigest.new(
        dst_path,
        source_paths,
        meta: {
          target: File.basename(dst_path),
          type: :mras,
          target_channel: RGBA,
          srgb: false,
        },
        salt: {
          version: MRAS_VERSION,
          size: target_size,
          type: :mras,
          depth: target_depth,
          parts: sorted_parts.map do |tex_info|
            {
              name: tex_info.name,
              source_channel: tex_info.source_channel,
              target_channel: tex_info.target_channel,
              source_depth: tex_info.source_depth,
              srgb: tex_info.srgb,
            }
          end.sort_by { |e| e[:name] }
        },
        force:,
        tid:)

      unless dst_digest.changed?
        return dst_digest.update_if_needed
      end

      info "MRAS: [#{group}] [size=#{target_size}] [depth=#{target_depth}] #{dst_path}"

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
        src_channel_ids = tex_info.source_channel.chars
        dst_channel_ids = tex_info.target_channel.chars

        src_channel_ids.zip(dst_channel_ids).each do |src_channel_id, dst_channel_id|
          src_channel = select_channel(src_channel_id)
          dst_channel = select_channel(dst_channel_id)

          next unless src_channel && dst_channel

          src_path = tex_info.src_path(src_dir)

          info "LOAD: [#{group}] #{dst_channel} = #{src_channel} #{src_path}"

          # https://imagemagick.org/script/command-line-options.php#separate
          # NOTE KI *NOT* supporting non power-of-2 images
          # => should be resonable restriction
          channel_img = Magick::Image.read(src_path)
            .first
            .separate(src_channel)
            .first

          # NOTE KI enforce RGB space (not grayscale)
          channel_img.colorspace = Magick::RGBColorspace

          channel_img = Util.scale_data_image(channel_img, target_size)
          channel_img = channel_img.set_channel_depth(Magick::AllChannels, target_depth)

          target_w = channel_img.columns
          target_h = channel_img.rows

          target_channels[dst_channel] = {
            image: channel_img,
            channel: src_channel,
          }
        end
      end

      img_list = Magick::ImageList.new
      alpha_img = nil

      black = black_image(target_w, target_h, target_depth)
      white = white_image(target_w, target_h, target_depth)

      metal_default = black
      roughness_default = white
      occlusion_default = white
      specular_default = black

      target_placeholders = {
        Magick::RedChannel => metal_default,
        Magick::GreenChannel => roughness_default,
        Magick::BlueChannel => occlusion_default,
        Magick::AlphaChannel => specular_default,
      }

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

        info "SAVE: [#{group}] #{file_format + dst_path}"
        dst_img.write(file_format + dst_path)

        dst_digest.write_digest

        info "DONE: [#{group}] #{dst_path}"
      end
    end

    ########################################
    # HEIGHT
    ########################################
    def create_height_texture(
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

      tex_info = parts.first
      src_path = tex_info.src_path(src_dir)

      dst_digest = TextureDigest.new(
        dst_path,
        [src_path],
        meta: {
          target: File.basename(dst_path),
          type: :height,
          target_channel: RED,
          srgb: false
        },
        salt: {
          version: HEIGHT_VERSION,
          size: target_size,
          type: :height,
          depth: target_depth,
          parts: [
            {
              name: tex_info.name,
              source_channel: tex_info.source_channel,
              target_channel: tex_info.target_channel,
              source_depth: tex_info.source_depth,
              srgb: tex_info.srgb,
            }
          ]
        },
        force:,
        tid:)

      unless dst_digest.changed?
        return dst_digest.update_if_needed
      end

      info "HEIG: [#{group}] [size=#{target_size}] [depth=#{target_depth}] ]#{dst_path}"

      src_channel = select_channel(tex_info.source_channel) || select_channel(RED)

      info "LOAD: [#{group}] Extracting channel #{src_channel} from #{src_path}"

      # Extract the single raw channel
      dst_img = Magick::Image.read(src_path)
        .first
        .separate(src_channel)
        .first

      # Enforce RGB/Linear space for accurate calculations
      dst_img.colorspace = Magick::RGBColorspace
      dst_img = Util.scale_data_image(dst_img, target_size)
      dst_img = dst_img.set_channel_depth(Magick::AllChannels, target_depth)

      # Force into pure Gray Colorspace and GrayscaleType metadata
      dst_img.colorspace = Magick::GRAYColorspace
      dst_img.image_type = Magick::GrayscaleType

      unless dry_run
        FileUtils.mkdir_p(dst_dir)

        # "PNG00:" prefix to strictly enforce a 1-channel 16-bit Grayscale PNG output
        file_format = "PNG00:"

        info "SAVE: [#{group}] #{file_format + dst_path}"
        dst_img.write(file_format + dst_path)

        dst_digest.write_digest

        info "DONE: [#{group}] #{dst_path}"
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

      tex_info = parts.first
      src_path = tex_info.src_path(src_dir)

      dst_digest = TextureDigest.new(
        dst_path,
        [src_path],
        meta: {
          target: File.basename(dst_path),
          type: :displacement,
          target_channel: RED,
          srgb: false,
        },
        salt: {
          version: DISPLACEMENT_VERSION,
          size: target_size,
          type: :displacement,
          depth: target_depth,
          parts: [
            {
              name: tex_info.name,
              source_channel: tex_info.source_channel,
              target_channel: tex_info.target_channel,
              source_depth: tex_info.source_depth,
              srgb: tex_info.srgb,
            }
          ]
        },
        force:,
        tid:)

      unless dst_digest.changed?
        return dst_digest.update_if_needed
      end

      info "DISP: [#{group}] [size=#{target_size}] [depth=#{target_depth}] #{dst_path}"

      src_channel = select_channel(tex_info.source_channel) || select_channel(RED)

      info "LOAD: [#{group}] Extracting channel #{src_channel} from #{src_path}"

      # Separate the single raw displacement data channel
      dst_img = Magick::Image.read(src_path)
        .first
        .separate(src_channel)
        .first

      # Enforce RGB/Linear space for accurate calculations
      dst_img.colorspace = Magick::RGBColorspace

      # Scale to target size utilizing data-preserving Cubic Filter
      dst_img = Util.scale_data_image(dst_img, target_size)
      dst_img = dst_img.set_channel_depth(Magick::AllChannels, target_depth)

      # Force into pure Gray Colorspace and GrayscaleType metadata
      dst_img.colorspace = Magick::GRAYColorspace
      dst_img.image_type = Magick::GrayscaleType

      unless dry_run
        FileUtils.mkdir_p(dst_dir)

        # FIX: Force 8-bit single-channel grayscale output utilizing the "PNG8:" prefix.
        # This completely bypasses the Q16 quantum pipeline expansion during disk write.
        file_format = "PNG8:"

        info "SAVE: [#{group}] #{file_format + dst_path}"
        dst_img.write(file_format + dst_path)

        dst_digest.write_digest

        info "DONE: [#{group}] #{dst_path}"
      end
    end
  end
end
