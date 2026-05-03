# Maple Combat Calculator Wrapper Makefile

.PHONY: all build wasm clean test rebuild help

BUILD_DIR := build
WASM_BUILD_DIR := build-wasm

# 환경 변수에서 읽어오거나 기본값 설정
EMSDK_DIR ?= $(HOME)/emsdk

# vcpkg 경로 탐색: 1. 프로젝트 내부, 2. 시스템 환경변수 (CI 등)
ifeq ($(wildcard $(shell pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake),)
    VCPKG_ROOT ?= $(VCPKG_INSTALLATION_ROOT)
else
    VCPKG_ROOT ?= $(shell pwd)/vcpkg
endif

VCPKG_TOOLCHAIN := $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake

# 병렬 빌드 설정
# 사용자가 직접 JOBS=n 으로 지정 가능, 미지정 시 CMake의 --parallel 기본값 사용
ifdef JOBS
    PARALLEL_FLAGS := --parallel $(JOBS)
else
    PARALLEL_FLAGS := --parallel
endif

# 기본 타겟
all: build

# 프로젝트 빌드 (Native)
build:
	@echo "--- Configuring and Building Project (Native) ---"
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN)
	@cmake --build $(BUILD_DIR) $(PARALLEL_FLAGS)

# WebAssembly 빌드
# emsdk_env.sh는 반드시 해당 디렉토리에서 source 되어야 정상 작동하는 경우가 많음
wasm:
	@echo "--- Configuring and Building Project (WebAssembly) ---"
	@if [ ! -d "$(EMSDK_DIR)" ]; then \
		echo "Error: Emscripten SDK directory not found at $(EMSDK_DIR)."; \
		exit 1; \
	fi
	@mkdir -p $(WASM_BUILD_DIR)
	@cd $(EMSDK_DIR) && . ./emsdk_env.sh && \
	cd $(shell pwd) && \
	cmake -B $(WASM_BUILD_DIR) \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) \
		-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$(EMSDK_DIR)/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
		-DVCPKG_TARGET_TRIPLET=wasm32-emscripten \
		-DCMAKE_BUILD_TYPE=Release \
		-DVCPKG_INSTALLED_DIR=$(WASM_BUILD_DIR)/vcpkg_installed && \
	cmake --build $(WASM_BUILD_DIR) $(PARALLEL_FLAGS)

# 빌드 결과물 및 폴더 삭제
clean:
	@echo "--- Cleaning Build Artifacts ---"
	@rm -rf $(BUILD_DIR) $(WASM_BUILD_DIR)
	@echo "Build directories removed."

# 테스트 실행 (Native)
test:
	@echo "--- Running Tests (Native) ---"
	@if [ -d "$(BUILD_DIR)" ]; then \
		cd $(BUILD_DIR) && ctest --output-on-failure; \
	else \
		echo "Build directory not found. Please run 'make build' first."; \
	fi

# 완전히 새로 빌드
rebuild: clean build

# 도움말
help:
	@echo "Available commands:"
	@echo "  make         : Build the project (Native Release mode)"
	@echo "  make build   : Build the project (Native Release mode)"
	@echo "  make wasm    : Build the project (WebAssembly mode)"
	@echo "  make clean   : Remove all build directories"
	@echo "  make test    : Run native tests"
	@echo "  make rebuild : Clean and then build native"
	@echo "  make help    : Show this message"
