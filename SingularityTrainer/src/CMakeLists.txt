target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/communicator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
)

target_sources(HeadlessTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/communicator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
    ${CMAKE_CURRENT_LIST_DIR}/headless.cpp
)

target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
)

target_sources(Tests
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/communicator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
)

add_subdirectory(graphics)
add_subdirectory(training)
add_subdirectory(screens)