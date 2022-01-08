project(objectview C)
cmake_minimum_required(VERSION 3.11)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(LIBSRC objectview.c ovbutton.c ovbar.c ovwindow.c ovcontext.c ovobject.c)
add_library(objectview SHARED ${LIBSRC})
target_link_libraries(objectview ${SDL2_LIBRARIES})

add_executable(ov objectview_main.c)
target_link_libraries(ov objectview)
