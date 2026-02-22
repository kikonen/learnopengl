# frozen_string_literal: true

require 'open3'
require 'debug'
require 'yaml'
require 'digest'
require 'pathname'
require 'thor'
require 'fileutils'
require 'json'
require 'hashie'
require 'logger'
require "rmagick"

require_relative "util"
require_relative "file_encoder"

module Encode
  ############################################################
  # KTX
  ############################################################
  class KtxEncoder < FileEncoder
    attr_reader:src_path,
      :dst_dir,
      :type,
      :target_type,
      :srgb,
      :normal_mode

    def initialize(
      src_path:,
      src_dir:,
      dst_dir:,
      type:,
      target_type: RGB,
      srgb: true,
      normal_mode: false,
      target_size:,
      force:
    )
      super(src_dir:, dst_dir:, target_size:, force:)

      @src_path = src_path
      @type = type
      @target_type = target_type
      @srgb = srgb
      @normal_mode = normal_mode
    end

    def encode(tid:)
      @tid = tid

      encode_ktx_image(
        src_path:,
        dst_dir:,
        type:,
        target_type:,
        srgb:,
        normal_mode:
      )
    end

    private

    def encode_ktx_image(
      src_path:,
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

      dst_digest = TextureDigest.new(
        dst_path,
        [src_path],
        {
          version: KTX_VERSION,
          target_size:,
          parts: [
            name: File.basename(src_path)
          ]
        },
        force,
        tid)

      return unless dst_digest.changed?

      info "ENCODE[#{type.to_s.upcase}]: #{src_path}"

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

      info "CMD: #{cmd.join(" ")}"

      unless dry_run
        info "WRITE: #{dst_path}"

        %x{#{cmd.join(" ")}}

        if File.exist?(dst_tmp_path)
          FileUtils.cp(dst_tmp_path, dst_path)
          FileUtils.rm_f(dst_tmp_path)

          info "DONE:  #{dst_path}"
        end

        unless File.exist?(dst_path)
          info "FAIL:  #{src_path}"
          return
        end

        dst_digest.write_digest
      end
    end
  end
end
