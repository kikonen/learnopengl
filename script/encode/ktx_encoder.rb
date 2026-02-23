# frozen_string_literal: true

require 'debug'
require 'pathname'
require 'thor'
require 'fileutils'
require 'hashie'

require_relative "util"
require_relative "file_encoder"

module Encode
  ############################################################
  # KTX
  ############################################################
  class KtxEncoder < FileEncoder
    attr_reader:digest_path

    def initialize(
      digest_path:,
      src_dir:,
      dst_dir:,
      force:,
      dry_run:
    )
      super(src_dir:, dst_dir:, target_size:, force:, dry_run:)

      @digest_path = digest_path
    end

    def encode(tid:)
      @tid = tid

      digest_data = YAML.load(File.read(digest_path), symbolize_names: true)
      ap digest_data

      src_path = digest_path.gsub(".digest", "")
      salt = digest_data[:salt]
      type = (salt[:type] || :color).to_sym

      encode_ktx_image(
        src_path:,
        dst_dir:,
        type:,
        target_type: salt[:channels] || RGB,
        srgb: salt[:srgb] || false,
        normal_mode: type == :normal
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
