add_library(Common STATIC GameProtocol.h Network.cpp)
target_include_directories(Common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Common PUBLIC Ws2_32)