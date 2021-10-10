cmake_minimum_required(VERSION 3.12)
# set_property(TARGET weight PROPERTY CXX_STANDARD 17)
set(CXX_STANDARD 17)
project(weight)

include_directories(
	./lib/ssh2/include)

set(LIB_MASS mass)
set(LIB_MPROCESS mprocess)

option(OPT_LIB_MASS_LOGGING "MASS - enable logging")

set(SRC_COMMON
	source/MASS_base.cpp
	source/Mthread.cpp
	source/Message.cpp
	source/Places_base.cpp
	source/Agents_base.cpp
	source/DllClass.cpp
	source/ExchangeHelper.cpp
	source/Socket.cpp
	source/Place.cpp
	source/Agent.cpp
	source/GraphModel.cpp
	source/VertexModel.cpp
	source/VertexPlace.cpp
	source/FileParser.cpp
	source/File.cpp
	source/TxtFile.cpp
	source/NetcdfFile.cpp)

set(SRC_MASS
	source/Utilities.cpp
	source/Ssh2Connection.cpp
	source/MASS.cpp
	source/MNode.cpp
	source/Places.cpp
	source/Agents.cpp
	${SRC_COMMON}
	source/GraphPlaces.cpp)

# Mass library

set(SHARED_LIB_MASS ${LIB_MASS})
add_library(${SHARED_LIB_MASS} SHARED ${SRC_MASS})
target_compile_options(${SHARED_LIB_MASS} PRIVATE "-fPIC" "-rdynamic")
target_link_libraries(${SHARED_LIB_MASS} ssh2 pthread dl)

set(STATIC_LIB_MASS ${LIB_MASS}_static)
add_library(${STATIC_LIB_MASS} STATIC ${SRC_MASS})
target_link_libraries(${STATIC_LIB_MASS} ssh2 pthread dl)

if(OPT_LIB_MASS_LOGGING)
	target_compile_definitions(${SHARED_LIB_MASS} PRIVATE LOGGING)
	target_compile_definitions(${STATIC_LIB_MASS} PRIVATE LOGGING)
endif()

install(TARGETS ${SHARED_LIB_MASS} DESTINATION ${CMAKE_SOURCE_DIR})