cmake_minimum_required(VERSION 3.12)

set(CMAKE_BUILD_TYPE Debug)

project(chess)

file(GLOB SRC 
"./src/main.cpp"
"./src/game.cpp"
"./src/GLFW_wnd.cpp"
"./src/board.cpp"
"./src/opengl.cpp"
"./src/chess.cpp"
"./src/glad.c"
)

file(GLOB TEST_SRC
"./src/test.cpp"
"./src/glad.c"
)

find_package(Boost COMPONENTS program_options system)
find_package(glfw3 REQUIRED)
 
if(Boost_FOUND)

    add_executable(${PROJECT_NAME} ${SRC})
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
    endif()
    target_include_directories(${PROJECT_NAME} ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options Boost::system glfw)
endif()