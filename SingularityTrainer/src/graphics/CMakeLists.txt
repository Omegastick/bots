target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/graphics_playground.cpp
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stb_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/itransformable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sprite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/post_proc_layer.cpp
)

add_subdirectory(screens)
add_subdirectory(renderers)
add_subdirectory(backend)