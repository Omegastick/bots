cmake_minimum_required(VERSION 3.15.0)
project(SingularityTrainer)

# CMake policies
cmake_policy(SET CMP0069 NEW)
if (UNIX)
    cmake_policy(SET CMP0077 NEW)
endif()

# Build Types
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel TSAN ASAN LSAN MSAN UBSAN"
    FORCE)

# ThreadSanitizer
set(CMAKE_C_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C compiler during ThreadSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds."
    FORCE)

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds."
    FORCE)

# LeakSanitizer
set(CMAKE_C_FLAGS_LSAN
    "-fsanitize=leak -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during LeakSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_LSAN
    "-fsanitize=leak -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during LeakSanitizer builds."
    FORCE)

# MemorySanitizer
set(CMAKE_C_FLAGS_MSAN
    "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2"
    CACHE STRING "Flags used by the C compiler during MemorySanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_MSAN
    "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2"
    CACHE STRING "Flags used by the C++ compiler during MemorySanitizer builds."
    FORCE)

# UndefinedBehaviour
set(CMAKE_C_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "Flags used by the C compiler during UndefinedBehaviourSanitizer builds."
    FORCE)
set(CMAKE_CXX_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "Flags used by the C++ compiler during UndefinedBehaviourSanitizer builds."
    FORCE)

# Configuration
set(LIB_DIR lib)
set(ARGH_DIR ${LIB_DIR}/argh)
set(BOX2D_DIR ${LIB_DIR}/Box2D-cmake)
set(CPPRL_DIR ${LIB_DIR}/pytorch-cpp-rl)
set(CURLPP_DIR ${LIB_DIR}/curlpp)
set(DOCTEST_DIR ${LIB_DIR}/doctest)
set(EASY_PROFILER_DIR ${LIB_DIR}/easy_profiler)
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
set(TROMPELOEIL_DIR ${LIB_DIR}/trompeloeil)
set(TWEENY_DIR ${LIB_DIR}/tweeny)
set(ZMQ_DIR ${LIB_DIR}/libzmq)

# CMake Properties
set(CMAKE_CXX_STANDARD 17)
if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
endif(UNIX)

# CppCheck
list(APPEND CPPCHECK_ARGS
    --enable=warning
    --std=c++14
    --verbose
    --quiet
    --inline-suppr
    --error-exitcode=1
    -j8
    --language=c++
    --config-exclude=${CMAKE_CURRENT_LIST_DIR}/src/third_party/stb_image.h
    --config-exclude=${CMAKE_CURRENT_LIST_DIR}/lib
    --suppressions-list=${CMAKE_CURRENT_LIST_DIR}/CppCheckSuppressions.txt 
    -i ${LIB_DIR}/
    -i ${CMAKE_CURRENT_LIST_DIR}/src/third_party/
    -I ${CMAKE_CURRENT_LIST_DIR}/src 
    -I ${CMAKE_CURRENT_LIST_DIR}/lib/*
    -I ${CMAKE_CURRENT_LIST_DIR}/lib/*/include
    -I ${CMAKE_CURRENT_LIST_DIR}/lib/doctest/doctest
    ${CMAKE_CURRENT_LIST_DIR}/src
)
add_custom_target(
    cppcheck
    COMMAND cppcheck ${CPPCHECK_ARGS}
    COMMENT "Running CppCheck"
)

# Ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

# Gold linker
if(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -ldl")
endif(UNIX)

# Executables and local libraries
add_library(Graphics OBJECT "")
add_library(Misc OBJECT "")
add_library(Networking OBJECT "")
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
    Networking
    Training
)

