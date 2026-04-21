cmake_minimum_required(VERSION 3.15)
project(maple_combat_calculator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- vcpkg 툴체인 연동 (선택 사항) ---
# 사용자가 외부에서 CMAKE_TOOLCHAIN_FILE을 지정하지 않았고, 프로젝트 내 vcpkg가 존재할 경우에만 설정
if(NOT CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Found vcpkg in project, using it as default toolchain.")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# --- 의존성 로드 ---
find_package(Protobuf REQUIRED)
find_package(CURL REQUIRED)
find_package(yaml-cpp REQUIRED)

# yaml-cpp 타겟 호환성 처리 (시스템 패키지에 따라 yaml-cpp 또는 yaml-cpp::yaml-cpp 일 수 있음)
if(NOT TARGET yaml-cpp::yaml-cpp AND TARGET yaml-cpp)
    add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED)
    set_target_properties(yaml-cpp::yaml-cpp PROPERTIES INTERFACE_LINK_LIBRARIES yaml-cpp)
endif()

# 하위 컴포넌트 추가
add_subdirectory(mcc)
add_subdirectory(mcm)
add_subdirectory(nexon_api)
