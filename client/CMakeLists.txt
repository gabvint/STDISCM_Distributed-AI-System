cmake_minimum_required(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

set(CLIENT_GUI_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/grpc_client.cpp
    generated/ocr.pb.cc
    generated/ocr.grpc.pb.cc
)

set(CLIENT_GUI_HEADERS
    src/mainwindow.h
    src/grpc_client.h
)

qt_add_executable(client_gui
    ${CLIENT_GUI_SOURCES}
    ${CLIENT_GUI_HEADERS}
)

target_include_directories(client_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(client_gui PRIVATE
    Qt6::Widgets
    Qt6::Concurrent
    gRPC::grpc++
    protobuf::libprotobuf
)
