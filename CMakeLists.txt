cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME ComputerGraphics)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_NAME main)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp)
add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Lib
)

file(GLOB LIBS ${CMAKE_CURRENT_SOURCE_DIR}/Lib/*.lib)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LIBS})

find_package(OpenGL REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE OpenGL::GL OpenGL::GLU)

target_compile_definitions(${EXECUTABLE_NAME}
  PUBLIC
    GLM_ENABLE_EXPERIMENTAL
    TINYOBJLOADER_IMPLEMENTATION
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
  $<$<CONFIG:Debug>:OE_DEBUG>
  $<$<CONFIG:Release>:OE_RELEASE>
  $<$<CONFIG:Profiling>:OE_PROFILING>
)

if(CMAKE_BUILD_TYPE STREQUAL "Profiling")
    set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy" FORCE)
    add_subdirectory(Lib/tracy)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE Tracy::TracyClient)
endif()

# --- Assets ---
set(ASSETS_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/Assets)
set(ASSETS_BIN_DIR ${CMAKE_BINARY_DIR}/Assets)

message("\n\n\n")
message("${CMAKE_BINARY_DIR}")
message("\n\n\n")

file(GLOB_RECURSE ASSETS_SOURCES "${ASSETS_SRC_DIR}/*")
foreach(ASSET_SRC ${ASSETS_SOURCES})
    file(RELATIVE_PATH REL_PATH ${ASSETS_SRC_DIR} ${ASSET_SRC})

    set(ASSET_DST ${ASSETS_BIN_DIR}/${REL_PATH})

    get_filename_component(ASSET_DST_DIR ${ASSET_DST} DIRECTORY)

    file(MAKE_DIRECTORY ${ASSET_DST_DIR})

    add_custom_command(
        OUTPUT ${ASSET_DST}
        COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_SRC} ${ASSET_DST}
        DEPENDS ${ASSET_SRC}
        COMMENT "Copying ${ASSET_SRC} to ${ASSET_DST}"
        VERBATIM
    )

    list(APPEND ASSETS_OUTPUTS ${ASSET_DST})
endforeach()

add_custom_target(${EXECUTABLE_NAME}-assets DEPENDS ${ASSETS_OUTPUTS})
add_dependencies(${EXECUTABLE_NAME} ${EXECUTABLE_NAME}-assets)