# Disable HeadlessTrainer and GraphicsPlayground by default
set_target_properties(HeadlessTrainer PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(GraphicsPlayground PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Set position independent code
set_target_properties(Graphics PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Misc PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Networking PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Training PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# Link time optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
    message(STATUS "IPO / LTO enabled")
else(IPO_SUPPORTED)
    message(STATUS "IPO / LTO not supported: <${IPO_ERROR}>")
endif(IPO_SUPPORTED)

# Options/definitions for all targets
foreach(target ${ST_TARGETS})
    target_compile_definitions(${target} PUBLIC SPDLOG_FMT_EXTERNAL)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W0)
        target_compile_definitions(${target} PUBLIC WIN32_LEAN_AND_MEAN)
    else(MSVC)
        target_compile_options(${target} PRIVATE 
            -Wall 
            -Wextra 
            -pedantic 
            -Wno-maybe-uninitialized
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wrestrict
            -Wnull-dereference
            -Wold-style-cast
            -Wuseless-cast
            # -Wdouble-promotion
            # -Wshadow
            -Wformat=2
            -Wconversion)
    endif(MSVC)    

    if(IPO_SUPPORTED AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif(IPO_SUPPORTED AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
endforeach(target ${ST_TARGETS})

# Libraries included in repository
option(SPDLOG_FMT_EXTERNAL "" ON)
## Agones
# find_package(agones CONFIG REQUIRED)
## Fmt
set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(${FMT_DIR})
set_target_properties(fmt PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## PyTorch
if (NOT TORCH_FOUND)
    find_package(Torch REQUIRED)
    if (TORCH_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS ${TORCH_CXX_FLAGS})
    endif(TORCH_CXX_FLAGS)
endif (NOT TORCH_FOUND)
message(STATUS "Torch install location: ${TORCH_INSTALL_PREFIX}")
## spdlog
if (NOT TARGET spdlog)
    option(SPDLOG_BUILD_TESTING "" OFF)
    add_subdirectory(${SPDLOG_DIR})
endif(NOT TARGET spdlog)
## ZMQ
option(ZMQ_BUILD_TESTS "" OFF)
option(WITH_DOC "" OFF)
option(BUILD_SHARED "" OFF)
add_subdirectory(${ZMQ_DIR})
## cURLpp
find_package(CURL REQUIRED)
add_subdirectory(${CURLPP_DIR})
set_target_properties(curlpp_static PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## CppRl
option(CPPRL_BUILD_TESTS "" OFF)
option(CPPRL_BUILD_EXAMPLE "" OFF)
add_subdirectory(${CPPRL_DIR})
set_target_properties(cpprl PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## Easy Profiler
set(EASY_PROFILER_NO_SAMPLES TRUE)
add_subdirectory(${EASY_PROFILER_DIR})
set_target_properties(easy_profiler PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(profiler_gui PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(profiler_converter PROPERTIES EXCLUDE_FROM_ALL TRUE)
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
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLM
option(GLM_TEST_ENABLE "" OFF)
add_subdirectory(${GLM_DIR})
## imgui
add_library(imgui STATIC)
target_include_directories(imgui 
PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
)
target_sources(imgui 
PRIVATE
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)
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
set_target_properties(PythonBindings PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Link libraries
target_link_libraries(Misc PUBLIC curlpp_static doctest imgui)
target_link_libraries(Graphics doctest easy_profiler glad glfw ${GLFW_LIBRARIES} glm imgui Misc)
target_link_libraries(Training PUBLIC cpprl doctest  fmt::fmt Graphics ${PYTHON_LIBRARIES} tobanteGaming::Box2D ${TORCH_LIBRARIES})
# UNIX systems sometimes need to explicitly link the threading library
if (UNIX)
    message(STATUS "Linking against Linux libraries")
    target_link_libraries(Training PUBLIC pthread)
endif(UNIX)
target_link_libraries(Networking Training libzmq-static)
target_link_libraries(SingularityTrainer Graphics Misc Training Networking)
target_link_libraries(HeadlessTrainer Graphics Misc Training)
target_link_libraries(GraphicsPlayground Graphics Misc Training)
target_link_libraries(Server Graphics Misc Training Networking)
target_link_libraries(PythonBindings PRIVATE Graphics Misc Training)
set_target_properties(PythonBindings PROPERTIES OUTPUT_NAME "singularity_trainer")
set_target_properties(PythonBindings PROPERTIES SUFFIX ".so")

# Inlcudes
set(INCLUDE_DIRS src)
target_include_directories(SingularityTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(HeadlessTrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(GraphicsPlayground PUBLIC ${INCLUDE_DIRS})
target_include_directories(Server PUBLIC ${INCLUDE_DIRS})
target_include_directories(Graphics PUBLIC ${INCLUDE_DIRS})
target_include_directories(Misc PUBLIC ${INCLUDE_DIRS})
target_include_directories(Networking PUBLIC ${INCLUDE_DIRS})
target_include_directories(Training PUBLIC ${INCLUDE_DIRS})
target_include_directories(PythonBindings PUBLIC ${INCLUDE_DIRS})

set(NO_ERR_INCLUDE_DIRS
    ${CURL_INCLUDE_DIRS}
    ${CPPRL_DIR}/include
    ${CURLPP_DIR}/include
    ${DOCTEST_DIR}/doctest
    ${EASY_PROFILER_DIR}/easy_profiler_core/include
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${GLM_DIR} 
    ${SPDLOG_DIR}/include
    ${TASKFLOW_DIR}
    ${TORCH_INCLUDE_DIRS}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${BOX2D_DIR}/Box2D
    ${MSGPACK_DIR}/include
    ${ARGH_DIR}
    ${JSON_DIR}/include
    ${TROMPELOEIL_DIR}/include
    ${TWEENY_DIR}/include
    ${PYBIND_DIR}/include
    ${PYTHON_INCLUDE_DIRS}
    ${FMT_DIR}/include
    ${ZMQ_DIR}/include
)

target_include_directories(SingularityTrainer SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(HeadlessTrainer SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(GraphicsPlayground SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(Server SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(Graphics SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(Misc SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(Networking SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(Training SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(PythonBindings SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})

add_subdirectory(src)
