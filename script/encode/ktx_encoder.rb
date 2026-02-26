# frozen_string_literal: true

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
    GPU_FORMATS = {
      intel: "uastc",
      # nvidia: "astc",
      nvidia: "uastc",
    }

    GPU_NORMAL_FORMATS = {
      intel: "uastc",
      # nvidia: "astc",
      nvidia: "uastc",
    }

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

      src_path = digest_path.gsub(".digest", "")
      meta = digest_data[:meta]
      type = (meta[:type] || :color).to_sym

      if meta[:no_ktx]
        info "NO_KTX: #{src_path}"
        return
      end

      gpu_type = :intel

      encode_ktx_image(
        src_path:,
        src_digests: {
          meta: digest_data[:meta_sha_digest],
          salt: digest_data[:salt_sha_digest],
          file: digest_data[:file_sha_digest],
        },
        dst_dir:,
        type:,
        #target_type: meta[:target_channe] || RGBA,
        target_type: RGBA,
        srgb: meta[:srgb] || false,
        normal_mode: type == :normal,
        gpu_type:
      )
    end

    private

    def encode_ktx_image(
      src_path:,
      src_digests:,
      dst_dir:,
      type:,
      target_type: RGBA,
      srgb: true,
      normal_mode: false,
      gpu_type:
    )
      basename = File.basename(src_path, ".*")
      dst_path = "#{dst_dir}/#{basename}.ktx"
      dst_tmp_path = "#{dst_path}.tmp"

      encoding = normal_mode ? GPU_NORMAL_FORMATS[gpu_type] : GPU_FORMATS[gpu_type]

      # NOTE KI VK_FORMAT_ASTC_6x6_SRGB_BLOCK. That's ASTC 6x6 block size.
      # Intel Arc does support ASTC, but check if it supports that
      # specific block size. The most universally supported is 4x4.
      base_cmd = [
        "toktx.exe",
        "--t2",
        "--verbose",
        "--genmipmap",
        #"--automipmap",
        "--encode",
        encoding,
        #"--astc_blk_d", "4x4",
        "--target_type",
        target_type,
        "--assign_oetf",
        srgb ? "srgb" : "linear",
        "--lower_left_maps_to_s0t0",
      ].compact

      if normal_mode
        base_cmd << "--normal_mode"
        "--uastc_rdo_l"
      end

      dst_digest = TextureDigest.new(
        dst_path,
        [src_path],
        meta: {
          type:,
          target_channel: target_type,
          srgb:,
        },
        salt: {
          version: KTX_VERSION,
          size: target_size,
          type:,
          cmd: base_cmd,
          parts: [
            name: File.basename(src_path),
            digests: src_digests,
            target_channel: target_type,
            srgb:,
          ]
        },
        force:,
        tid:)

      unless dst_digest.changed?
        dst_digest.update_if_needed
        return
      end

      info "ENCODE[#{type.to_s.upcase}]: #{src_path}"

      unless dry_run
        FileUtils.mkdir_p(dst_dir)

        if File.exist?(dst_tmp_path)
          FileUtils.rm(dst_tmp_path)
        end
      end

      cmd = base_cmd.dup

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
