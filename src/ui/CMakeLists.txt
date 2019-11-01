add_subdirectory(build_screen)
add_subdirectory(create_program_screen)
add_subdirectory(multiplayer_screen)
add_subdirectory(train_screen)
add_subdirectory(watch_screen)

target_sources(Misc
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/plot.cpp
)

target_sources(SingularityTrainer
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/back_button.cpp
)