cmake_minimum_required(VERSION 3.10)


project(SnakeGame C)


set(CMAKE_C_STANDARD 11)


find_package(Curses REQUIRED)


add_executable(snake_game 
    src/main.c 
    src/game.c 
    src/renderer.c 
    src/client.c 
    src/server.c 
    src/snake.c)  


target_link_libraries(snake_game ${CURSES_LIBRARIES})


include_directories(${CURSES_INCLUDE_DIR})

