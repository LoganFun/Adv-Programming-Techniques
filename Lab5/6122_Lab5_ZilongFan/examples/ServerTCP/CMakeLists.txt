
set(SRCROOT ${PROJECT_SOURCE_DIR}/examples/ServerTCP)

# all source files
set(SRC ${SRCROOT}/ServerTCP.cpp
        )

# define the sockets target
sfml_add_example(ServerTCP
                 SOURCES ${SRC}
                 DEPENDS sfml-network)
