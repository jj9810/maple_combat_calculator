cmake_minimum_required(VERSION 3.10)
project(mcc)

# Protobuf 라이브러리 찾기
find_package(Protobuf REQUIRED)

# 프로토 파일 생성 설정
get_filename_component(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto" ABSOLUTE)
set(MCC_PROTO_FILE "${PROTO_DIR}/internal/mcc_stat.proto")

set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}/internal")

set(MCC_PROTO_SRCS "${PROTO_GEN_DIR}/internal/mcc_stat.pb.cc")
set(MCC_PROTO_HDRS "${PROTO_GEN_DIR}/internal/mcc_stat.pb.h")

add_custom_command(
    OUTPUT "${MCC_PROTO_SRCS}" "${MCC_PROTO_HDRS}"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS "--cpp_out=${PROTO_GEN_DIR}" "-I${PROTO_DIR}" "${MCC_PROTO_FILE}"
    DEPENDS "${MCC_PROTO_FILE}"
    COMMENT "Running C++ protocol buffer compiler on ${MCC_PROTO_FILE}"
    VERBATIM
)

# MCC 라이브러리 정의
add_library(mcc STATIC
    inc/combat_power.h
    inc/skill_damage.h
    src/combat_power.cpp
    src/skill_damage.cpp
    ${MCC_PROTO_SRCS}
    ${MCC_PROTO_HDRS}
)

# 인클루드 경로 설정
target_include_directories(mcc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Protobuf 라이브러리 연결
target_link_libraries(mcc PRIVATE protobuf::libprotobuf)
