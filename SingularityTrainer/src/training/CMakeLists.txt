target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/rigid_body.cpp
)

target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/rigid_body.cpp
)

add_subdirectory(actions)
add_subdirectory(agents)
add_subdirectory(environments)
add_subdirectory(modules)
add_subdirectory(trainers)
add_subdirectory(entities)
