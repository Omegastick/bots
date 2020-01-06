target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/font.cpp
)

target_sources(graphicsplayground PUBLIC ${CMAKE_CURRENT_LIST_DIR}/graphics_playground.cpp)

add_subdirectory(backend)
add_subdirectory(post_processing)
add_subdirectory(renderers)
add_subdirectory(screens)