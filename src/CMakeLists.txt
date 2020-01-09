target_sources(artificialinsentience
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/app.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_sources(headlesstrainer
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/headless.cpp
    ${CMAKE_CURRENT_LIST_DIR}/headless_app.cpp
)

target_sources(server
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/server.cpp
)

target_sources(pythonbindings
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/python_bindings.cpp
)

target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/server_app.cpp
)

add_subdirectory(audio)
add_subdirectory(graphics)
add_subdirectory(misc)
add_subdirectory(screens)
add_subdirectory(networking)
add_subdirectory(third_party)
add_subdirectory(training)
add_subdirectory(ui)
