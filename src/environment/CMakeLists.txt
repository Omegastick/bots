target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/build_env.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ecs_env.cpp
)

add_subdirectory(observers)
add_subdirectory(serialization)
add_subdirectory(systems)
add_subdirectory(utils)