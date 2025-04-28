cmake_minimum_required(VERSION 3.13)

project(Tic-Tac-Toe
	LANGUAGES C CXX)

add_executable(Tic-Tac-Toe
	src/main.cpp
	src/Settings.cpp
	src/Settings.h)

if(MSVC)
    set_target_properties(Tic-Tac-Toe PROPERTIES
        LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS")
endif()


