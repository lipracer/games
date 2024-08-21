cmake_minimum_required(VERSION 3.10)

project(games)

#set(GAMES_COMPILE_OPTIONS -Wall -Wextra -Wno-unused-parameter 
#-Wno-unused-function -Wno-unused-result -Wno-missing-field-initializers 
#-Wno-unknown-pragmas -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas 
#-Wno-strict-overflow -Wno-strict-aliasing -Wno-missing-braces -Wno-uninitialized
#-Wno-sign-compare -Wno-switch -Wno-implicit-fallthrough
#-Wno-return-type)
#
#if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
#  list(APPEND GAMES_COMPILE_OPTIONS -Wno-cast-function-type)
#else()
#  list(APPEND GAMES_COMPILE_OPTIONS -Wno-semicolon-before-method-body)
#endif()
#
#set(compile_options ${GAMES_COMPILE_OPTIONS})

add_library(HeaderLib INTERFACE)
target_include_directories(HeaderLib INTERFACE ${CMAKE_SOURCE_DIR}/include)

function(add_commom_library TargetName ...)
    add_library(${ARGV})
    target_link_libraries(${TargetName} PUBLIC HeaderLib)
    get_property(common_lipraries GLOBAL PROPERTY common_names)
    list(APPEND common_lipraries ${TargetName})
    set_property(GLOBAL PROPERTY common_names ${common_lipraries})
endfunction()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

##########################################################################
# SDL
##########################################################################
set(EXECUTABLE_NAME ${PROJECT_NAME})

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")


# Create an executable or a shared library based on the platform and add our sources to it
if (ANDROID)
	# The SDL java code is hardcoded to load libmain.so on android, so we need to change EXECUTABLE_NAME
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
	add_executable(${EXECUTABLE_NAME})
endif()

# NB: need link before sdl, we need regist games before sdl initialize
get_property(common_lipraries GLOBAL PROPERTY common_names)

foreach(it IN LISTS common_lipraries)
    target_link_libraries(games PUBLIC ${it})
endforeach()

if(NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
target_link_libraries(${EXECUTABLE_NAME} PRIVATE X11)
endif()

target_sources(${EXECUTABLE_NAME} 
PRIVATE 
    src/main.cpp
)

# What is iosLaunchScreen.storyboard? This file describes what Apple's mobile platforms
# should show the user while the application is starting up. If you don't include one,
# then you get placed in a compatibility mode that does not allow HighDPI.
# This file is referenced inside Info.plist.in, where it is marked as the launch screen file.
# It is also ignored on non-Apple platforms. 


# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

# Link SDL to our executable. This also makes its include directory available to us. 
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# set some extra configs for each platform
set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch. 
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

add_subdirectory(utils)
