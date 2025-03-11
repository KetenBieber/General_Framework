# 管理编译后任务的.cmake
set(CMAKE_PROJECT_NAME              new_build)
set(CMAKE_OBJCOPY                   arm-none-eabi-objcopy)
set(CMAKE_SIZE                      arm-none-eabi-size)

# add_custom_command 添加自定义构建步骤
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD # 指定自定义构建的对象是.elf
    COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex # 使用objcopy工具将elf文件格式转换成hex文件
    COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin # 使用ogjcopy工具将elf文件转换为二进制格式
    COMMENT "Building hex & bin file..." # 这些注释在构建过程中显示
    COMMENT "EXCUTABLE SIZE:"
    COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf # 使用size工具显示elf文件的大小信息
)