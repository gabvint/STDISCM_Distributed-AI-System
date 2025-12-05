find_package(Tesseract CONFIG REQUIRED)

add_executable(server_ping
    src/main.cpp
    generated/ocr.pb.cc
    generated/ocr.grpc.pb.cc
    src/tesseract_wrapper.cpp
    src/thread_pool.cpp 
)

target_include_directories(server_ping PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_link_libraries(server_ping PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
    Tesseract::libtesseract
)
