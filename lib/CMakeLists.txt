SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS date_time regex system)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIRS})

SET(PBFT_EXPORT_FILES
    export/include/BackdoorConnectionInterface.h
    export/include/ClientConnectionInterface.h
    export/include/ClientTypes.h
    export/include/ConnectionFactory.h
    export/include/DatabaseTypes.h)

SOURCE_GROUP("Source Files\\Export" FILES ${PBFT_EXPORT_FILES})

SET(PBFT_FILES
    BackdoorConnection.cpp
    BackdoorConnection.h
    ClientConnection.cpp
    ClientConnection.h
    Connection.cpp
    Connection.h
    ConnectionFactory.cpp
    Database.cpp
    Database.h
    DatabaseFactory.cpp
    DatabaseFactory.h
    DatabaseFactoryInterface.h
    DatabaseInterface.h
    LinkInterface.h
    LinkProxy.cpp
    LinkProxy.h
    Node.cpp
    Node.h
    NodeInterface.h
    Utilities.h)

SOURCE_GROUP("Source Files" FILES ${PBFT_FILES})

ADD_LIBRARY(pbft STATIC ${PBFT_EXPORT_FILES} ${PBFT_FILES})
TARGET_LINK_LIBRARIES(pbft ${Boost_LIBRARIES})
