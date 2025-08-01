add_executable(Server Server.cpp)
target_link_libraries(Server PRIVATE Common Ws2_32)