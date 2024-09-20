cmake_minimum_required(VERSION 3.16)
# 指定编译平台/架构与语言标准, 推荐指定Ninja为构建工具,可以加快编译速度(相比make)
set(CMAKE_SYSTEM_NAME Generic) # 表示正在为一个通用的系统编译，而非指定的操作系统
set(CMAKE_SYSTEM_PROCESSOR arm) # 指定目标处理器架构为 arm
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY) # 指示cmake在尝试编译时生成静态库
# 指定工具链

set(CMAKE_C_COMPILER_FORCED TRUE) # skip compiler test 跳过编译器测试，直接使用指定的编译器
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER arm-none-eabi-gcc) # 指定c语言编译器为arm-none-eabi-gcc
set(CMAKE_CXX_COMPILER arm-none-eabi-g++) # 指定c++语言编译器为arm-none-eabi-g++
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc) # 指定汇编编译器为arm-none-eabi-gcc
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size) 
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_CXX_STANDARD 17) # 指定c++标准
set(CMAKE_C_STANDARD 17) # 指定c标准
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成编译命令的JSON文件
# 指定工程名称和语言类型
project(new_build C CXX ASM)  

# 选择构建类型
set(CMAKE_BUILD_TYPE Debug) # Debug Release RelWithDebInfo MinSizeRel

# board specific settings, arch/fpu/instruction
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mthumb-interwork -mfloat-abi=hard -mfpu=fpv4-sp-d16)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/ToolChain/STM32F407ZGTx_FLASH.ld") # 指定链接脚本

# option(BUILD_DSP ON) # 是否编译DSP库,若要定制须修改DSP/Source/CMakeLists.txt
# 定义选项

# Generic compiler settings for optimization and basic link lib
add_compile_options(-pipe ${MCU_FLAGS} -Wall -Werror -fmessage-length=0 # basic options
-ffunction-sections -fdata-sections -fno-common # optimize options 
)
add_link_options(-pipe ${MCU_FLAGS} -T${LINKER_SCRIPT} -Wl,--no-warn-rwx-segments # close RWX warning
-lc -lstdc++ -lm -lnosys # lib options
-Wl,--gc-sections -flto -specs=nano.specs -specs=nosys.specs # optimize options
-Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map -Wl,--cref -Wl,--print-memory-usage # map options
) # if your executable is too large , try option '-s' to strip symbols

# add_compile_definitions() works for compile stage
# while add_definitions() works for both compile and link stage
# 定义预处理宏
add_definitions(
    -DUSE_HAL_DRIVER 
    -DSTM32F407xx 
    -DARM_MATH_CM4 
    -DARM_MATH_MATRIX_CHECK 
    -DARM_MATH_ROUNDING 
    -DARM_MATH_LOOPUNROLL 
    -DDISABLEFLOAT16
    ) # need -D<macro> to define macro
    
# add inc 添加头文件路径
# .cmake是cmake的脚本文件，用于定义构建系统的配置和行为
include(${CMAKE_SOURCE_DIR}/ToolChain/find_header.cmake) # 递归包含头文件的函数，添加该文件的位置
# 使用函数，传入根目录下的子目录名，递归包含所有子目录
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/App)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Bsp)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Core)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Drivers)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Middlewares)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Module)

# add source, only surfix .c .cpp 添加源文件路径 
# GLOB_RECURSE 递归查找 将匹配的文件路径存储在变量 SOURCES 中
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/**/*.c"
    "${CMAKE_SOURCE_DIR}/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/**/*.cc"
    "${CMAKE_SOURCE_DIR}/**/*.hpp"
)
# 汇编文件路径
set(ASM_SOURCES
Toolchain/startup_stm32f407xx.s
Middlewares/Third_Party/SystemView/SEGGER/SEGGER_RTT_ASM_ARMv7M.s
)
# "-x assembler-with-cpp" 标志告诉编译器，这些汇编文件在编译之前需要经过c的预处理
# 在嵌入式开发中这样做就可以支持宏 和 其他预处理指令
set_source_files_properties(${ASM_SOURCES} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

# Build types 编译优化等级选择
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g -gdwarf-2)
add_definitions(-DESC_DEBUG) # ESC Debug
endif ()

# build binary and hex file 生成目标.elf 可执行文件
add_executable(${PROJECT_NAME}.elf  ${SOURCES} ${ASM_SOURCES} ${LINKER_SCRIPT})

# add_custom_command 添加自定义构建步骤
add_custom_command(
    TARGET ${PROJECT_NAME}.elf POST_BUILD # 指定自定义构建的对象是.elf
    COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex # 使用objcopy工具将elf文件格式转换成hex文件
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin # 使用ogjcopy工具将elf文件转换为二进制格式
    COMMENT "Building hex & bin file..." # 这些注释在构建过程中显示
    COMMENT "EXCUTABLE SIZE:"
    COMMAND ${SIZE} ${PROJECT_NAME}.elf # 使用size工具显示elf文件的大小信息
    )