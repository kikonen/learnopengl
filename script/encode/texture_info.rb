# frozen_string_literal: true

require 'hashie'

module Encode
  #
  # NOTE KI using Hashie
  #
  class TextureInfo < Hashie::Dash
    include Hashie::Extensions::MergeInitializer
    include Hashie::Extensions::MethodAccess
    include Hashie::Extensions::Dash::PropertyTranslation

    property :name
    property :target_name
    property :group
    property :type
    property :action
    property :mode
    property :detected_channels
    property :source_channel
    property :target_channel
    property :source_invert
    property :target_invert
    property :source_depth
    property :target_depth
    property :source_size
    property :srgb
    property :manual

    def normalize!
      self[:type] = self.type.to_sym if self.type
      self[:action] = self.action.to_sym if self.action
      self[:mode] = self.mode.to_sym if self.mode
      self
    end

    def src_path(src_dir)
      "#{src_dir}/#{self.name}"
    end

    def encode_srgb?
      diffuse?
    end

    def diffuse?
      self.type == :diffuse || self.type == :emission
    end
  end
end
