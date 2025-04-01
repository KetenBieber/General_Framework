# bsp_usart 编写
## 1.头文件准备
- 使用宏 确定要挂载什么库的接口，目前学院统一使用的是hal库，所以这里默认提供hal库的接口，后续如果需要移植到别的芯片类型，可以提供对应的硬件库接口
- 为module层露出接口
    - usart interface 编写
    这里是写回调函数的格式，你需要注意：不要直接拿传进去的实例来操作，而是需要在函数里面用局部的实例指针去取 传进来的 实例指针，并且进行类型转换！

    ```cpp
    uint8_t my_uart_callback(void* uart_device) {
    // 将 void* 类型的参数转换为 Uart_Instance_t* 类型
    Uart_Instance_t* uart_instance = (Uart_Instance_t*)uart_device;

    // 处理接收到的数据
    // 例如，访问 uart_instance->uart_package->rx_buffer
    // ...

    return 0; // 返回值根据需要定义
    }
    ```

# 如何新建一个串口设备
```cpp
Uart_Insatance_t *Action_Device;
uart_package_t action_package ={
    ... // 初始化

};

typedef struct
{
    
}action_data;

void *action_queue;

Action_Device = Uart_Register(&action_package, &queue_handler,10,sizeof(action_data));

if(Action_Device == NULL)
{
    LOGERROR("create failed!");
}
else
{
    LOGINFO("create successfully!");
}

```
    