TEX_SIZE ?= 1024
TEX_DEPTH ?= 8
MSBUILD ?= MSBuild.exe
PROJECT = engine.vcxproj

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
	@echo "  make all TEX_SIZE=${TEX_SIZE}"
	@echo "    OR"
	@echo "  make assets-meta TEX_SIZE=${TEX_SIZE}"
	@echo "  make assets-build TEX_SIZE=${TEX_SIZE}"

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
	./x64/Debug/sample_app.exe

run-release:
	./x64/Release/sasmple_app.exe

run-build:
	./x64/Build/sample_app.exe

all: assets-meta assets-build

setup:
	git submodule init
	git submodule update --init  --recursive
	ruby --version
	bundle install

assets: assets-meta assets-build

assets-meta:
	ruby script/convert_tex.rb meta --src resources/assets --dry-run false --recursive true --target-size ${TEX_SIZE} --target-depth ${TEX_DEPTH}

assets-build:
	ruby script/convert_tex.rb build --src resources/assets --dry-run false --recursive true --target-size ${TEX_SIZE} --target-depth ${TEX_DEPTH} --encode --combine

assets-ktx:
	ruby script/convert_tex.rb build --src resources/assets --dry-run false --recursive true --target-size ${TEX_SIZE} --target-depth ${TEX_DEPTH} --ktx
