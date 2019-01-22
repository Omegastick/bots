# target_sources(SingularityTrainer
# PUBLIC
#     ${CMAKE_CURRENT_LIST_DIR}/communicator.cpp
#     ${CMAKE_CURRENT_LIST_DIR}/dummy_screen.cpp
#     ${CMAKE_CURRENT_LIST_DIR}/main.cpp
#     ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
#     ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
# )

add_subdirectory(actions)
add_subdirectory(agents)
add_subdirectory(environments)
add_subdirectory(modules)
add_subdirectory(trainers)
