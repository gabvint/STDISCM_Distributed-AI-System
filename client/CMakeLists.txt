cmake_minimum_required(VERSION 3.15)

add_executable(client_ping
    src/main.cpp
    generated/ocr.pb.cc
    generated/ocr.grpc.pb.cc
)

target_include_directories(client_ping PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_link_libraries(client_ping PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
)
