# frozen_string_literal: true

require 'open3'
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
      EXIT = "__shutdown__"
      DONE = "__done__"

      attr_reader :tid

      def initialize(
        processor,
        tid
      )
        @processor = processor
        @tid = tid
        @wait_queue = Queue.new
      end

      def info(msg)
        puts "TID[#{@tid}]: #{msg}"
      end

      def create
        env = {}
        env["RUBY_DEBUG_PORT"] = "21#{@tid.to_s.rjust(3, '0')}" if @tid == 0

        stdin_r, stdin_w = IO.pipe
        stdout_r, stdout_w = IO.pipe

        pid = Process.spawn(
          env,
          RbConfig.ruby,
          "script/encode/async_worker.rb",
          in: stdin_r,
          out: stdout_w,
          err: :err,
          close_others: true
        )
        stdin_r.close
        stdout_w.close

        thread = Thread.new do
          Process.waitpid(pid)
        rescue => e
          stacktrace = e.backtrace.join("\n")
          puts "ERROR: #{e.message}\n#{stacktrace}"
        end

        @pipe = [stdin_w, stdout_r, thread]
      end

      def start
        @task_id_seq = 0
        consume(@pipe[0], @pipe[1], @pipe[2])
      end

      def consume(stdin_w, stdout_w, thread)
        Thread.new do
          reader = Thread.new do
            while (line = stdout_w.gets)
              #handle_response(JSON.parse(line, symbolize_names: true))
              handle_response(line)
            end
          end

          process_tasks(stdin_w)
          stdin_w.close

          reader.join
          thread.join
        end
      end

      def handle_response(line)
        response = JSON.parse(line, symbolize_names: true)
        if response[:response] == DONE
          @wait_queue << response
        end
      rescue
        puts line
      end

      def process_tasks(stdin_w)
        while @processor.alive?
          json = @processor.poll_task

          if json == :shutdown_worker
            stdin_w.puts(JSON.generate({ action: EXIT, tid: tid }))
          else
            #puts json
            task = JSON.parse(json, symbolize_names: true)

            @task_id_seq += 1
            task_id = "#{tid}-#{@task_id_seq}"

            task[:tid] = tid
            task[:task_id] = task_id

            stdin_w.puts(task.to_json)

            response = @wait_queue.pop
            if response[:task_id] != task_id
              raise "MISMATCH: #{response}"
            end
          end
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
      #puts "queue: #{@queue.size}"
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
        worker = Worker.new(self, idx)
        worker.create
        @workers << worker
      end

      @workers.each do |worker|
        worker.start
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
