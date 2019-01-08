cmake_minimum_required(VERSION 3.5.1)
project(SingularityTrainer)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# Libraries
# SFML
find_package(SFML 2.5 REQUIRED audio graphics window system)
# 0MQ
find_package(cppzmq)
# Box2D
find_package(Box2D)

# Includes
include_directories(include)

# Executable
file(GLOB SOURCES "src/**.cpp")
add_executable(SingularityTrainer ${SOURCES})

target_link_libraries(SingularityTrainer sfml-graphics sfml-window sfml-system cppzmq Box2D)