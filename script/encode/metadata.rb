# frozen_string_literal: true

require 'hashie'

require_relative "texture_info"

module Encode
  #
  # Meta
  #
  class Metadata < Hashie::Dash
    include Hashie::Extensions::Coercion
    include Hashie::Extensions::MergeInitializer
    include Hashie::Extensions::MethodAccess

    coerce_key :textures, Array[TextureInfo]

    property :dir
    property :textures

    def self.new_empty
      self.new(textures: [])
    end
  end
end
