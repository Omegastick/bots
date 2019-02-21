cmake_minimum_required(VERSION 3.5.1)
project(SingularityTrainer)

# CMake policies
cmake_policy(SET CMP0077 NEW)

# Configuration
set(LIB_DIR lib)
set(GLFW_DIR ${LIB_DIR}/GLFW)
set(GLAD_DIR ${LIB_DIR}/GLAD)
set(GLM_DIR ${LIB_DIR}/GLM)
set(SPDLOG_DIR ${LIB_DIR}/spdlog)
set(IMGUI_DIR ${LIB_DIR}/imgui)

# CMake Properties
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE -O3)

# Executable
add_executable(SingularityTrainer "")
add_executable(GraphicsPlayground "")

# Libraries
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

# Libraries
find_package(SFML 2.5 REQUIRED audio graphics window system)
find_package(cppzmq)
find_package(Box2D)
target_link_libraries(SingularityTrainer glad glfw ${GLFW_LIBRARIES} glm imgui cppzmq Box2D)
if (UNIX)
    # UNIX systems sometimes need to explicitly link the threading library
    target_link_libraries(SingularityTrainer pthread)
endif()
target_link_libraries(GraphicsPlayground glad glfw ${GLFW_LIBRARIES} glm imgui Box2D)

# Inlcudes
target_include_directories(SingularityTrainer PUBLIC 
    src
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${GLM_DIR} 
    ${SPDLOG_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
)
target_include_directories(GraphicsPlayground 
PUBLIC 
    src 
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${GLM_DIR} 
    ${SPDLOG_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
)

add_subdirectory(src)