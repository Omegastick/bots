target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/audio_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clean_up_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/distortion_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hill_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/module_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/particle_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/physics_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/render_system.cpp
    ${CMAKE_CURRENT_LIST_DIR}/trail_system.cpp
)

add_subdirectory(contact_handlers)
add_subdirectory(modules)