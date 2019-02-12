target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/communicator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
)

target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
)

add_subdirectory(test_screen)
add_subdirectory(graphics)
add_subdirectory(training)
add_subdirectory(screens)
add_subdirectory(particles)