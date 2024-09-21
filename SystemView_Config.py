import os

# 定义文件路径
file_path_port = './Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c'
file_path_freertos = './Middlewares/Third_Party/FreeRTOS/Source/include/FreeRTOS.h'

# 定义要插入的代码片段
code_to_insert_before_port = '\t/* 添加systemview依赖项 */\n\ttraceISR_ENTER();\n\n'
code_to_insert_after_if_port = '\t\t\t/* 添加systemview依赖项 */\n\t\t\ttraceISR_EXIT_TO_SCHEDULER();\n'
code_to_insert_else_port = '\t\telse\n\t\t{\n\t\t\t/* 添加systemview依赖项 */\n\t\t\ttraceISR_EXIT();\n\t\t}\n'
code_to_insert_freertos = '\n/* 添加systemview依赖项 */\n#include "SEGGER_SYSVIEW_FREERTOS.h"\n'


# 读取文件内容
with open(file_path_port, 'r', encoding='utf-8') as file:
    lines_port = file.readlines()

with open(file_path_freertos, 'r', encoding='utf-8') as file:
    lines_freertos = file.readlines()

# 查找插入位置
insert_index_before_port = None
insert_index_after_if_port = None
insert_index_freertos = None


for i, line in enumerate(lines_port):
    if 'void xPortSysTickHandler( void )' in line:
        insert_index_before_port = i + 2  # 插入到函数定义之后
    if 'portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;' in line:
        insert_index_after_if_port = i + 1  # 插入到portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;之后

# 插入自定义代码
if insert_index_before_port is not None:
    lines_port.insert(insert_index_before_port, code_to_insert_before_port)

# 查找特定的if语句并插入代码
for i in range(len(lines_port)):
    if 'if( xTaskIncrementTick() != pdFALSE )' in lines_port[i]:
        for j in range(i, len(lines_port)):
            if '}' in lines_port[j]:
                insert_index_after_if_right_brace = j + 1
                break
        break

if insert_index_after_if_right_brace is not None:
    lines_port.insert(insert_index_after_if_right_brace, code_to_insert_else_port)

# 插入traceISR_EXIT_TO_SCHEDULER()到if语句内部
for i in range(len(lines_port)):
    if 'portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;' in lines_port[i]:
        lines_port.insert(i + 1, code_to_insert_after_if_port)
        break


for i, line in enumerate(lines_freertos):
    if '#include "portable.h"' in line:
        insert_index_freertos = i + 1  # 插入到#include "portable.h"之后

# 插入自定义代码
if insert_index_freertos is not None:
    lines_freertos.insert(insert_index_freertos, code_to_insert_freertos)


# 写回文件
with open(file_path_port, 'w', encoding='utf-8') as file:
    file.writelines(lines_port)

with open(file_path_freertos, 'w', encoding='utf-8') as file:
    file.writelines(lines_freertos)

print("Custom code inserted successfully.")