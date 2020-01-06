cmake_minimum_required(VERSION 3.15.0)
project(artificialinsentience)

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
set(NANOVG_DIR ${LIB_DIR}/nanovg)
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
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
endif(CCACHE_FOUND)

# Gold linker
if(UNIX)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -ldl")
endif(UNIX)

# Executables and local libraries
add_library(shared OBJECT "")
add_executable(artificialinsentience "")
add_executable(headlesstrainer "")
add_executable(graphicsplayground "")
add_executable(server "")
set(ST_TARGETS
    artificialinsentience
    headlesstrainer
    graphicsplayground
    server
    shared
)

# Disable headlesstrainer and graphicsplayground by default
set_target_properties(headlesstrainer PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(graphicsplayground PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Set position independent code
set_target_properties(shared PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

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
target_compile_definitions(curlpp_static PUBLIC CURL_STATICLIB)
## CppRl
option(CPPRL_BUILD_TESTS "" OFF)
option(CPPRL_BUILD_EXAMPLE "" OFF)
add_subdirectory(${CPPRL_DIR})
set_target_properties(cpprl PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## Easy Profiler
set(EASY_PROFILER_NO_SAMPLES TRUE)
add_subdirectory(${EASY_PROFILER_DIR})
set_target_properties(easy_profiler PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(easy_profiler PROPERTIES EXCLUDE_FROM_ALL TRUE)
if (TARGET profiler_gui)
    set_target_properties(profiler_gui PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif (TARGET profiler_gui)
set_target_properties(profiler_converter PROPERTIES EXCLUDE_FROM_ALL TRUE)
## GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
target_compile_definitions(artificialinsentience PRIVATE "GLFW_INCLUDE_NONE")
target_compile_definitions(graphicsplayground PRIVATE "GLFW_INCLUDE_NONE")
add_subdirectory(${GLFW_DIR})
set_target_properties(glfw PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLAD
add_library(glad STATIC)
target_sources(glad PRIVATE ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)
set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## GLM
option(GLM_TEST_ENABLE "" OFF)
add_subdirectory(${GLM_DIR})
## imgui
add_library(imgui STATIC)
target_include_directories(imgui 
PRIVATE
    ${IMGUI_DIR}/examples
    ${GLFW_DIR}/include
    ${GLAD_DIR}/include
PUBLIC 
    ${IMGUI_DIR}
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
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
target_link_libraries(imgui PUBLIC glad glfw glm)
set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
## NanoVG
add_library(nanovg STATIC)
target_include_directories(nanovg SYSTEM 
PRIVATE
    src/third_party
PUBLIC
    ${NANOVG_DIR}/src
)
target_sources(nanovg PRIVATE ${NANOVG_DIR}/src/nanovg.c)
target_link_libraries(nanovg glad)
set_target_properties(nanovg PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
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
pybind11_add_module(pythonbindings "")
set_target_properties(pythonbindings PROPERTIES EXCLUDE_FROM_ALL TRUE)

# Link libraries
target_link_libraries(shared
PUBLIC
    cpprl
    curlpp_static
    doctest
    fmt::fmt
    glad
    glfw
    ${GLFW_LIBRARIES}
    glm
    imgui
    libzmq-static
    nanovg
    tobanteGaming::Box2D
    ${TORCH_LIBRARIES}
)
# UNIX systems sometimes need to explicitly link the threading library
if (UNIX)
    message(STATUS "Linking against Linux libraries")
    target_link_libraries(shared PUBLIC pthread)
endif(UNIX)
target_link_libraries(artificialinsentience shared)
target_link_libraries(headlesstrainer shared)
target_link_libraries(graphicsplayground shared)
target_link_libraries(server shared)
target_link_libraries(pythonbindings PRIVATE ${PYTHON_LIBRARIES} shared)
set_target_properties(pythonbindings PROPERTIES OUTPUT_NAME "artificial_insentience")
set_target_properties(pythonbindings PROPERTIES SUFFIX ".so")

# Inlcudes
set(INCLUDE_DIRS src)
target_include_directories(artificialinsentience PUBLIC ${INCLUDE_DIRS})
target_include_directories(headlesstrainer PUBLIC ${INCLUDE_DIRS})
target_include_directories(graphicsplayground PUBLIC ${INCLUDE_DIRS})
target_include_directories(server PUBLIC ${INCLUDE_DIRS})
target_include_directories(shared PUBLIC ${INCLUDE_DIRS})
target_include_directories(pythonbindings PUBLIC ${INCLUDE_DIRS})

set(NO_ERR_INCLUDE_DIRS
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

target_include_directories(artificialinsentience SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(headlesstrainer SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(graphicsplayground SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(server SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(shared SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(pythonbindings SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})

add_subdirectory(src)
