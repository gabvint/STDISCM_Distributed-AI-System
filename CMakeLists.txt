cmake_minimum_required(VERSION 3.15)
project(STDISCM_DistributedAISystem CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where Homebrew installs packages (Mac); Windows can ignore if using vcpkg.
# You can keep both paths; only the existing one will matter.
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_subdirectory(client)
add_subdirectory(server)
