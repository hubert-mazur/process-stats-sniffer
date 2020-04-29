cmake_minimum_required(VERSION 3.14)
project(process_stats_sniffer C)

set(CMAKE_C_STANDARD 11)
add_executable(process_stats_sniffer main.c process.c process.h colors.h)
target_link_libraries(process_stats_sniffer ncurses)
