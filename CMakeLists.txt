cmake_minimum_required(VERSION 3.31)
project(TP_C_Nicolas_Millet C)

set(CMAKE_C_STANDARD 11)

# --- SDL2 locale ---
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2")

# Inclure les headers
include_directories(${SDL2_DIR}/include)

# Créer l'exécutable
add_executable(TP_C_Nicolas_Millet main.c)

# Indiquer les libs (ici on suppose x64)
target_link_directories(TP_C_Nicolas_Millet PRIVATE ${SDL2_DIR}/lib/x64)

# Lier les bibliothèques SDL2
target_link_libraries(TP_C_Nicolas_Millet SDL2main SDL2)

# Copier la DLL à côté de l'exécutable après la compilation
add_custom_command(TARGET TP_C_Nicolas_Millet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DIR}/lib/x64/SDL2.dll"
        $<TARGET_FILE_DIR:TP_C_Nicolas_Millet>)

