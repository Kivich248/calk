cmake_minimum_required(VERSION 3.10)
project(CalkLab)

set(CMAKE_CXX_STANDARD 17)

# Основная программа
add_executable(calk main.cpp)

# Тесты (отдельный файл)
add_executable(calk_tests tests.cpp)
# Если test.cpp не должен содержать main(), убери из него функцию main()
# и оставь только тесты, вызываемые из отдельного runner.cpp