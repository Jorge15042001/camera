# cmake_minimum_required(VERSION 2.8)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# project(camera)
# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})
#
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
#
#
# add_executable(camera ./src/main.cpp ./src/util.cpp)
#
# target_link_libraries(camera ${OpenCV_LIBS}
#     PkgConfig::gstreamer
#     PkgConfig::gstreamer-video
#     PkgConfig::gobject-introspection
#
#   )
#

project(camera)
 
cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
set(CMAKE_BUILD_TYPE Debug)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)


include_directories( 
    ${CMAKE_CURRENT_BINARY_DIR}  
    ${GSTREAMER_INCLUDE_DIRS} 
    ${OpenCV_INCLUDE_DIRS} 
    )

add_definitions( ${GSTREAMER_CFLAGS_OTHER} )  

add_executable(camera 
                ./src/main.cpp
                ./src/util.cpp
            )

target_link_libraries(camera 
                    ${GSTREAMER_LIBRARIES} 
                    ${OpenCV_LIBS} 
                    )

# install(TARGETS camera RUNTIME DESTINATION bin)
 

