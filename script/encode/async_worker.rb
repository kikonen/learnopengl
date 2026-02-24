# frozen_string_literal: true

require "debug"
require 'debug/open_nonstop'

require 'json'
require 'logger'

require_relative "util"

require_relative "texture_info"
require_relative "image_encoder"
require_relative "combine_encoder"
require_relative "ktx_encoder"

$stdout.sync = true

module Encode
  class AsyncWorker
    EXIT = "__shutdown__"
    DONE = "__done__"

    attr_reader :tid

    def initialize
      @tid = -1
    end

    def info(msg)
      # data = { message: "TID[#{@tid}]: #{msg}" }
      # puts data.to_json
      puts "TID[#{@tid}]: #{msg}"
    end

    def run
      while task = read_task
        break unless task
        break if task[:action] == EXIT

        @tid = task[:tid]
        task_id = task[:task_id]

        cls = Object.const_get(task[:class])
        args = task[:args] || {}

        encoder = cls.new(**args)
        encoder.encode(tid:)

        response = { response: DONE, task_id: task_id }
        puts response.to_json
      end
    rescue => e
      stacktrace = e.backtrace.join("\n")
      info "ERROR: #{e.message}\n#{stacktrace}"
    end

    def read_task
      line = $stdin.gets

      return unless line

      task = JSON.parse(line, symbolize_names: true)
      args = task[:args] || {}

      if args[:type]
        args[:type] = args[:type].to_sym
      end

      if args[:target_mode]
        args[:target_mode] = args[:target_mode].to_sym
      end

      if args[:target_type]
        args[:target_type] = args[:target_type].to_sym
      end

      if args[:parts]
        args[:parts] = args[:parts].map { |e| TextureInfo.new(e) }
      end

      if args[:tex_info]
        args[:tex_info] = TextureInfo.new(args[:tex_info])
      end

      task
    end
  end
end

Encode::AsyncWorker.new.run
