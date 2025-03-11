# 开启DSP数学运算库，支持硬件浮点运算
## 因为不同的TARGET对于DSP数学运算库的支持不同，所以这里需要根据TARGET来设置不同的预处理器宏定义
## 所以单独添加一个.cmake来进行管理

include(${CMAKE_SOURCE_DIR}/cmake/cmake_func.cmake) # 递归包含头文件的函数，添加该文件的位置


add_library(dsp_settings INTERFACE)

# 添加预处理器宏定义
target_compile_definitions(dsp_settings INTERFACE
    __FPU_PRESENT=1U 
    __FPU_USED=1U
    ARM_MATH_CM4
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    ARM_MATH_LOOPUNROLL
)

# 为了便于移植，所以这个文件会去检查其他使用它的用户有没有 add_compile_options 和 add_link_options 


# 检查编译标志
check_and_add_flags(COMPILE_FLAGS_TO_CHECK CMAKE_C_FLAGS)
set(COMPILE_FLAGS_ALREADY_SET ${flag_present})
# 检查链接标志
check_and_add_flags(LINK_FLAGS_TO_CHECK CMAKE_C_LINK_FLAGS)
set(LINK_FLAGS_ALREADY_SET ${flag_present})


# 如果编译标志未设置，则添加
if(NOT COMPILE_FLAGS_ALREADY_SET)
    add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
    message(STATUS "Added compile options: -mfloat-abi=hard -mfpu=fpv4-sp-d16")
endif()

# 如果链接标志未设置，则添加
if(NOT LINK_FLAGS_ALREADY_SET)
    add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
    message(STATUS "Added link options: -mfloat-abi=hard -mfpu=fpv4-sp-d16")
endif()