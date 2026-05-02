cmake_minimum_required(VERSION 3.10)
project(nexon_api)

# 라이브러리 찾기 (Protobuf & CURL)
find_package(Protobuf REQUIRED)
if(NOT TARGET CURL::libcurl)
    find_package(CURL REQUIRED)
endif()

# 프로토 파일 생성 설정
get_filename_component(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto" ABSOLUTE)
set(PROTO_FILES
    "${PROTO_DIR}/nexon/battle_practice_character_info.proto"
    "${PROTO_DIR}/nexon/battle_practice_result.proto"
    "${PROTO_DIR}/nexon/error_message.proto"
    "${PROTO_DIR}/nexon/replay_id.proto"
    "${PROTO_DIR}/nexon/skill_timeline.proto"
    "${PROTO_DIR}/internal/combat_log.proto"
    "${PROTO_DIR}/internal/mcc_stat.proto"
)

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

# nexon_api 라이브러리 정의
add_library(nexon_api STATIC
    inc/nexon_api_loader.h
    src/nexon_api_loader.cpp
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# nexon_api 라이브러리에 include 경로 추가
target_include_directories(nexon_api PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# nexon_api 라이브러리가 의존하는 라이브러리 연결
target_link_libraries(nexon_api PUBLIC CURL::libcurl protobuf::libprotobuf)

# 테스트 실행 파일 정의
add_executable(nexon_api_test test/nexon_api_loader_test.cpp)
target_link_libraries(nexon_api_test PRIVATE nexon_api)
add_test(NAME nexon_api_test COMMAND nexon_api_test)
