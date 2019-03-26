target_sources(Graphics
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/itransformable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sprite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/post_proc_layer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/render_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/font.cpp
)

target_sources(GraphicsPlayground PUBLIC ${CMAKE_CURRENT_LIST_DIR}/graphics_playground.cpp)

add_subdirectory(screens)
add_subdirectory(renderers)
add_subdirectory(backend)