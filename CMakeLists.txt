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

# Set the project name
set(CMAKE_PROJECT_NAME new_build)

# Include toolchain file 设置arm-none-eabi 工具链
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C C++ languages
enable_language(C CXX ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources 这里主要是你通过CubeMX生成的部分，这部分源码CubeMX会自动帮你打包到cmake/stm32cubemx/CMakeLists.txt中
add_subdirectory(cmake/stm32cubemx)
# Add SEGGER sources 添加SEGGER的源码，这部分源码SEGGER会自动帮你打包到cmake/SEGGER/CMakeLists.txt中
add_subdirectory(cmake/SEGGER)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

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
    USE_FULL_ASSERT
    USE_RTOS_FOR_UART
    TOPICS_DEBUG
    DEBUG_FOR_ACTION 
    USE_ACTION_FOR_IMU
    # CHASSIS_TO_DEBUG # 当前板是否为底盘控制板
    USE_OMNI_CHASSIS # 使用全向轮底盘
    # USE_SWERVE_CHASSIS # 使用舵轮底盘
    # USE_AIRJOY_CONTROL # 使用航模遥控
    XBOX_CONTROL
    # TEST_SYSTEM_TURNER
    # TEST_SYSTEM_M3508 
    # TEST_SYSTEM_M2006
    # TEST_SYSTEM_GM6020 
    DISABLE_LOG_SYSTEM
    # DISABLE_SYSVIEW_SYSTEM
    DEBUG_TASK_RUN 
    # DEBUG_NO_TRACKING
    # TEST_YAW_ADJUST
    # DEBUG_GO1_MOTOR
    RUBBER_ANGLE_OPTIMIZE # 使用舵向劣弧优化
    TRY_AUTO_CONTROL # 使用自动控制
    # TEST_VESC
    TEST_DM
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