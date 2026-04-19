cmake_minimum_required(VERSION 3.10)
project(mdc)

set(CMAKE_CXX_STANDARD 20)

# Protobuf 라이브러리 찾기
find_package(Protobuf REQUIRED)

# yaml-cpp 라이브러리 찾기
find_package(yaml-cpp REQUIRED)

# shared/proto 디렉토리의 .proto 파일 목록 (서브디렉토리 포함)
file(GLOB_RECURSE PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto/*.proto")

# .proto 파일로부터 C++ 소스 생성
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# MDC 라이브러리 정의
add_library(mdc STATIC
    inc/engine.h
    inc/component.h
    inc/entity.h
    inc/policy.h
    inc/logger.h
    inc/loader.h
    src/engine.cpp
    src/entity.cpp
    src/policy.cpp
    src/logger.cpp
    src/loader.cpp
    # component.cpp는 인터페이스만 있으므로 제외
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# MDC 라이브러리에 include 경로 추가
target_include_directories(mdc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_BINARY_DIR} # 생성된 protobuf 헤더를 찾기 위해
)

# MDC 라이브러리가 의존하는 라이브러리 연결
target_link_libraries(mdc PRIVATE ${Protobuf_LIBRARIES} yaml-cpp)

# MDC 실행 파일 정의
add_executable(mdc_runner src/main.cpp)

# MDC 실행 파일에 라이브러리 연결
target_link_libraries(mdc_runner PRIVATE mdc)
