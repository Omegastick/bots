target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/random.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io.cpp
)

target_sources(HeadlessTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/resource_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/screen_manager.cpp
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

add_subdirectory(graphics)
add_subdirectory(screens)
add_subdirectory(third_party)
add_subdirectory(training)
add_subdirectory(ui)
