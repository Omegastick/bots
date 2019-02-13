target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/graphics_playground.cpp
    ${CMAKE_CURRENT_LIST_DIR}/window.cpp
    ${CMAKE_CURRENT_LIST_DIR}/shader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vertex_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/element_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vertex_array.cpp
    ${CMAKE_CURRENT_LIST_DIR}/renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stb_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/itransformable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sprite.cpp
)

add_subdirectory(screens)