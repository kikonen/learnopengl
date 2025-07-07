TARGET_SIZE ?= 1024

default: help

help:
	@echo "make setup"
	@echo "---------------"
	@echo "make all TARGET_SIZE=${TARGET_SIZE}"
	@echo "  OR"
	@echo "make meta TARGET_SIZE=${TARGET_SIZE}"
	@echo "make build TARGET_SIZE=${TARGET_SIZE}"

all: meta build

setup:
	git submodule init
	git submodule update --init  --recursive
	ruby --version
	bundle install

meta:
	ruby script/encode_ktx.rb meta --src resources/assets --dry-run false --recursive true --target-size ${TARGET_SIZE}

build:
	ruby script/encode_ktx.rb build --src resources/assets --dry-run false --recursive true --target-size ${TARGET_SIZE} --encode --combine
