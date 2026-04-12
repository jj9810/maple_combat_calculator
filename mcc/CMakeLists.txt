cmake_minimum_required(VERSION 3.10)
project(mcc)

# C++ 20 버전 사용 권장
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Protobuf 라이브러리 찾기 (mcc_stat.pb.h 생성 필요)
find_package(Protobuf REQUIRED)

# shared/proto/mcc_stat.proto를 사용하여 C++ 소스 생성
set(MCC_PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto/mcc_stat.proto")
protobuf_generate_cpp(MCC_PROTO_SRCS MCC_PROTO_HDRS ${MCC_PROTO_FILE})

# MCC 라이브러리 정의
add_library(mcc STATIC
    inc/combat_power.h
    inc/skill_damage.h
    src/combat_power.cpp
    src/skill_damage.cpp
    ${MCC_PROTO_SRCS}
    ${MCC_PROTO_HDRS}
)

# 인클루드 경로 설정 (PUBLIC으로 설정하여 의존하는 프로젝트도 참조 가능하게 함)
target_include_directories(mcc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_BINARY_DIR}
)

# Protobuf 라이브러리 연결
target_link_libraries(mcc PRIVATE ${Protobuf_LIBRARIES})
