set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.20)
project(gcgos C ASM)


set(CMAKE_C_COMPILER clang)
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_C_FLAGS "--target=aarch64-elf -g -O0 -ffreestanding -nostdlib")
# set(CMAKE_EXE_LINKER_FLAGS "--target=aarch64-elf -nostdlib -ffreestanding -T ${CMAKE_CURRENT_SOURCE_DIR}/source/kernel.ld")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/source/kernel.ld")




file(GLOB SOURCES_C "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c")
file(GLOB SOURCES_ASM "${CMAKE_CURRENT_SOURCE_DIR}/source/*.S")

add_executable(kernel8 ${SOURCES_C} ${SOURCES_ASM})

target_include_directories(kernel8 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(kernel8 PRIVATE "--target=aarch64-elf -g -O0 -ffreestanding")

set_target_properties(kernel8 PROPERTIES OUTPUT_NAME "kernel8.elf")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
    COMMAND llvm-objcopy -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel8.elf ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
    DEPENDS kernel8
    COMMENT "Generating kernel8.img from kernel8.elf"
)

add_custom_target(run-qemu
  # COMMAND qemu-system-aarch64 -M raspi4b -kernel ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img -nographic -S -s
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
    COMMENT "Running kernel8.img in QEMU"
)

