cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,这个CMakeLists.txt文件只会生成一次，所以可以在这里做修改
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

## CubeMX生成的CMake工程结构中，只有cmake/stm32cubemx/CMakeLists.txt文件会被CubeMX自动更新
## 所以不要在那个文件中添加自己的代码，而是在这个文件中添加自己的代码或者自行添加

# 引入工具.cmake
include(${CMAKE_SOURCE_DIR}/cmake/cmake_func.cmake) # 递归包含头文件的函数，添加该文件的位置


# Setup compiler settings 设置编译器相关
include(${CMAKE_SOURCE_DIR}/cmake/compiler_settings.cmake)

# 设置dsp支持
include(${CMAKE_SOURCE_DIR}/cmake/dsp_settings.cmake)


# Define the build type 默认使用DEBUG模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 指定交叉编译工具链，不论是在Linux还是Windows下，都使用arm-none-eabi
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


# Generic compiler settings for optimization and basic link lib
# add_compile_options(-pipe ${MCU_FLAGS} -Wall -Werror -fmessage-length=0 # basic options
add_compile_options(-pipe ${MCU_FLAGS} -Wall -fmessage-length=0 # basic options
-ffunction-sections -fdata-sections -fno-common # optimize options 
)
add_link_options(-pipe ${MCU_FLAGS} -T${LINKER_SCRIPT}  # close RWX warning
-lc -lstdc++ -lm  # lib options
-Wl,--gc-sections -flto -specs=nano.specs -specs=nosys.specs # optimize options
-Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map -Wl,--cref -Wl,--print-memory-usage # map options
) # if your executable is too large , try option '-s' to strip symbols

# 开启f4系列芯片的FPU单精度浮点运算
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

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
    -DUSE_FULL_ASSERT
    -DUSE_RTOS_FOR_UART
    -DTOPICS_DEBUG
    -DDEBUG_FOR_ACTION 
    -DUSE_ACTION_FOR_IMU
    # -DCHASSIS_TO_DEBUG
    # -DTEST_SYSTEM_TURNER
    # -DTEST_SYSTEM_M3508 
    # -DTEST_SYSTEM_M2006
    # -DTEST_SYSTEM_GM6020 
    -DDISABLE_LOG_SYSTEM
    # -DDISABLE_SYSVIEW_SYSTEM
    -DDEBUG_TASK_RUN 
    # -DDEBUG_NO_TRACKING
    # -DTEST_YAW_ADJUST
    # -DDEBUG_GO1_MOTOR
    -DTEST_VESC
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
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/USB_DEVICE)

# Add sources to executable 
file(GLOB_RECURSE SOURCES
    "Drivers/*.c"
    "Core/*.c"
    "Middlewares/*.c"
    "App/*.c"
    "Bsp/*.c"
    "Module/*.c"
    "Module/*.cpp"
    "App/*.cpp"
    "USB_DEVICE/*.c"
)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${SOURCES}
)

# 使用函数，传入根目录下的子目录名，递归包含所有子目录
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/App)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Bsp)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Core)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Drivers)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Middlewares)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Module)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/USB_DEVICE)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    DISABLEFLOAT16
    USE_FULL_ASSERT
    USE_RTOS_FOR_UART
    TOPICS_DEBUG
    DEBUG_FOR_ACTION 
    USE_ACTION_FOR_IMU
    # CHASSIS_TO_DEBUG # 当前板是否为底盘控制板
    # USE_OMNI_CHASSIS # 使用全向轮底盘
    USE_SWERVE_CHASSIS # 使用舵轮底盘
    USE_AIRJOY_CONTROL # 使用航模遥控
    # TEST_SYSTEM_TURNER
    # TEST_SYSTEM_M3508 
    # TEST_SYSTEM_M2006
    # TEST_SYSTEM_GM6020 
    DISABLE_LOG_SYSTEM
    # DISABLE_SYSVIEW_SYSTEM
    DEBUG_TASK_RUN 
    # DEBUG_NO_TRACKING
    # TEST_YAW_ADJUST
    DEBUG_GO1_MOTOR
    RUBBER_ANGLE_OPTIMIZE # 使用舵向劣弧优化
    TRY_AUTO_CONTROL # 使用自动控制
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    SEGGER
    dsp_settings
)

#include transfer tasks after build
include("cmake/post_build_tasks.cmake")

add_custom_target(download_jlink
    COMMAND JFlash -openprj"${CMAKE_SOURCE_DIR}/ToolChain/STM32F407ZGT6.jflash" -open"${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex",0x08000000 -auto -startapp -exit
    DEPENDS ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
    COMMENT "Download ${CMAKE_PROJECT_NAME}.hex to target device"
)