cmake_minimum_required(VERSION 3.5.1)
project(SingularityTrainer)

# Properties
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

# Executable
add_executable(SingularityTrainer "")

# Libraries
find_package(SFML 2.5 REQUIRED audio graphics window system)
find_package(cppzmq)
find_package(Box2D)
target_link_libraries(SingularityTrainer sfml-graphics sfml-window sfml-system cppzmq Box2D thor)

# Inlcudes
target_include_directories(SingularityTrainer PUBLIC include)

add_subdirectory(src)