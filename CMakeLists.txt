cmake_minimum_required(VERSION 3.14)

# nome progetto versione esclusivamente C++
project(DCT VERSION 1.0 LANGUAGES CXX)

# abilita export dei comandi di compilazione per strumenti esterni (es. clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++17 standard e raise error se non supportato
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# se non specificato il build type, builda in release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# cerca Eigen >= 3.3 raise error se non trovato
find_package(Eigen3 3.3 REQUIRED NO_MODULE)


add_executable(DCTComparison dct2compare.cpp)
target_link_libraries(DCTComparison PUBLIC Eigen3::Eigen)
target_link_libraries(DCTComparison PRIVATE fftw3 m)