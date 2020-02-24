target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/ecs_env.cpp
)

add_subdirectory(observers)
add_subdirectory(systems)
add_subdirectory(utils)