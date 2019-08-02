cmake_minimum_required(VERSION 3.5.1)
project(SingularityTrainer)

# CMake policies
if (UNIX)
    cmake_policy(SET CMP0077 NEW)
endif()

# Configuration
set(LIB_DIR lib)
set(ARGH_DIR ${LIB_DIR}/argh)
set(BOX2D_DIR ${LIB_DIR}/Box2D-cmake)
set(CPPRL_DIR ${LIB_DIR}/pytorch-cpp-rl)
set(DOCTEST_DIR ${LIB_DIR}/doctest)
set(FMT_DIR ${LIB_DIR}/fmt)
set(GLFW_DIR ${LIB_DIR}/GLFW)
set(GLAD_DIR ${LIB_DIR}/GLAD)
set(GLM_DIR ${LIB_DIR}/GLM)
set(IMGUI_DIR ${LIB_DIR}/imgui)
set(JSON_DIR ${LIB_DIR}/json)
set(MSGPACK_DIR ${LIB_DIR}/msgpack-c)
set(PYBIND_DIR ${LIB_DIR}/pybind11)
set(SPDLOG_DIR ${LIB_DIR}/spdlog)
set(TASKFLOW_DIR ${LIB_DIR}/cpp-taskflow)
set(TWEENY_DIR ${LIB_DIR}/tweeny)
set(ZMQ_DIR ${LIB_DIR}/libzmq)

# CMake Properties
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE -O3)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO -O3)

# Executables and local libraries
add_library(Graphics OBJECT "")
add_library(Misc OBJECT "")
add_library(Training OBJECT "")
add_executable(SingularityTrainer "")
add_executable(HeadlessTrainer "")
add_executable(GraphicsPlayground "")
add_executable(Server "")
set(ST_TARGETS
    SingularityTrainer
    HeadlessTrainer
    GraphicsPlayground
    Server
    Graphics
    Misc
    Training
)

# Disable HeadlessTrainer and GraphicsPlayground by default
set_target_properties(HeadlessTrainer PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(GraphicsPlayground PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Set position independent code
set_target_properties(Graphics PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Misc PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Training PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# Enable all warnings
foreach(target ${ST_TARGETS})
    target_compile_definitions(${target} PUBLIC SPDLOG_FMT_EXTERNAL)
    if(MSVC)
    target_compile_options(${target} PRIVATE /W0)
    else(MSVC)
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
    endif(MSVC)    
endforeach(target ${ST_TARGETS})

# Libraries included in repository
option(SPDLOG_FMT_EXTERNAL "" ON)
## Fmt
add_subdirectory(${FMT_DIR})
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## PyTorch
if (NOT TORCH_FOUND)
    find_package(Torch REQUIRED)
    if (TORCH_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS ${TORCH_CXX_FLAGS})
    endif(TORCH_CXX_FLAGS)
endif (NOT TORCH_FOUND)
## spdlog
if (NOT TARGET spdlog)
    option(SPDLOG_BUILD_TESTING "" OFF)
    add_subdirectory(${SPDLOG_DIR})
endif(NOT TARGET spdlog)
## CppRl
option(CPPRL_BUILD_TESTS "" OFF)
option(CPPRL_BUILD_EXAMPLE "" OFF)
add_subdirectory(${CPPRL_DIR})
set_target_properties(cpprl PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
target_compile_definitions(SingularityTrainer PRIVATE "GLFW_INCLUDE_NONE")
target_compile_definitions(GraphicsPlayground PRIVATE "GLFW_INCLUDE_NONE")
add_subdirectory(${GLFW_DIR})
set_target_properties(glfw PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLAD
add_library(glad ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLM
option(GLM_TEST_ENABLE "" OFF)
add_subdirectory(${GLM_DIR})
## imgui
add_subdirectory(${IMGUI_DIR})
set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## Box2D
add_subdirectory(${BOX2D_DIR})
set_target_properties(Box2D-cmake PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## Doctest
add_subdirectory(${DOCTEST_DIR})
## Json
option(JSON_BuildTests "" OFF)
add_subdirectory(${JSON_DIR})
## Python
find_package(PythonLibs REQUIRED)
## Pybind11
add_subdirectory(${PYBIND_DIR})
pybind11_add_module(PythonBindings "")
## ZMQ
option(ZMQ_BUILD_TESTS "" OFF)
add_subdirectory(${ZMQ_DIR})

# Link libraries
target_link_libraries(Misc PUBLIC doctest)
target_link_libraries(Training PUBLIC Graphics cpprl torch ${TORCH_LIBRARIES} tobanteGaming::Box2D doctest fmt::fmt ${PYTHON_LIBRARIES})
# UNIX systems sometimes need to explicitly link the threading library and filesystem library
if (UNIX)
    target_link_libraries(Training PUBLIC pthread stdc++fs)
endif(UNIX)
target_link_libraries(Graphics glad glfw ${GLFW_LIBRARIES} glm imgui doctest)
target_link_libraries(SingularityTrainer Graphics Misc Training nlohmann_json::nlohmann_json)
target_link_libraries(HeadlessTrainer Graphics Misc Training)
target_link_libraries(GraphicsPlayground Graphics Misc Training)
target_link_libraries(Server Graphics Misc Training)
target_link_libraries(PythonBindings PRIVATE Graphics Misc Training)
set_target_properties(PythonBindings PROPERTIES OUTPUT_NAME "singularity_trainer")
set_target_properties(PythonBindings PROPERTIES SUFFIX ".so")
# Inlcudes
set(INCLUDE_DIRS
    src
    ${CPPRL_DIR}/include
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${GLM_DIR} 
    ${SPDLOG_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${BOX2D_DIR}/Box2D
    ${MSGPACK_DIR}/include
    ${DOCTEST_DIR}/doctest
    ${ARGH_DIR}
    ${JSON_DIR}/include
    ${TORCH_INCLUDE_DIRS}
    ${TWEENY_DIR}/include
    ${PYBIND_DIR}/include
    ${PYTHON_INCLUDE_DIRS}
    ${FMT_DIR}/include
    ${TASKFLOW_DIR}
    ${ZMQ_DIR}/include
)
target_include_directories(SingularityTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(HeadlessTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(GraphicsPlayground PUBLIC ${INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${INCLUDE_DIRS})
target_include_directories(Graphics PUBLIC ${INCLUDE_DIRS})
target_include_directories(Misc PUBLIC ${INCLUDE_DIRS})
target_include_directories(Training PUBLIC ${INCLUDE_DIRS})
target_include_directories(PythonBindings PUBLIC ${INCLUDE_DIRS})

add_subdirectory(src)
