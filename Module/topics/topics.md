# pub-sub 话题发布订阅模块
## 1. 介绍
- 话题发布订阅模块是一个消息传递模块，用于在不同模块之间传递消息。
- 话题发布订阅模块是一个发布订阅模式的实现，它允许节点在特定的话题上发布消息，同时允许其他节点订阅这些话题。
- 为了实现各线程的解耦，所以尽量减少线程之间的直接通信（即直接使用全局变量进行通信的方式），采用话题订阅发布机制实现更加轻量级且具有线程安全的通信（相对freertos提供的任务间通讯方式）
- 这种机制的优点是，发布者和订阅者之间的关系是松散的，发布者和订阅者之间不需要知道对方的存在，只需要通过话题进行通信即可。并且如果有一方发布者死机，不会影响到订阅者的正常运行。并且通过看门狗，可以通过发布者的死机情况，进行相应处理

## 2. 话题发布订阅模块的使用
### 2.1 话题发布订阅模块的初始化
- 话题发布订阅模块的初始化函数如下：
只需要在系统初始化的时候调用一次即可（在freertos启动之前、其他各任务模块初始化之前（因为任务模块会依赖于这个话题发布订阅模块））
```c
void SubPub_Init();
```
### 2.2 创建一个发布者
举例：robot_ins线程中，需要将机器人姿态数据发布出去，给底盘任务使用
先定义话题上发布的消息类型
注意：需要使用1字节对齐，否则会出现数据错乱
```c
#pragma pack(1)

/* 将发布的底盘数据消息 */
typedef struct 
{
    float x;
    float y;
    float yaw;
}pub_Chassis_Pos;
/* 将发布的imu数据类型 */
typedef struct 
{
    float yaw;
}pub_imu_yaw;

#pragma pack()
```
然后创建发布者
```c
Publisher *chassis_imu_pub;// 创建一个发布者
pub_imu_yaw *imu_data;// 将发布的航向角数据
Publisher *chassis_pos_pub;// 创建一个发布者
pub_Chassis_Pos *pos_data;// 将发布的坐标数据
```
```c
/* 先创建局部变量publish_data 这是实际上被传输的通用消息类型，我们要发布的信息都会被转换成这种消息类型 */
publish_data p_chassis_imu_data;
publish_data p_chassis_pos_data;
/* 发布的数据指针申请内存 */
ins_interface.imu_data = (pub_imu_yaw*)pvPortMalloc(sizeof(pub_imu_yaw));
assert_param(ins_interface.imu_data != NULL);
ins_interface.pos_data = (pub_Chassis_Pos*)pvPortMalloc(sizeof(pub_Chassis_Pos));
assert_param(ins_interface.pos_data != NULL);
/* 调用register_pub函数进行发布者的注册，传参为"topics_name",就是你要发布的话题名字 */
ins_interface.chassis_imu_pub = register_pub("chassis_imu_pub");
ins_interface.chassis_pos_pub = register_pub("chassis_pos_pub");
```
然后就是数据的发布
```c
ins_interface.imu_data->yaw = action_instance->action_diff_data->yaw;// 获取数据
p_chassis_imu_data.data = (uint8_t *)(ins_interface.imu_data);// 将数据指针赋值给通用消息类型
p_chassis_imu_data.len = sizeof(pub_imu_yaw);// 数据长度
ins_interface.chassis_imu_pub->publish(ins_interface.chassis_imu_pub,p_chassis_imu_data);// 调用publish函数进行数据发布
```
### 2.3 创建一个订阅者
举例：chassis线程中，需要获取机器人姿态数据
先定义话题上发布的消息
```c
/* 订阅发布 */
Subscriber* chassis_imu_sub;
Subscriber* chassis_pos_sub; 

/* 订阅数据类型指针 */
pub_imu_yaw *imu_data;// 调用它来读yaw值
pub_Chassis_Pos *pos_data;// 调用它来读位置值
```
然后就是注册操作
```c
chassis_imu_sub = register_sub("chassis_imu_pub");// 注册订阅者
chassis_pos_sub = register_sub("chassis_pos_pub");// 注册订阅者
imu_data = (pub_imu_yaw*)pvPortMalloc(sizeof(pub_imu_yaw));// 申请内存
assert_param(imu_data != NULL);
pos_data = (pub_Chassis_Pos*)pvPortMalloc(sizeof(pub_Chassis_Pos));// 申请内存
assert_param(pos_data != NULL);
```
然后就是数据的订阅
```c
publish_data chassis_imu_data;// 同样是先定义一个通用消息类型
chassis_imu_data = chassis_imu_sub->get_data(chassis_imu_sub);// 读取数据
if(chassis_imu_data != -1)// 如果已经有了新的数据
{
    // 读取，记得数据类型转换
    imu_data->yaw = (pub_imu_yaw*)chassis_imu_data.data;// 获取数据
}
```
