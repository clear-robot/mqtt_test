cmake_minimum_required(VERSION 3.22)
project(mqtt)

set(CMAKE_CXX_STANDARD 14)


add_executable(send send.cpp)
target_link_libraries(send paho-mqttpp3 paho-mqtt3as)

add_executable(receive receive.cpp)
target_link_libraries(receive paho-mqttpp3 paho-mqtt3as)
