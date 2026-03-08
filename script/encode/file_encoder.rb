# frozen_string_literal: true

require 'rmagick'

require_relative "util"
require_relative "texture_digest"


module Encode
  ################################################################################
  # File encoder
  ################################################################################
  class FileEncoder
    MAGICK_CHANNELS = {
      NONE => nil,
      RED => Magick::RedChannel,
      GREEN => Magick::GreenChannel,
      BLUE => Magick::BlueChannel,
      ALPHA => Magick::AlphaChannel,
    }.freeze

    attr_reader :src_dir,
      :dst_dir,
      :target_size,
      :force,
      :dry_run,
      :tid

    def initialize(
      src_dir:,
      dst_dir:,
      target_size:,
      force:,
      dry_run:
    )
      @src_dir = src_dir
      @dst_dir = dst_dir
      @target_size = target_size
      @force = force
      @dry_run = dry_run
    end

    def encode(tid:)
      raise "NYI!"
    end

    def info(msg)
      puts "TID[#{@tid}]: #{msg}"
    end

    def select_channel(channel_id)
      MAGICK_CHANNELS[channel_id&.upcase]
    end

    def select_channels(channel_ids)
      channel_ids.chars.map do |channel_id|
        MAGICK_CHANNELS[channel_id&.upcase]
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
  end
end
