# Maple Combat Calculator Wrapper Makefile

.PHONY: all build clean test rebuild help

BUILD_DIR := build
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=OFF

# 기본 타겟: 빌드 수행
all: build

# 프로젝트 빌드
build:
	@echo "--- Configuring and Building Project ---"
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS)
	@cmake --build $(BUILD_DIR) -j$$(nproc)

# 빌드 결과물 및 폴더 삭제
clean:
	@echo "--- Cleaning Build Artifacts ---"
	@if [ -d "$(BUILD_DIR)" ]; then \
		cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true; \
		rm -rf $(BUILD_DIR); \
		echo "Build directory removed."; \
	else \
		echo "Nothing to clean."; \
	fi

# 테스트 실행
test:
	@echo "--- Running Tests ---"
	@if [ -d "$(BUILD_DIR)" ]; then \
		cd $(BUILD_DIR) && ctest --output-on-failure; \
	else \
		echo "Build directory not found. Please run 'make' first."; \
	fi

# 완전히 새로 빌드
rebuild: clean build

# 도움말
help:
	@echo "Available commands:"
	@echo "  make         : Build the project (Release mode, System libs)"
	@echo "  make clean   : Remove build directory"
	@echo "  make test    : Run all tests"
	@echo "  make rebuild : Clean and then build"
	@echo "  make help    : Show this message"
