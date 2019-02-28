target_sources(Training
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/rigid_body.cpp
    ${CMAKE_CURRENT_LIST_DIR}/score_processor.cpp
)

add_subdirectory(actions)
add_subdirectory(agents)
add_subdirectory(environments)
add_subdirectory(modules)
add_subdirectory(trainers)
add_subdirectory(entities)
