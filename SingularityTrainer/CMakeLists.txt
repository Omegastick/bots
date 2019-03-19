cmake_minimum_required(VERSION 3.5.1)
project(SingularityTrainer)

# CMake policies
if (UNIX)
    cmake_policy(SET CMP0077 NEW)
endif()

# Configuration
set(LIB_DIR lib)
set(GLFW_DIR ${LIB_DIR}/GLFW)
set(GLAD_DIR ${LIB_DIR}/GLAD)
set(GLM_DIR ${LIB_DIR}/GLM)
set(SPDLOG_DIR ${LIB_DIR}/spdlog)
set(IMGUI_DIR ${LIB_DIR}/imgui)
set(ZMQ_DIR ${LIB_DIR}/libzmq)
set(CPPZMQ_DIR ${LIB_DIR}/cppzmq)
set(BOX2D_DIR ${LIB_DIR}/Box2D-cmake)
set(MSGPACK_DIR ${LIB_DIR}/msgpack-c)
set(DOCTEST_DIR ${LIB_DIR}/doctest)
set(ARGH_DIR ${LIB_DIR}/argh)
set(JSON_DIR ${LIB_DIR}/json)

# CMake Properties
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE -O3)

# Executables and loca libraries
add_executable(SingularityTrainer "")
add_executable(HeadlessTrainer "")
add_executable(GraphicsPlayground "")
add_library(Training STATIC "")
add_library(Graphics STATIC "")

# Libraries included in repository
## GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
target_compile_definitions(SingularityTrainer PRIVATE "GLFW_INCLUDE_NONE")
target_compile_definitions(GraphicsPlayground PRIVATE "GLFW_INCLUDE_NONE")
add_subdirectory(${GLFW_DIR})
## GLAD
add_library(glad ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)
## GLM
set(GLM_TEST_ENABLE OFF)
add_subdirectory(${GLM_DIR})
## spdlog
add_subdirectory(${SPDLOG_DIR})
## imgui
add_subdirectory(${IMGUI_DIR})
## ZMQ
set(ZMQ_BUILD_TESTS OFF)
add_subdirectory(${ZMQ_DIR})
## Box2D
add_subdirectory(${BOX2D_DIR})
## Doctest
add_subdirectory(${DOCTEST_DIR})
## Json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${JSON_DIR})

# Link libraries
target_link_libraries(Training libzmq-static TG::Box2D doctest)
# UNIX systems sometimes need to explicitly link the threading library and filesystem library
if (UNIX)
    target_link_libraries(Training pthread)
    target_link_libraries(SingularityTrainer stdc++fs)
endif()
target_link_libraries(Graphics glad glfw ${GLFW_LIBRARIES} glm imgui doctest)
target_link_libraries(SingularityTrainer Training Graphics nlohmann_json::nlohmann_json)
target_link_libraries(HeadlessTrainer Training Graphics)
target_link_libraries(GraphicsPlayground Training Graphics)

# Inlcudes
set(INCLUDE_DIRS
    src
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${GLM_DIR} 
    ${SPDLOG_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${BOX2D_DIR}/Box2D
    ${MSGPACK_DIR}/include
    ${CPPZMQ_DIR}
    ${ZMQ_DIR}/include
    ${DOCTEST_DIR}/doctest
    ${ARGH_DIR}
    ${JSON_DIR}/include
)
target_include_directories(SingularityTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(HeadlessTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(GraphicsPlayground PUBLIC ${INCLUDE_DIRS})
target_include_directories(Training PUBLIC ${INCLUDE_DIRS})
target_include_directories(Graphics PUBLIC ${INCLUDE_DIRS})

add_subdirectory(src)
