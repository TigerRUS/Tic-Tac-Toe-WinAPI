cmake_minimum_required(VERSION 3.13)

project(Tic-Tac-Toe
	LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/Settings.cpp
	src/Settings.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gdi32
    user32
)

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
elseif(MINGW)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-mwindows")
endif()


