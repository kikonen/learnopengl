# frozen_string_literal: true

require 'debug'
require 'yaml'
require 'pathname'
require 'fileutils'
require 'json'
require 'hashie'
require 'logger'

require_relative "shared"
require_relative "util"
require_relative "texture_info"
require_relative "metadata"

module Encode
  ################################################################################
  # Meta data encoding
  ################################################################################
  class MetaResolver
    # NOTE KI match against *basename" without *extension*
    # => this \z instead of \.
    REG_EX = [
      # Level 1: most accurate
      {
        color: [
          /\Acolor\z/,
          /\Acolor[-_ ]/,
          /[-_ ]color[-_ ]/,
          /[-_ ]color\z/,
          ###
          /\Abasecolor\z/,
          /\Abasecolor[-_ ]/,
          /[-_ ]basecolor[-_ ]/,
          /[-_ ]basecolor/,
          /[-_ ]basecolor\z/,
          /[-_ ]basecolrgba\z/,
          ###
          /\Adiffuse\z/,
          /\Adiffuse[-_ ]/,
          /[-_ ]diffuse[-_ ]/,
          /[-_ ]diffuse\z/,
          /[-_ ]diffuse[\d]+[-_. ]/,
          ###
          /\Aalbedo\z/,
          /\Aalbedo[-_ ]/,
          /[-_ ]albedo/,
          /[-_ ]albedo\z/,
          ###
        ],
        emission: [
          /\Aemission\z/,
          /\Aemission[-_ ]/,
          /[-_ ]emission[-_ ]/,
          /[-_ ]emission\z/,
          ###
          /\Aemissive\z/,
          /\Aemissive[-_ ]/,
          /[-_ ]emissive[-_ ]/,
          /[-_ ]emissive\z/,
          ###
          /\Aglow\z/,
          /\Aglow[-_ ]/,
          /[-_ ]glow[-_ ]/,
          /[-_ ]glow\z/,
          ###
        ],
        opacity: [
          /\Aopacity\z/,
          /\Aopacity[-_ ]/,
          /[-_ ]opacity[-_ ]/,
          /[-_ ]opacity\z/,
          ###
          /\Aalpha\z/,
          /\Aalpha[-_ ]/,
          /[-_ ]alpha[-_ ]/,
          /[-_ ]alpha\z/,
          ###
        ],
        normal: [
          /[-_ ]normalgl[-_ ]/,
          /[-_ ]normalgl\z/,
          ###
          /\Anormal\z/,
          /\Anormal[-_ ]/,
          /[-_ ]normal[-_ ]/,
          /[-_ ]normal\z/,
          ###
          /\Anormals\z/,
          /\Anormals[-_ ]/,
          /[-_ ]normals[-_ ]/,
          /[-_ ]normals\z/,
          ###
        ],
        bump: [
          /\Abump\z/,
          /\Abump[-_ ]/,
          /[-_ ]bump[-_ ]/,
          /[-_ ]bump\z/,
          ###
        ],
        metal: [
          /\Ametalness\z/,
          /\Ametalness[-_ ]/,
          /[-_ ]metalness[-_ ]/,
          /[-_ ]metalness\z/,
          ###
          /\Ametallic\z/,
          /\Ametallic[-_ ]/,
          /[-_ ]metallic[-_ ]/,
          /[-_ ]metallic\z/,
          ###
          /[-_ ]metallicsmoothness/,
          ###
          /\Ametalness\z/,
          /\Ametalness[-_ ]/,
          /[-_ ]metalness[-_ ]/,
          /[-_ ]metalness\z/,
        ],
        roughness: [
          /\Aroughness\z/,
          /\Aroughness[-_ ]/,
          /[-_ ]roughness[-_ ]/,
          /[-_ ]roughness\z/,
        ],
        occlusion: [
          /\Aambientocclusion\z/,
          /\Aambientocclusion[-_ ]/,
          /[-_ ]ambientocclusion[-_ ]/,
          /[-_ ]ambientocclusion\z/,
          ###
          /\Aocclusion\z/,
          /\Aocclusion[-_ ]/,
          /[-_ ]occlusion[-_ ]/,
          /[-_ ]occlusion\z/,
        ],
        metal_roughness: [
          /metallicroughness/,
          /metalrough/,
        ],
        metal_roughness_occlusion: [
          /metallicroughnessocclusion/,
        ],
        roughness_metal_occlusion: [
          /roughnessmetallicocclusion/,
        ],
        roughness_occlusion_metal: [
          /roughnessocclusionmetal/,
        ],
        occlusion_roughness_metal: [
          /occlusionroughnessmetal/,
        ],
        # Elf_LegArmor_MetalAOHeightRough.png.meta
        metal_occlusion_height_roughness: [
          /metalaoheightrough/,
        ],
        # Kitbash MADS
        metal_occlusion_height_smoothness: [
          /_mads\z/,
        ],
        displacement: [
          /\Adisplacement\z/,
          /\Adisplacement[-_ ]/,
          /[-_ ]displacement[-_ ]/,
          /[-_ ]displacement\z/,
          ###
          /\Adepth\z/,
          /\Adepth[-_ ]/,
          /[-_ ]depth[-_ ]/,
          /[-_ ]depth\z/,
          ###
        ],
        height: [
          /_height\z/,
        ],
        cavity: [
          /\Acavity\z/,
          /\Acavity[-_ ]/,
          /[-_ ]cavity[-_ ]/,
          /[-_ ]cavity\z/,
        ],
        specular: [
          /\Aspecular\z/,
          /\Aspecular[-_ ]/,
          /[-_ ]specular[-_ ]/,
          /[-_ ]specular\z/,
        ],
        gloss: [
          /\Agloss\z/,
          /\Agloss[-_ ]/,
          /[-_ ]gloss[-_ ]/,
          /[-_ ]gloss\z/,
        ],
        noise: [
          /\Anoise\z/,
          /\Anoise[-_ ]/,
          /[-_ ]noise[-_ ]/,
          /[-_ ]noise\z/,
        ],
        preview: [
          /preview/,
        ],
      },
      # Level 2; less accurate
      {
        color: [
          /\Acol\z/,
          /\Acol[-_ ]/,
          /[-_ ]col[-_ ]/,
          /[-_ ]col\z/,
          ###
          /\Aalb\z/,
          /\Aalb[-_ ]/,
          /[-_ ]alb[-_ ]/,
          /[-_ ]alb\z/,
          ###
          /\Adiff\z/,
          /\Adiff[-_ ]/,
          /[-_ ]diff[-_ ]/,
          /[-_ ]diff\z/,
          ###
        ],
        emission: [
          /\Aemi\z/,
          /\Aemi[-_ ]/,
          /[-_ ]emi[-_ ]/,
          /[-_ ]emi\z/,
          ###
        ],
        opacity: [
        ],
        normal: [
          /\Anrm\z/,
          /\Anrm[-_ ]/,
          /[-_ ]nrm[-_ ]/,
          /[-_ ]nrm\z/,
          ###
          /\Anml\z/,
          /\Anml[-_ ]/,
          /[-_ ]nml[-_ ]/,
          /[-_ ]nml\z/,
          ###
          /\Anor\z/,
          /\Anor[-_ ]/,
          /[-_ ]nor[-_ ]/,
          /[-_ ]nor\z/,
          ###
        ],
        metal: [
          /\Amet\z/,
          /\Amet[-_ ]/,
          /[-_ ]met[-_ ]/,
          /[-_ ]met\z/,
          ###
        ],
        roughness: [
          /\Argh\z/,
          /\Argh[-_ ]/,
          /[-_ ]rgh[-_ ]/,
          /[-_ ]rgh\z/,
          ###
          /\Arough\z/,
          #/\Arough[-_ ]/,
          /[-_ ]rough[-_ ]/,
          /[-_ ]rough\z/,
          ###
        ],
        occlusion: [
        ],
        metal_roughness_occlusion: [
          /\Amra\z/,
          /\Amra[-_ ]/,
          /[-_ ]mra[-_ ]/,
          /[-_ ]mra\z/,
        ],
        roughness_metal_occlusion: [
          /\Arma\z/,
          /\Arma[-_ ]/,
          /[-_ ]rma[-_ ]/,
          /[-_ ]rma\z/,
        ],
        roughness_occlusion_metal: [
          /\Arom\z/,
          /\Arom[-_ ]/,
          /[-_ ]rom[-_ ]/,
          /[-_ ]rom\z/,
        ],
        occlusion_roughness_metal: [
          /\Aorm\z/,
          /\Aorm[-_ ]/,
          /[-_ ]orm[-_ ]/,
          /[-_ ]orm\z/,
        ],
        displacement: [
          /\Adisp\z/,
          /\Adisp[-_ ]/,
          /[-_ ]disp[-_ ]/,
          /[-_ ]disp\z/,
        ],
        specular: [
          /\Aspec\z/,
          /\Aspec[-_ ]/,
          /[-_ ]spec[-_ ]/,
          /[-_ ]spec\z/,
        ],
        gloss: [
        ],
        preview: [
        ],
      },
      # Level 3; even less accurate (TLA, OLA)
      {
        color: [
          /\Abc\z/,
          /\Abc[-_ ]/,
          /[-_ ]bc[-_ ]/,
          /[-_ ]bc\z/,
          ###
          /\Ac\z/,
          /[-_ ]c\z/,
          ###
          /\Aa\z/,
          /[-_ ]a[-_ ]\z/,
          /[-_ ]a\z/,
          ###
          /\Aa_m\z/,
          /[-_ ]a_m[-_ ]\z/,
          /[-_ ]a_m\z/,
          ###
          /\Ab\z/,
          /[-_ ]b\z/,
          ###
          /\Ad\z/,
          /[-_ ]d\z/,
          ###
          /[-_ ]tx\z/,
          ###
        ],
        opacity: [
        ],
        normal: [
          /[-_ ]tn\z/,
          ###
          /[-_ ]n\z/,
          ###
        ],
        metal: [
        ],
        roughness: [
          /\Ar\z/,
          /[-_ ]r\z/,
          ###
        ],
        occlusion: [
          /\Aao\z/,
          /\Aao[-_ ]/,
          /[-_ ]ao[-_ ]/,
          /[-_]ao\z/,
        ],
        metal_roughness_occlusion: [
        ],
        roughness_metal_occlusion: [
        ],
        roughness_occlusion_metal: [
        ],
        displacement: [
        ],
        specular: [
        ],
        gloss: [
        ],
        preview: [
        ],
      },
    ]

    attr_reader :extensions,
      :recursive,
      :dry_run

    def initialize(
      extensions:,
      recursive:,
      dry_run:
    )
      @extensions = extensions
      @recursive = recursive
      @dry_run = dry_run
    end

    def info(msg)
      puts msg
    end

    def generate_metadata(
      src_dir:
    )
      metadata = Util.read_metadata(src_dir:)

      manual_textures = {}
      textures = []
      sub_dirs = []

      metadata[:textures]&.each do |info|
        next unless info[:manual]
        manual_textures[info[:name]] = info
      end

      # TODO KI ktx options
      #
      # --resize <width>x<height>
      #              Resize images to @e width X @e height. This should not be used
      #              with @b--mipmap as it would resize all the images to the same
      #              size. Resampler options can be set via --filter and --fscale.
      #
      # --input_swizzle <swizzle>
      #              Swizzle the input components according to swizzle which is an
      #              alhpanumeric sequence matching the regular expression
      #              ^[rgba01]{4}$.

      files = Util.list_files(src_dir)
      files.sort_by(&:downcase).each do |f|
        name = File.basename(f)
        plain_name = File.basename(f, ".*")

        old_tex_info = manual_textures[name]

        src_path = "#{src_dir}/#{name}"
        if File.directory?(src_path)
          sub_dirs << name
          next
        end

        img = nil
        tex_info = nil

        file_ext = File.extname(f).downcase[1, 5]
        if extensions.include?(file_ext)
          img = Magick::Image.ping(src_path).first

          type = detect_type(name)

          case type
          when :color
            tex_info = {
              type: :color,
              action: :encode,
            }
          when :emission
            tex_info = {
              type: :emission,
              action: :encode,
            }
          when :normal
            tex_info = {
              type: :normal,
              action: :encode,
              mode: :normal,
              # NOTE KI only 3 channels in normal
              # => DROP alpha as redundant
              target_channel: RGB,
              srgb: false,
            }
          when :bump
            tex_info = {
              type: :bump,
              action: :encode,
              mode: :bump,
              # NOTE KI only 1 channel in bump
              # i.e. not same as normal
              target_channel: RED,
              srgb: false,
            }
          when :specular
            tex_info = {
              type: :specular,
              action: :skip,
              srgb: false,
            }
          when :opacity
            tex_info = {
              type: :opacity,
              action: :encode,
              #mode: MODE_OPACITY,
              #target_name: OPACITY_MAP,
              #source_channel: RED,
              #target_channel: RED,
              srgb: false,
            }
          when :metal
            tex_info = {
              group: 'default',
              type: :metalness,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED,
              target_channel: GREEN,
              srgb: false,
            }
          when :roughness
            tex_info = {
              group: 'default',
              type: :roughness,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED,
              target_channel: BLUE,
              srgb: false,
            }
          when :occlusion
            tex_info = {
              group: 'default',
              type: :occlusion,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED,
              target_channel: RED,
              srgb: false,
            }
          when :metal_roughness
            tex_info = {
              group: 'default',
              type: :metal_roughness,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN,
              target_channel: GREEN_BLUE,
              srgb: false,
            }
          when :metal_roughness_occlusion
            tex_info = {
              group: 'default',
              type: :metal_roughness_occlusion,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_BLUE,
              target_channel: GREEN_BLUE_RED,
              srgb: false,
            }
          when :roughness_metal_occlusion
            tex_info = {
              group: 'default',
              type: :roughness_metal_occlusion,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_BLUE,
              target_channel: BLUE_GREEN_RED,
              srgb: false,
            }
          when :roughness_occlusion_metal
            tex_info = {
              group: 'default',
              type: :roughness_occlusion_metal,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_BLUE,
              target_channel: BLUE_RED_GREEN,
              srgb: false,
            }
          when :occlusion_roughness_metal
            tex_info = {
              group: 'default',
              type: :occlusion_roughness_metal,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_BLUE,
              target_channel: RED_BLUE_GREEN,
              srgb: false,
            }
          when :metal_occlusion_height_roughness
            tex_info = {
              group: 'default',
              type: :metal_occlusion_height_roughness,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_ALPHA,
              target_channel: GREEN_RED_BLUE,
              srgb: false,
            }
          when :metal_occlusion_height_smoothness
            tex_info = {
              group: 'default',
              type: :metal_occlusion_height_smoothness,
              action: :combine,
              mode: MODE_MRAS,
              target_name: MRAS_MAP,
              source_channel: RED_GREEN_ALPHA,
              target_channel: GREEN_RED_BLUE,
              srgb: false,
            }
          when :displacement
            tex_info = {
              group: 'default',
              type: :displacement,
              action: :combine,
              mode: MODE_DISPLACEMENT,
              target_name: DISPLACEMENT_MAP,
              source_channel: RED,
              target_channel: RED,
              srgb: false,
            }
          when :height
            tex_info = {
              group: 'default',
              type: :displacement,
              action: :combine,
              mode: MODE_DISPLACEMENT,
              target_name: DISPLACEMENT_MAP,
              source_channel: RED,
              target_channel: RED,
              srgb: false,
            }
          when :cavity
            tex_info = {
              group: 'default',
              type: :cavity,
              action: :skip,
              mode: :cavity,
              target_name: CAVITY_MAP,
              source_channel: RED,
              target_channel: RED,
              srgb: false,
            }
          when :gloss
            tex_info = {
              type: :gloss,
              action: :skip,
            }
          when :noise
            tex_info = {
              type: :noise,
              action: :copy,
              srgb: false,
            }
          when :preview
            tex_info = {
              type: :preview,
              action: :skip,
            }
          else
            tex_info = {
              type: :unknown,
              action: :copy,
            }
          end

          if tex_info
            if img.colorspace == Magick::GRAYColorspace
              channels = RED
            else
              channels = img.alpha? ? RGBA : RGB
            end

            base = {
              name:,
              target_name: plain_name,
              group: nil,
              type: :unknown,
              action: :skip,
              mode: 'copy',
              detected_channels: channels,
              source_channel: channels,
              target_channel: channels,
              source_depth: img.quantum_depth,
              target_depth: img.quantum_depth,
              source_size: "#{img.columns}x#{img.rows}",
              srgb: img.colorspace == Magick::SRGBColorspace,
              manual: false,
            }
            tex_info = base.merge(tex_info)
          end
        end

        if tex_info
          tex_info = tex_info.merge(old_tex_info) if old_tex_info
        else
          # NOTE KI keep old dead manual entries
          tex_info = old_tex_info
        end

        if tex_info
          if tex_info[:type] == :unknown
            info "**WARN** unknown type: #{src_dir}  #{tex_info[:name]} **"
          end

          textures << tex_info
        end
      end

      need_process = !textures.empty?

      begin
        grouped_textures = group_by_prefix(textures)

        texture_to_groups = {}

        grouped_textures.each do |group, group_textures|
          group_textures.each do |tex_info|
            texture_to_groups[tex_info[:name]] ||= []
            texture_to_groups[tex_info[:name]] << group
          end
        end

        textures.each do |tex_info|
          groups = texture_to_groups[tex_info[:name]]
          next unless groups

          # NOTE KI ignore group with single entry if better ones exist
          # => at extreme there is no groups
          group_sizes = groups.map { |e| [e, grouped_textures[e].size] }.to_h
          if groups.size > 1
            groups.delete_if { |e| group_sizes[e] == 1 }
          end

          groups = groups.sort_by { |e| e.length }
          tex_info[:group] = groups.last unless groups.empty?
        end
      end

      textures.each do |tex_info|
        next unless tex_info[:group]
        next if tex_info[:manual]
        next if tex_info[:action] != :combine

        tex_info[:target_name] = "#{tex_info[:group]}_#{tex_info[:target_name]}"
      end

      if need_process
        metadata[:dir] = src_dir
        metadata[:textures] = textures.sort_by { |e| e[:name].downcase }

        Util.write_metadata(src_dir:, data: metadata, dry_run:)
      end

      if recursive
        sub_dirs.each do |sub_dir|
          generate_metadata(
            src_dir: [src_dir, sub_dir].join('/'))
        end
      end
    end

    def detect_type(f)
      basename = File.basename(f, ".*").downcase

      REG_EX.each_with_index do |level, idx|
        level.each do |type, expressions|
          return type if expressions.any? { |e| e.match?(basename) }
        end
      end

      :unknown
    end

    def group_by_prefix(textures)
      groups = {}

      textures.each do |tex_info|
        #next unless tex_info[:action] == :combine

        parts = tex_info[:name].split('_')
        parts.size.times do |idx|
          group = parts[0, idx + 1].join('_')
          next if group == tex_info[:name]

          type = detect_type(group)
          next if type != :unknown

          (groups[group] ||= []) << tex_info
        end
      end

      groups
    end
  end
end
