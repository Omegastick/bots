target_sources(Server
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/action_store.cpp
    ${CMAKE_CURRENT_LIST_DIR}/game.cpp
    ${CMAKE_CURRENT_LIST_DIR}/msgpack_codec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server_communicator.cpp
)

target_sources(SingularityTrainer
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/client_communicator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server_communicator.cpp
)