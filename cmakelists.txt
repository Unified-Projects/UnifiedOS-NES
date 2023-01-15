project(unified-nes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options($<$<C_COMPILER_ID:Clang>:-fcolor-diagnostics>)

add_compile_options(-Werror -Wall -Wextra -Wno-write-strings -Wno-missing-braces)
add_compile_options(-march=x86-64-v2)
# add_compile_options(-ffreestanding -nostdlib)

include_directories(include)
include(GNUInstallDirs)

set(unifienes_SRC
    src/BUS.cpp
    src/CPU.cpp
    src/PPU.cpp
    src/Cartridge.cpp
    
    src/Mappers/000/Mapper_000.cpp
    src/Mappers/001/Mapper_001.cpp
    src/Mappers/002/Mapper_002.cpp
    src/Mappers/003/Mapper_003.cpp
    src/Mappers/004/Mapper_004.cpp
    src/Mappers/066/Mapper_066.cpp
    
    src/main.cpp
)

# add_link_options(-z max-page-size=0x1000 -ffreestanding -nostdlib)
add_link_options(-lunified)

add_executable(nes.sys ${unifienes_SRC})

install(TARGETS nes.sys
    RUNTIME DESTINATION unified)