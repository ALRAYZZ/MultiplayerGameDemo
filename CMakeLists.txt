cmake_minimum_required(VERSION 3.15)
project(MultiplayerGameDemo LANGUAGES CXX)


# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subtirectories
add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/server)