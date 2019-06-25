target_sources(Training
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/checkpointer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mock_saver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rigid_body.cpp
    ${CMAKE_CURRENT_LIST_DIR}/saver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/score_processor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/training_program.cpp
)

add_subdirectory(actions)
add_subdirectory(bodies)
add_subdirectory(environments)
add_subdirectory(modules)
add_subdirectory(trainers)
add_subdirectory(entities)
