cmake_minimum_required(VERSION 3.10)

# Definícia projektu
project(SnakeGame C)

# Nastavenie verzie jazyka C
set(CMAKE_C_STANDARD 11)

# Hľadanie knižnice ncurses
find_package(Curses REQUIRED)

# Zahrnutie adresárov pre ncurses
include_directories(${CURSES_INCLUDE_DIR})

# Pridanie spustiteľného súboru
add_executable(snake_game src/main.c src/game.c src/renderer.c)

# Prepojenie knižníc (pre ncurses)
target_link_libraries(snake_game ${CURSES_LIBRARIES})

