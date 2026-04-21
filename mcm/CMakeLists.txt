cmake_minimum_required(VERSION 3.10)
project(mcm)

# Protobuf 찾기
find_package(Protobuf REQUIRED)

# shared/proto 디렉토리의 .proto 파일 목록
get_filename_component(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto" ABSOLUTE)
set(PROTO_FILES
    "${PROTO_DIR}/internal/combat_log.proto"
    "${PROTO_DIR}/nexon/battle_practice_character_info.proto"
    "${PROTO_DIR}/nexon/battle_practice_result.proto"
    "${PROTO_DIR}/nexon/error_message.proto"
    "${PROTO_DIR}/nexon/replay_id.proto"
    "${PROTO_DIR}/nexon/skill_timeline.proto"
)

# .proto 파일로부터 C++ 소스 생성
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}/internal" "${PROTO_GEN_DIR}/nexon")

set(PROTO_SRCS)
set(PROTO_HDRS)
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WLE)
    get_filename_component(PROTO_REL_DIR ${PROTO_FILE} DIRECTORY)
    file(RELATIVE_PATH PROTO_REL_PATH "${PROTO_DIR}" "${PROTO_REL_DIR}")
    
    set(GEN_SRC "${PROTO_GEN_DIR}/${PROTO_REL_PATH}/${PROTO_NAME}.pb.cc")
    set(GEN_HDR "${PROTO_GEN_DIR}/${PROTO_REL_PATH}/${PROTO_NAME}.pb.h")
    
    add_custom_command(
        OUTPUT "${GEN_SRC}" "${GEN_HDR}"
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS "--cpp_out=${PROTO_GEN_DIR}" "-I${PROTO_DIR}" "${PROTO_FILE}"
        DEPENDS "${PROTO_FILE}"
        COMMENT "Running C++ protocol buffer compiler on ${PROTO_FILE}"
        VERBATIM
    )
    list(APPEND PROTO_SRCS "${GEN_SRC}")
    list(APPEND PROTO_HDRS "${GEN_HDR}")
endforeach()

# MCM 라이브러리 정의
add_library(mcm STATIC
    inc/mcm.h
    inc/context.h
    inc/aggregator.h
    inc/loader.h
    inc/connector.h
    src/mcm.cpp
    src/context.cpp
    src/aggregator.cpp
    src/loader.cpp
    src/connector.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# MCM 라이브러리에 inc 경로 추가
target_include_directories(mcm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/mcc> # mcc의 헤더 위치 (루트 빌드 기준)
)

# MCM 라이브러리가 의존하는 라이브러리 연결
target_link_libraries(mcm PUBLIC mcc protobuf::libprotobuf yaml-cpp::yaml-cpp)

# MCM 실행 파일 정의
add_executable(mcm_runner src/main.cpp)
target_link_libraries(mcm_runner PRIVATE mcm)
