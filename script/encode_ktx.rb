# frozen_string_literal: true

# ENV['UBY_DEBUG_HISTORY_FILE'] = File.absolute_path(
#   "#{File.dirname(__FILE__)}/../.rdbg_history")
# ENV['RUBY_DEBUG_SAVE_HISTORY'] = "10000"

require 'debug'
require 'thor'
require 'logger'

require_relative "encode/shared"
require_relative "encode/util"
require_relative "encode/meta_resolver"

require_relative "encode/image_encoder"
require_relative "encode/combine_encoder"
require_relative "encode/ktx_encoder"

require_relative "encode/async_processor"

module Encode
  ################################################################################
  # Converter
  ################################################################################
  class Converter < Thor
    attr_reader :assets_root_dir,
      :build_root_dir,
      :target_size,
      :use_combine,
      :use_encode,
      :use_encode_ktx,
      :recursive,
      :force,
      :dry_run,
      :types,
      :thread_count

    def self.exit_on_failure?
      true
    end

    ############################################################
    #
    ############################################################
    desc "meta", "Generate texture meta info"
    method_option :src, default: 'resources/assets'
    method_option :assets_root_dir, default: 'resources/assets'
    method_option :target_size,
      type: :numeric,
      required: false,
      default: 2048
    method_option :ext,
      type: :array,
      default: nil
    method_option :recursive,
      type: :boolean,
      default: false
    method_option :force,
      type: :boolean,
      default: false
    method_option :dry_run,
      type: :boolean,
      default: true
    def meta
      src_dir = options[:src]
      extensions = options[:ext] || EXTENSIONS
      extensions = extensions.map(&:downcase)

      @assets_root_dir = options[:assets_root_dir]
      @target_size = options[:target_size]
      @recursive = options[:recursive]
      @force = options[:force]
      @dry_run = options[:dry_run]

      @assets_root_dir = Util.clean_dir_path(@assets_root_dir)
      src_dir = Util.clean_dir_path(src_dir)

      puts "SRC_DIR:     #{src_dir}"
      puts "EXT:         #{extensions}"
      puts "TARGET_SIZE: #{target_size}"
      puts "FORCE:       #{force}"
      puts "RECURSIVE:   #{recursive}"
      puts "DRY_RUN:     #{dry_run}"

      MetaResolver
        .new(
          extensions:,
          recursive:,
          dry_run:
        )
        .generate_metadata(src_dir:)
    end

    ############################################################
    #
    ############################################################
    desc "build", "build KTX assets"
    method_option :src, default: 'resources/assets'
    method_option :assets_root_dir, default: 'resources/assets'
    method_option :build_root_dir, default: 'resources/build'
    method_option :target_size,
      type: :numeric,
      required: true
    method_option :ext,
      type: :array,
      default: nil
    method_option :all,
      type: :boolean,
      default: false
    method_option :combine,
      type: :boolean,
      default: false
    method_option :encode,
      type: :boolean,
      default: false
    method_option :ktx,
      type: :boolean,
      default: false
    method_option :recursive,
      type: :boolean,
      default: false
    method_option :force,
      type: :boolean,
      default: false
    method_option :dry_run,
      type: :boolean,
      default: true
    method_option :type,
      type: :array,
      default: ['all']
    method_option :thread_count,
      type: :numeric,
      default: 4
    def build
      src_dir = options[:src]
      extensions = options[:ext] || EXTENSIONS
      extensions = extensions.map(&:downcase)

      @assets_root_dir = options[:assets_root_dir]
      @build_root_dir = options[:build_root_dir]
      @target_size = options[:target_size]
      @recursive = options[:recursive]
      @force = options[:force]
      @dry_run = options[:dry_run]
      @types = options[:type].map(&:to_sym)
      @use_combine = options[:combine]
      @use_encode = options[:encode]
      @use_encode_ktx = options[:ktx]
      @thread_count = options[:thread_count]

      if options[:all]
        @use_combine = true
        @use_encode = true
        @use_encode_ktx = true
      end

      unless @use_combine || @use_encode || @use_encode_ktx
        puts "--combine or --encode or --ktx required"
        exit
      end

      @assets_root_dir = Util.clean_dir_path(@assets_root_dir)
      @build_root_dir = Util.clean_dir_path(@build_root_dir)
      src_dir = Util.clean_dir_path(src_dir)

      puts "SRC_DIR:     #{src_dir}"
      puts "ASSETS_DIR:  #{assets_root_dir}"
      puts "BUILD_DIR:   #{build_root_dir}"
      puts "TYPE:        #{types}"
      puts "TARGET_SIZE: #{target_size}"
      puts "COMBINE:     #{use_combine}"
      puts "ENCODE:      #{use_encode}"
      puts "ENCODE_KTX:  #{use_encode_ktx}"
      puts "EXT:         #{extensions}"
      puts "FORCE:       #{force}"
      puts "RECURSIVE:   #{recursive}"
      puts "DRY_RUN:     #{dry_run}"
      puts "THREAD_COUNT:#{thread_count}"

      @processor = AsyncProcessor.new(thread_count:)

      if use_combine
        puts "[COMBINE]"
        build_dir(
          src_dir:,
          extensions:,
          combine_pass: true,
          encode_pass: false,
          encode_ktx_pass: false
        )
      end

      if use_encode
        puts "[ENCODE]"
        build_dir(
          src_dir:,
          extensions:,
          combine_pass: false,
          encode_pass: true,
          encode_ktx_pass: false)
      end

      if use_encode_ktx
        puts "[ENCODE_KTX]"
        build_dir(
          src_dir:,
          extensions:,
          combine_pass: false,
          encode_pass: false,
          encode_ktx_pass: true)
      end

      puts "ENCODE: #{@processor.size}"

      @processor.start
      @processor.wait
      @processor.shutdown
      puts "REMAINING: #{@processor.size}"
    end

    private

    ############################################################
    #
    ############################################################
    def build_dir(
      src_dir:,
      extensions:,
      combine_pass:,
      encode_pass:,
      encode_ktx_pass:)

      plain_dir = src_dir[assets_root_dir.length + 1, src_dir.length]
      dst_dir = "#{build_root_dir}/#{target_size}/#{plain_dir}"
      puts "DIR: #{src_dir} => #{dst_dir}"

      metadata = Util.read_metadata(src_dir:)

      combine_textures = {}
      encode_textures = []

      metadata.textures&.each do |tex_info|
        name = tex_info.name.downcase
        next unless name

        action = tex_info.action.to_sym
        type = tex_info.type.to_sym

        unless extensions.any? { |ext| name.downcase.end_with?(ext) }
          puts "IGNORE: wrong ext - #{type} #{src_dir} #{tex_info.name}"
        end

        if action == :combine
          (combine_textures[tex_info.target_name] ||= []) << tex_info
          next
        end

        if action == :encode
          encode_textures << tex_info
          next
        end

        puts "IGNORE: #{type} #{src_dir} #{tex_info.name}"
      end

      if combine_pass
        combine_textures.each do |target_name, parts|
          target_mode = parts.first.mode.to_sym

          @processor.add_task(
            class: 'Encode::CombineEncoder',
            args: {
              src_dir:,
              dst_dir:,
              target_name:,
              target_mode:,
              parts:,
              target_size:,
              force:,
              dry_run:
            })
        end
      end

      if encode_pass
        encode_textures.each do |tex_info|
          @processor.add_task(
            class: "Encode::ImageEncoder",
            args: {
              src_dir:,
              dst_dir:,
              tex_info:,
              target_size:,
              force:,
              dry_run:})
        end
      end

      if encode_ktx_pass
        encode_textures.each do |tex_info|
          src_path = "#{src_dir}/#{name}"

          @processor.add_task(
            class: "Encode::KtxEncoder",
            args: {
              src_path:,
              dst_dir:,
              type: tex_info.type,
              target_type: tex_info.target_type || RGB,
              srgb: tex_info.srgb || false,
              normal_mode: tex_info.mode-to_sym == :normal,
              target_size:,
              force:,
              dry_run:})
        end
      end

      if recursive
        sub_dirs = []

        files = Util.list_files(src_dir)
        files.sort_by(&:downcase).each do |f|
          if File.directory?("#{src_dir}/#{f}")
            sub_dirs << f
          end
        end

        sub_dirs.each do |sub_dir|
          build_dir(
            src_dir: [src_dir, sub_dir].join('/'),
            extensions:,
            combine_pass:,
            encode_pass:,
            encode_ktx_pass:)
        end
      end
    end
  end
end

Encode::Converter.start(ARGV)
