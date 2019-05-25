target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_sources(HeadlessTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/headless.cpp
)

add_subdirectory(graphics)
add_subdirectory(misc)
add_subdirectory(screens)
add_subdirectory(third_party)
add_subdirectory(training)
add_subdirectory(ui)
