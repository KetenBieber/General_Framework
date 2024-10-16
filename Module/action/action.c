/**
 * @file action.c
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 使用module层应注意！
 *              在app层实现bsp层接口和module层接口，module只管使用bsp层提供的接口，但是具体需要先进行bsp层接口的配置，才可以进行module层的初始化！

 * @note :
 * @versioninfo :
 */
#include "action.h"
#include "bsp_usart.h"
#include "cmsis_os2.h"


extern osThreadId_t ins_SensorTaskHandle;
extern osThreadId_t MotorTaskHandle;
union 
{
    uint8_t posture[24];
    float ActVal[6];
}action_posture;

/* 由于xQueueSendFromISR为宏定义，不能直接挂载函数指针，所以先用函数包装一下 */
static BaseType_t queue_send_wrapper(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken) {
    return xQueueSendFromISR(xQueue, pvItemToQueue, pxHigherPriorityTaskWoken);
}

static uint8_t Action_Rtos_Init(Action_Instance_t* action_instance,uint32_t queue_length);


Action_Instance_t* Action_Init(Uart_Instance_t *action_uart, IWDG_Instance_t *action_iwdg,uint32_t queue_length)
{
    if(action_uart == NULL)
    {
        LOGERROR("action uart is not prepared!");
        return NULL;
    }
    if(action_iwdg == NULL)
    {
        LOGERROR("action iwdg is not prepared!");
        return NULL; 
    }
    Action_Instance_t *temp_action_instance = (Action_Instance_t*)pvPortMalloc(sizeof(Action_Instance_t));
    if(temp_action_instance == NULL)
    {
        /* 如果pvPortMalloc失败了 */
        LOGERROR("action pvPortMalloc failed!");
        return NULL;
    }
    /* 挂载外部接口 */
    /* 检查看门狗是否挂载成功 */
    if(action_iwdg->dog_config == NULL || action_iwdg->fall_asleep == NULL || action_iwdg->feed_dog == NULL || action_iwdg->is_dog_online == NULL || action_iwdg->sleep_func_interface == NULL)
    {
        LOGERROR("action_iwdg have something is not valid!");
        return NULL;
    }
    action_iwdg->owner_id = (void*)temp_action_instance;// 看门狗存储父类
    temp_action_instance->action_iwdg_instance = action_iwdg;// 挂载看门狗实例
    /* 检查串口设备是否检查 */
    if(action_uart->uart_package.rx_buffer == NULL)
    {
        LOGERROR("action_uart have somethong is not vaild!");
        vPortFree(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    
    temp_action_instance->action_uart_instance = action_uart;// 在这里闲聊几句:开发流程中,一般是先开发bsp层,然后向编写module层的人提供bsp层暴露的接口,module层通过使用这些api开发module层,最终再向app层开发人员提供module层接口
    /* 挂载rtos接口 */
    if(Action_Rtos_Init(temp_action_instance,queue_length) != 1)
    {
        LOGERROR("rtos for uart init wrong!");
        vPortFree(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }

    /* 挂载内部接口 */
    temp_action_instance->action_orin_data = (action_original_info*)pvPortMalloc(sizeof(action_original_info));
    if(temp_action_instance->action_orin_data == NULL)
    {
        LOGERROR("action_orin_data pvPortMalloc failed!");
        vPortFree(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    memset(temp_action_instance->action_orin_data,0,sizeof(action_original_info));

    temp_action_instance->action_diff_data = (robot_info_from_action*)pvPortMalloc(sizeof(robot_info_from_action));
    if(temp_action_instance->action_diff_data == NULL)
    {
        LOGERROR("action_diff_data pvPortMalloc failed!");
        vPortFree(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    memset(temp_action_instance->action_diff_data,0,sizeof(robot_info_from_action));
    /* 挂载函数指针 */
    temp_action_instance->action_get_data = Action_GetData;
    temp_action_instance->action_refresh_data = Action_RefreshData;
    temp_action_instance->action_task = Action_Task;
    temp_action_instance->action_deinit = Action_Deinit;

    temp_action_instance->action_uart_instance->device = temp_action_instance;
    LOGINFO("action is prepared!");
    return temp_action_instance;
}


static uint8_t Action_Rtos_Init(Action_Instance_t* action_instance,uint32_t queue_length)
{
    if(action_instance == NULL)
    {
        LOGERROR("uart_rtos init failed!");
        return 0;
    }
    rtos_interface_t *rtos_interface = (rtos_interface_t *)pvPortMalloc(sizeof(rtos_interface_t));
    assert_param(rtos_interface != NULL);
    if(rtos_interface == NULL)
    {
        LOGERROR("rtos_interface pvPortMalloc failed!");
        return 0;
    }
    memset(rtos_interface,0,sizeof(rtos_interface_t));

    // 注册获取一个Freertos 队列句柄
    QueueHandle_t queue = xQueueCreate(queue_length,sizeof(UART_TxMsg));
    if(queue == NULL)
    {
        /* 如果队列创建失败 */
        LOGERROR("queue for uart create failed!");
        vPortFree(rtos_interface);
        rtos_interface = NULL;
        return 0;
    }
    else
    {
        // 如果队列创建成功，将队列句柄传递给rtos接口
        rtos_interface->xQueue = queue;
    }
    rtos_interface->queue_receive = xQueueReceive;
    rtos_interface->queue_send = queue_send_wrapper;    
    action_instance->rtos_for_action = rtos_interface;
    LOGINFO("uart rtos init success!");
    return 1;
}

uint8_t Action_Task(void* action_uart_instance)
{
    UART_TxMsg Msg;
    Action_Instance_t *temp_action_instance = action_uart_instance;
    // 接收方设置为portMAX_DELAY 在未收到数据时阻塞，减少cpu占用
    if(temp_action_instance->rtos_for_action->queue_receive(temp_action_instance->rtos_for_action->xQueue,&Msg,portMAX_DELAY) == pdPASS)
    {
#ifdef DEGUG_FOR_ACTION
        LOGINFO("action task is running!");
#endif
        temp_action_instance->action_get_data(Msg.data_addr,temp_action_instance->action_orin_data,temp_action_instance->action_diff_data);
        temp_action_instance->action_iwdg_instance->feed_dog(temp_action_instance->action_iwdg_instance);
        return 1;
    }
#ifdef DEGUG_FOR_ACTION
    LOGERROR("action task is not running!");
#endif
    return 0;
}


uint8_t Action_GetData(uint8_t *data,action_original_info *action_data,robot_info_from_action *info_from_action)
{
    if(data == NULL)
    {
        LOGWARNING("action data is NULL!");
        return 0;
    }
    /* 检查包头 */
    if(data[0] == 0x0d && data[1] == 0x0a)
    {
        // 没有操作
    }
    else
    {
        LOGWARNING("action_uart head error!");
        return 0;
    }
    
    /* 检查包尾 */
    if(data[26] == 0x0a && data[27] == 0x0d)
    {
        // 没有操作
    }
    else
    {
        LOGWARNING("action_uart tail error!");
        return 0;
    }

    for(int i = 0; i < 24; i++)
    {
        action_posture.posture[i] = data[i+2];
    }

#ifdef USE_DIFF_GET_DATA
    /* 储存上一次的值 */
    action_data->LAST_POS_X = action_data->POS_X;
    action_data->LAST_POS_Y = action_data->POS_Y;
    action_data->LAST_YAW = action_data->ANGLE_Z;
#endif
    action_data->ANGLE_Z = action_posture.ActVal[0];// 角度 -180~180
    action_data->ANGLE_X = action_posture.ActVal[1];
    action_data->ANGLE_Y = action_posture.ActVal[2];
    action_data->POS_X = action_posture.ActVal[3];
    action_data->POS_Y = action_posture.ActVal[4];
    action_data->W_Z = action_posture.ActVal[5];// 角速度
    // action_data->W_Z = 666;// 角速度


#ifdef USE_DIFF_GET_DATA
    /* 差分运算 */
    action_data->DELTA_POS_X = action_data->POS_X - action_data->LAST_POS_X;
    action_data->DELTA_POS_Y = action_data->POS_Y - action_data->LAST_POS_Y;
    action_data->DELTA_YAW = action_data->ANGLE_Z - action_data->LAST_YAW;

    /* 累加得出最终真实位置 */
    action_data->REAL_X += (-action_data->DELTA_POS_X);
    action_data->REAL_Y += (-action_data->DELTA_POS_Y);
    action_data->REAL_YAW += (-action_data->DELTA_YAW); 

    /* 在这里可以根据action实际安装位置进行坐标向底盘中心的转换,这里提供的是给出action位于底盘中心时的数据赋值 */
    info_from_action->x = action_data->REAL_X;
    info_from_action->y = action_data->REAL_Y;
    info_from_action->yaw = action_data->REAL_YAW;
    info_from_action->yaw_rate = action_data->W_Z*PI/180;

#endif

#ifdef DEGUG_FOR_ACTION
    LOGINFO("receive data!");
#endif
    memset(data,0,28);
    return 1;
}


uint8_t Action_RxCallback_Fun(void *action_instance, uint16_t data_len)
{
    UART_TxMsg Msg;
    if(action_instance == NULL)
    {
        LOGERROR("action_instance is NULL!");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = (Uart_Instance_t*)action_instance;
    Action_Instance_t *temp_action_instance = temp_uart_instance->device;

    if(temp_action_instance->rtos_for_action->xQueue !=NULL && temp_action_instance->action_uart_instance != NULL)
    {
        Msg.data_addr = temp_action_instance->action_uart_instance->uart_package.rx_buffer;
        Msg.len = data_len;
        Msg.huart = temp_action_instance->action_uart_instance->uart_package.uart_handle;
        if(Msg.data_addr != NULL)// 注意发送不能阻塞！
        {
            temp_action_instance->rtos_for_action->queue_send(temp_action_instance->rtos_for_action->xQueue,&Msg,NULL);
            return 1;
        }
    }
    return 0;
}


uint8_t Action_RefreshData(void)
{
    return 1;
}


// 又在这里瞎叭叭几句：为什么有的deinit需要传二级指针呢？因为在函数内部需要对指针进行操作，而在函数外部需要对指针进行释放，所以需要传二级指针
// 那为什么这里的不用呢？因为我在内部已经使用了一个指针指向了你传入的指针所指的地址，所以就可以直接实现对指针的操作了
uint8_t Action_Deinit(void *action_instance)
{
    /* 编写设备类的要点,记得调用所有父类的deinit函数 */
    if(action_instance == NULL)
    {
        LOGERROR("In Deinit action_instance is not valid!");
        return 0;
    }
    Action_Instance_t *temp_action_instance = action_instance;
    /* 注销父类,将串口设备类注销 */
    if(temp_action_instance->action_uart_instance->Uart_Deinit((temp_action_instance->action_uart_instance)) == 0)
    {
        LOGERROR("action uart deinit failed!");
        return 0;
    }
    /* 注销父类,将看门狗类注销 */
    if(temp_action_instance->action_iwdg_instance->iwdg_Deinit((temp_action_instance->action_iwdg_instance)) == 0)
    {
        LOGERROR("action iwdg deinit failed!");
        return 0;
    }
    /* 清除action设备中的接口 */
    if(temp_action_instance->action_orin_data != NULL)
    {
        vPortFree(temp_action_instance->action_orin_data);
        temp_action_instance->action_orin_data = NULL;
    }
    if(temp_action_instance->action_diff_data != NULL)
    {
        vPortFree(temp_action_instance->action_diff_data);
        temp_action_instance->action_diff_data = NULL;
    }

    if(temp_action_instance->rtos_for_action != NULL)
    {
        temp_action_instance->rtos_for_action->queue_receive =NULL;
        temp_action_instance->rtos_for_action->queue_send = NULL;
        if(temp_action_instance->rtos_for_action->xQueue != NULL)
        {
            vQueueDelete(temp_action_instance->rtos_for_action->xQueue);
            temp_action_instance->rtos_for_action->xQueue = NULL;
        }
        vPortFree(temp_action_instance->rtos_for_action);
        temp_action_instance->rtos_for_action = NULL;
    }
    /* 清除内部函数接口 */
    temp_action_instance->action_task = NULL;
    temp_action_instance->action_get_data = NULL;
    temp_action_instance->action_refresh_data = NULL;
    temp_action_instance->action_deinit = NULL;

    /* 清除action设备实例 */
    vPortFree(temp_action_instance);
    action_instance = NULL;

    return 1;
}
