# frozen_string_literal: true

require 'yaml'
require 'digest'
require 'pathname'
require 'fileutils'
require 'json'
require 'hashie'
require 'logger'

require_relative "../encode"
require_relative "util"

module Encode
  #
  # Digest calculation for single dst_path
  #
  class TextureDigest
    attr_reader :dst_path,
      :source_paths,
      :meta,
      :salt,
      :force,
      :meta_digest,
      :salt_digest,
      :file_digest,
      :tid

    # @param meta data, which does not affect file digest
    def initialize(
      dst_path,
      source_paths,
      salt:,
      meta:,
      force:,
      tid:
    )
      @dst_path = dst_path
      @source_paths = source_paths
      @salt = salt
      @meta = meta
      @force = force
      @tid = tid
    end

    def info(msg)
      puts "TID[#{tid}]: #{msg}"
    end

    #
    # @return true if digest has changed
    #
    def changed?
      @data = read_digest

      @meta_changed = meta_sha_changed?

      return true if force
      return true if salt_sha_changed?

      unless timestamp_changed?
        info "SKIP: TIMESTAMP_NOT_ChANGED #{dst_path}"
        return
      end

      unless file_sha_changed
        info "SKIP: DIGEST_NOT_ChANGED #{dst_path}"
        return
      end

      true
    end

    def read_digest
      digest_path = "#{dst_path}.digest"
      return {} unless File.exist?(digest_path)
      YAML.load(File.read(digest_path), symbolize_names: true)
    rescue => e
      puts "ERROR: FAILED_TO_READ_DIGEST: #{e.message}"
      {}
    end

    def update_if_needed
      return unless @meta_changed

      puts "META:UPDATE: #{dst_path}"

      write_digest
    end

    def write_digest
      digest_path = "#{dst_path}.digest"
      data = {
        meta_sha_digest: meta_digest || create_meta_digest,
        salt_sha_digest: salt_digest || create_salt_digest,
        file_sha_digest: file_digest || create_file_digest,
        meta:,
        salt:,
        sources: source_paths.sort.map do |src_path|
          img = Magick::Image.ping(src_path).first
          {
            path: src_path,
            size: "#{img.columns}x#{img.rows}",
            depth: img.quantum_depth,
          }
        end
      }
      File.write(digest_path, JSON.parse(data.to_json).to_yaml)
      @data = data
    end

    def timestamp_changed?
      return true unless File.exist?(dst_path)

      dst_mtime = File.mtime(dst_path)
      source_paths.any? do |path|
        File.mtime(path) > dst_mtime
      end
    end

    #
    # @return SHA if sha changed, nil otherwise
    #
    def meta_sha_changed?
      old_digest = @data[:meta_sha_digest]
      sha_digest = create_meta_digest

      if old_digest == sha_digest && File.exist?(dst_path) && !force
        return
      end

      @meta_digest = sha_digest
    end

    #
    # @return SHA if sha changed, nil otherwise
    #
    def salt_sha_changed?
      old_digest = @data[:salt_sha_digest]
      sha_digest = create_salt_digest

      if old_digest == sha_digest && File.exist?(dst_path) && !force
        return
      end

      @salt_digest = sha_digest
    end

    #
    # @return SHA if sha changed, nil otherwise
    #
    def file_sha_changed?(dst_path, source_paths, salt)
      old_digest = @data[:file_sha_digest]
      sha_digest = create_file_digest

      if old_digest == sha_digest && File.exist?(dst_path) && !force
        return
      end

      @file_digest = sha_digest
    end

    # https://stackoverflow.com/questions/64130698/sha256-value-for-large-binaries-when-using-ruby
    def create_meta_digest
      sha = Digest::SHA2.new
      sha << JSON.parse(meta.to_json).to_yaml
      sha.hexdigest
    end

    # https://stackoverflow.com/questions/64130698/sha256-value-for-large-binaries-when-using-ruby
    def create_salt_digest
      sha = Digest::SHA2.new
      sha << JSON.parse(salt.to_json).to_yaml
      sha.hexdigest
    end

    # https://stackoverflow.com/questions/64130698/sha256-value-for-large-binaries-when-using-ruby
    def create_file_digest
      sha = Digest::SHA2.new

      sha << JSON.parse(salt.to_json).to_yaml

      source_paths.sort.each do |file_path|
        File.open(file_path) do |f|
          while chunk = f.read(8192)
            sha << chunk
          end
        end
      end

      sha.hexdigest
    end
  end
end
