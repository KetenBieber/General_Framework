# 添加SEGGER的systemView工具
cmake_minimum_required(VERSION 3.22)

project(SEGGER)
add_library(SEGGER INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# 添加链接到目标的宏
target_compile_definitions(SEGGER INTERFACE  
	STM32F407xx
    $<$<CONFIG:Debug>:DEBUG>
)

# 添加源文件的库
target_include_directories(SEGGER INTERFACE
    ../../Middlewares/Third_Party/SystemView/SEGGER
    ../../Middlewares/Third_Party/SystemView/Config
)

# 添加源文件（.c 以及 .s）
target_sources(SEGGER INTERFACE
    ../../Middlewares/Third_Party/SystemView/SEGGER/SEGGER_RTT_printf.c
    ../../Middlewares/Third_Party/SystemView/SEGGER/SEGGER_RTT.c
    ../../Middlewares/Third_Party/SystemView/SEGGER/SEGGER_SYSVIEW_FreeRTOS.c
    ../../Middlewares/Third_Party/SystemView/SEGGER/SEGGER_SYSVIEW.c
    ../../Middlewares/Third_Party/SystemView/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.c
    ../../Middlewares/Third_Party/SystemView/Config/SEGGER_SYSVIEW_Config_FreeRTOS.c
    ../../Middlewares/Third_Party/SystemView/SEGGER/SEGGER_RTT_ASM_ARMv7M.s
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()