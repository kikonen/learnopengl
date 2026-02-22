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

require_relative "util"

module Encode
  class AsyncWorker
   def initialize
   end

   def run
   end
  end
end

AsyncWorker.new.run
