cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(cvutils)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Encontrar la biblioteca OpenCV
find_package(OpenCV REQUIRED)

# Añadir el ejecutable
add_executable(cvutils main.cpp)

# Enlazar OpenCV con el ejecutable
target_link_libraries(cvutils ${OpenCV_LIBS})

# Incluir directorios
include_directories(${OpenCV_INCLUDE_DIRS})