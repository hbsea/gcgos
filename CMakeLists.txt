set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.20)
project(gcgos C ASM)


set(CMAKE_C_COMPILER clang)
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_C_FLAGS "--target=aarch64-elf -g -O0 -nostdlib -ffreestanding")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld --target=aarch64-elf")


# user
file(GLOB U_SOURCES_C "${CMAKE_CURRENT_SOURCE_DIR}/user/*.c")
file(GLOB U_SOURCES_ASM "${CMAKE_CURRENT_SOURCE_DIR}/user/*.S")

foreach(u_src ${U_SOURCES_C})
    get_filename_component(name ${u_src} NAME_WE)
    add_executable(${name} ${u_src})
    target_compile_options(${name} PRIVATE "--target=aarch64-elf")
    target_link_options(${name} PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/user/user.ld")
    set_target_properties(${name} PROPERTIES OUTPUT_NAME "${name}.elf")

    set(OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${name}.o)
    set(BIN_FILE ${name}.bin)
    add_custom_command(
        OUTPUT ${OBJ_FILE}
        COMMAND llvm-objcopy -O binary $<TARGET_FILE:${name}> ${BIN_FILE}
        COMMAND ld.lld -m aarch64elf -r -b binary ${BIN_FILE} -o ${OBJ_FILE}
        DEPENDS ${name}
        COMMENT "Embedding ${name}.bin into ${name}.o"
    )

    add_custom_target(${name}_obj ALL DEPENDS ${OBJ_FILE})
    list(APPEND USER_OBJS ${OBJ_FILE})


endforeach()
message("${USER_OBJS}")

# kernel
file(GLOB K_SOURCES_C "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.c")
file(GLOB K_SOURCES_ASM "${CMAKE_CURRENT_SOURCE_DIR}/kernel/*.S")

add_library(kernel_obj OBJECT ${K_SOURCES_C} ${K_SOURCES_ASM})

target_include_directories(kernel_obj PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(kernel_obj PRIVATE "--target=aarch64-elf")

add_executable(kernel8 $<TARGET_OBJECTS:kernel_obj> ${USER_OBJS})
target_link_options(kernel8 PRIVATE "-T${CMAKE_CURRENT_SOURCE_DIR}/kernel/kernel.ld" "-nostdlib")
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


