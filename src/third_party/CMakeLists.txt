target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(HeadlessTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(GraphicsPlayground
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(Server
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(PythonBindings
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/doctest.cpp
)

target_sources(Shared
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/stb_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stb_truetype.cpp
)