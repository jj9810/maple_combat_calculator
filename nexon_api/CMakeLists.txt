cmake_minimum_required(VERSION 3.10)
project(nexon_api)

set(CMAKE_CXX_STANDARD 20)

# 라이브러리 찾기
find_package(Protobuf REQUIRED)
find_package(CURL REQUIRED)

# nexon_api 라이브러리 정의
add_library(nexon_api STATIC
    inc/nexon_api_loader.h
    src/nexon_api_loader.cpp
)

# nexon_api 라이브러리에 include 경로 추가
target_include_directories(nexon_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../ # shared_proto 헤더를 찾기 위해
)

# nexon_api 라이브러리가 의존하는 라이브러리 연결
target_link_libraries(nexon_api PUBLIC shared_proto CURL::libcurl ${Protobuf_LIBRARIES})

# 테스트 실행 파일 정의
add_executable(nexon_api_test test/nexon_api_loader_test.cpp)
target_link_libraries(nexon_api_test PRIVATE nexon_api)
