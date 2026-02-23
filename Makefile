TARGET_SIZE ?= 1024
MSBUILD ?= MSBuild.exe
PROJECT = learnopengl.vcxproj

default: help

help:
	@echo "Build (from MSYS2):"
	@echo "  make compile-debug"
	@echo "  make compile-release"
	@echo "  make compile-build"
	@echo "  make compile          (debug + release)"
	@echo ""
	@echo "Run:"
	@echo "  make run-debug"
	@echo "  make run-release"
	@echo "  make run-build"
	@echo ""
	@echo "Assets:"
	@echo "  make setup"
	@echo "  make all TARGET_SIZE=${TARGET_SIZE}"
	@echo "    OR"
	@echo "  make assets-meta TARGET_SIZE=${TARGET_SIZE}"
	@echo "  make assets-build TARGET_SIZE=${TARGET_SIZE}"

# Build targets (MSYS2)
compile: compile-debug compile-release

compile-debug:
	cmd //c "${MSBUILD} ${PROJECT} /p:Configuration=Debug /p:Platform=x64 /m"

compile-release:
	cmd //c "${MSBUILD} ${PROJECT} /p:Configuration=Release /p:Platform=x64 /m"

compile-build:
	cmd //c "${MSBUILD} ${PROJECT} /p:Configuration=Build /p:Platform=x64 /m"

# Run targets
run-debug:
	./x64/Debug/learnopengl.exe

run-release:
	./x64/Release/learnopengl.exe

run-build:
	./x64/Build/learnopengl.exe

all: assets-meta assets-build

setup:
	git submodule init
	git submodule update --init  --recursive
	ruby --version
	bundle install

assets: assets-meta assets-build

assets-meta:
	ruby script/encode_ktx.rb meta --src resources/assets --dry-run false --recursive true --target-size ${TARGET_SIZE}

assets-build:
	ruby script/encode_ktx.rb build --src resources/assets --dry-run false --recursive true --target-size ${TARGET_SIZE} --encode --combine
