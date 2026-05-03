cmake_minimum_required(VERSION 3.15)

# --- vcpkg 툴체인 연동 (선택 사항) ---
# 사용자가 외부에서 CMAKE_TOOLCHAIN_FILE을 지정하지 않았고, 프로젝트 내 vcpkg가 존재할 경우에만 설정
if(NOT CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "Found vcpkg in project, using it as default toolchain.")
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    endif()
endif()

project(maple_combat_calculator)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 의존성 로드 ---
find_package(Protobuf REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(boost_multiprecision CONFIG REQUIRED)
if(NOT EMSCRIPTEN)
    find_package(CURL REQUIRED)
endif()

# yaml-cpp 타겟 호환성 처리 (시스템 패키지에 따라 yaml-cpp 또는 yaml-cpp::yaml-cpp 일 수 있음)
if(NOT TARGET yaml-cpp::yaml-cpp AND TARGET yaml-cpp)
    add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED)
    set_target_properties(yaml-cpp::yaml-cpp PROPERTIES INTERFACE_LINK_LIBRARIES yaml-cpp)
endif()

# 하위 컴포넌트 추가
add_subdirectory(mcc)
add_subdirectory(mcm)
if(NOT EMSCRIPTEN)
    add_subdirectory(nexon_api)
endif()

# --- WebAssembly 전용 설정 ---
if(EMSCRIPTEN)
    message(STATUS "WebAssembly build detected. Adding Emscripten flags.")
    # 모든 타겟에 공통으로 적용할 Emscripten 플래그
    add_link_options(
        "-s WASM=1"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s NO_EXIT_RUNTIME=1"
    )
endif()
