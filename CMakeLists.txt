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
set(CPPRL_DIR ${LIB_DIR}/pytorch-cpp-rl)
set(DOCTEST_DIR ${LIB_DIR}/doctest)
set(EASY_PROFILER_DIR ${LIB_DIR}/easy_profiler)
set(ENTT_DIR ${LIB_DIR}/entt)
set(GLFW_DIR ${LIB_DIR}/GLFW)
set(GLAD_DIR ${LIB_DIR}/GLAD)
set(IMGUI_DIR ${LIB_DIR}/imgui)
set(JSON_DIR ${LIB_DIR}/json)
set(MSGPACK_DIR ${LIB_DIR}/msgpack-c)
set(NANOVG_DIR ${LIB_DIR}/nanovg)
set(PYBIND_DIR ${LIB_DIR}/pybind11)
set(SOLOUD_DIR ${LIB_DIR}/soloud)
set(TROMPELOEIL_DIR ${LIB_DIR}/trompeloeil)
set(TWEENY_DIR ${LIB_DIR}/tweeny)
set(ZMQ_DIR ${LIB_DIR}/libzmq)

# CMake Properties
set(CMAKE_CXX_STANDARD 17)
if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -rdynamic")
endif(UNIX)
if(MSVC)
    message(STATUS "Static linking CRT")
    add_compile_options(
        "$<$<CONFIG:Debug>:/MTd>"
        "$<$<CONFIG:RelWithDebInfo>:/MT>"
        "$<$<CONFIG:Release>:/MT>"
        "$<$<CONFIG:MinSizeRel>:/MT>"
    )
endif(MSVC)

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
    message(STATUS "IPO / LTO supported")
else(IPO_SUPPORTED)
    message(STATUS "IPO / LTO not supported: <${IPO_ERROR}>")
endif(IPO_SUPPORTED)

# Options/definitions for all targets
foreach(target ${ST_TARGETS})
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
## Cpp Taskflow
find_package(Cpp-Taskflow CONFIG REQUIRED)
## Fmt
find_package(fmt CONFIG REQUIRED)
## PyTorch
if (NOT TORCH_FOUND)
    find_package(Torch REQUIRED)
    if (TORCH_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS ${TORCH_CXX_FLAGS})
    endif(TORCH_CXX_FLAGS)
endif (NOT TORCH_FOUND)
message(STATUS "Torch install location: ${TORCH_INSTALL_PREFIX}")
## spdlog
find_package(spdlog CONFIG REQUIRED)
## ZMQ
option(ZMQ_BUILD_TESTS "" OFF)
option(WITH_DOC "" OFF)
option(BUILD_SHARED "" OFF)
add_subdirectory(${ZMQ_DIR})
## cURL
find_package(CURL CONFIG REQUIRED)
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
## EnTT
option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." OFF)
SET(BUILD_TESTING OFF CACHE BOOL "Build EnTT tests")
set(FIND_GTEST_PACKAGE)
add_subdirectory(${ENTT_DIR})
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
find_package(glm CONFIG REQUIRED)
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
## Soloud
option (SOLOUD_DYNAMIC "Set to ON to build dynamic SoLoud" OFF)
option (SOLOUD_STATIC "Set to ON to build static SoLoud" ON)
option (SOLOUD_BUILD_DEMOS "Set to ON for building demos" OFF)
option (SOLOUD_BACKEND_NULL "Set to ON for building NULL backend" OFF)
option (SOLOUD_BACKEND_SDL2 "Set to ON for building SDL2 backend" OFF)
option (SOLOUD_BACKEND_COREAUDIO "Set to ON for building CoreAudio backend" OFF)
option (SOLOUD_BACKEND_OPENSLES "Set to ON for building OpenSLES backend" OFF)
option (SOLOUD_BACKEND_XAUDIO2 "Set to ON for building XAudio2 backend" OFF)
option (SOLOUD_BACKEND_WASAPI "Set to ON for building WASAPI backend" OFF)
if (UNIX)
    message(STATUS "Using ALSA audio backend")
    option (SOLOUD_BACKEND_WINMM "Set to ON for building WINMM backend" OFF)
    add_definitions("-DWITH_ALSA")
else (UNIX)
    message(STATUS "Using WinMM audio backend")
    option (SOLOUD_BACKEND_WINMM "Set to ON for building WINMM backend" ON)
    add_definitions("-DWITH_WINMM")
endif (UNIX)
add_definitions("-DWITH_NULL")
add_subdirectory(${SOLOUD_DIR}/contrib)
target_sources(soloud PRIVATE ${SOLOUD_DIR}/src/backend/null/soloud_null.cpp)
if (UNIX)
    target_sources(soloud PRIVATE ${SOLOUD_DIR}/src/backend/alsa/soloud_alsa.cpp)
else (UNIX)
    target_sources(soloud PRIVATE ${SOLOUD_DIR}/src/backend/winmm/soloud_winmm.cpp)
endif (UNIX)
## Box2D
find_package(unofficial-box2d CONFIG REQUIRED)
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
    Cpp-Taskflow::Cpp-Taskflow
    CURL::libcurl
    doctest
    EnTT::EnTT
    fmt::fmt-header-only
    glad
    glfw
    ${GLFW_LIBRARIES}
    glm
    imgui
    libzmq-static
    nanovg
    soloud
    spdlog::spdlog_header_only
    unofficial::box2d::box2d
    ${TORCH_LIBRARIES}
)
# UNIX systems sometimes need to explicitly link the threading library
if (UNIX)
    message(STATUS "Linking against Linux libraries")
    target_link_libraries(shared PUBLIC asound pthread)
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
    ${DOCTEST_DIR}/doctest
    ${EASY_PROFILER_DIR}/easy_profiler_core/include
    ${GLAD_DIR}/include 
    ${GLFW_DIR}/include 
    ${SOLOUD_DIR}/include
    ${TORCH_INCLUDE_DIRS}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/examples
    ${MSGPACK_DIR}/include
    ${ARGH_DIR}
    ${JSON_DIR}/include
    ${TROMPELOEIL_DIR}/include
    ${TWEENY_DIR}/include
    ${PYBIND_DIR}/include
    ${PYTHON_INCLUDE_DIRS}
    ${ZMQ_DIR}/include
)

target_include_directories(artificialinsentience SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(headlesstrainer SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(graphicsplayground SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(server SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(shared SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})
target_include_directories(pythonbindings SYSTEM PUBLIC ${NO_ERR_INCLUDE_DIRS})

add_subdirectory(src)