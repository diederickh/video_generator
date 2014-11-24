cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app example)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
)

set(lib_sources
  ${sd}/video_generator.c
)

set(lib_headers
  ${sd}/video_generator.h
)

add_library(videogenerator ${lib_sources})
install(TARGETS videogenerator ARCHIVE DESTINATION lib)
install(FILES ${lib_headers} DESTINATION include)

add_executable(${app} ${sd}/example.c ${lib_sources})

if (UNIX AND NOT APPLE)
  target_link_libraries(${app} m)
endif()

if (WIN32)
else()
  target_link_libraries(${app} videogenerator pthread)
endif()

install(TARGETS ${app} DESTINATION bin)




