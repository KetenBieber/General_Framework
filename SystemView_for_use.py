import os

# 定义要插入的自定义代码
custom_code = """
/* USER CODE BEGIN CustomCode */
// 这里是你自己的代码
/* USER CODE END CustomCode */
"""

# 定义要插入代码的文件和位置
file_path = "../"
insert_after = "/* USER CODE BEGIN 0 */"

# 读取文件内容
with open(file_path, 'r') as file:
    content = file.read()

# 插入自定义代码
if insert_after in content:
    content = content.replace(insert_after, insert_after + custom_code)

# 写回文件
with open(file_path, 'w') as file:
    file.write(content)

print("Custom code inserted successfully.")