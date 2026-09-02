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
      :target_depth,
      :force,
      :dry_run,
      :tid

    def initialize(
      src_dir:,
      dst_dir:,
      target_size:,
      target_depth:,
      force:,
      dry_run:
    )
      @src_dir = src_dir
      @dst_dir = dst_dir
      @target_size = target_size
      @target_depth = target_depth
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
      @black_image ||= {}
      @black_image[[target_w, target_h, target_depth]] ||=
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
          img = Magick::Image.read("#{assets_root_dir}/textures/placeholder/black_color.png").first
          img = img
            .separate(Magick::RedChannel)
            .first

          # FIX: Pakotetaan lineaariseksi dataksi erottelun jälkeen!
          img.colorspace = Magick::RGBColorspace
          img = Util.scale_data_image(img, target_w) # Käytetään CubicFilteriä resizen sijaan
          img.set_channel_depth(Magick::AllChannels, target_depth)
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
          img = Magick::Image.read("#{assets_root_dir}/textures/placeholder/white_color.png").first
          img = img
            .separate(Magick::RedChannel)
            .first

          # NOTE KI enforce RGB colorspace (instead of gray)
          img.colorspace = Magick::RGBColorspace
          img = Util.scale_data_image(img, target_w)
          img.set_channel_depth(Magick::AllChannels, target_depth)
        end
    end
  end
end
