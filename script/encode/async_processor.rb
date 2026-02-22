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
  class AsyncProcessor
    class Worker
      attr_reader :tid

      def initialize(
        processor,
        tid
      )
        @processor = processor
        @tid = tid
      end

      def info(msg)
        puts "TID[#{@tid}]: #{msg}"
      end

      def process_files
        while @processor.alive?
          json = @processor.poll_task
          break if json == :shutdown_worker

          task = JSON.parse(json, symbolize_names: true)

          cls = Object.const_get(task[:class])
          args = task[:args]

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

          encoder = cls.new(**args)

          encoder.encode(tid:)
        end
      ensure
        @processor.worker_stopped
      end
    end

    def initialize(
      thread_count:
    )
      @thread_count = thread_count
      @mutex = Mutex.new
      @queue = Queue.new
      @workers = []
      @running = false
      @remaining = 0
    end

    def add_task(**task)
      @queue << task.to_json
    end

    def poll_task
      @queue.pop
    end

    def size
      @queue.size
    end

    def alive?
      @mutex.synchronize {
        @running
      }
    end

    def worker_stopped
      @mutex.synchronize {
        @remaining -= 1
      }
    end

    def remaining
      @mutex.synchronize {
        @remaining
      }
    end

    def start
      @running = true
      @thread_count.times.each do |idx|
        @mutex.synchronize {
          @remaining += 1
        }

        @workers << Thread.new do
          worker = Worker.new(self, idx)
          worker.process_files
        end
      end
    end

    def shutdown
      @running = false

      @thread_count.times.each do |idx|
        @queue << :shutdown_worker
      end

      wait_remaining = 1
      while wait_remaining > 0
        wait_remaining = remaining
        sleep 0.1 if wait_remaining > 0
      end
      @workers.clear
    end

    def wait
      while @running
        sleep 0.2
        return if @queue.empty?
      end
    end
  end
end
