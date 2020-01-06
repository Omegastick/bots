target_sources(artificialinsentience
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(headlesstrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(graphicsplayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(server
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(pythonbindings
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/stb_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stb_truetype.cpp
)