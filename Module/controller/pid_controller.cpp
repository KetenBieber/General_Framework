/**
 * @file pid.cpp
 * @author Yang JianYi 
 * @brief PID类的实现文件，该文件封装了基本PID的算法实现，包括微分先行，不完全微分，积分限幅，积分隔离以及增量式和位置式PID的实现。只需要修改PID的配置参数即可使用。
 * 
 * ******************************************************************************************************************************
 *  使用发方法：
 *      配置PID参数(PID_Param_Init(),PID_Mode_Init())。 实时传入current值和target值，然后调用Adjust函数获取结果即可。
 * ******************************************************************************************************************************
 * @version 0.1
 * @date 2024-05-16
 * 
 */
// #include "pid_controller.h"

/* 
    我只能说吊
    防止freertos中中断打断任务产生的任务调度频率错误
    使用硬件定时器进行时间戳更新操作，直接由硬件维护，不受其他中断打断
    通过更加精确的时间计算，确保误差项和积分项的累积是正确的
    这对pid控制器中积分、微分项的计算非常重要
    通过注册函数的方式，将获取系统时间的函数传入pid控制器中，这样就可以在不同的系统中使用pid控制器
 */

// SystemTick_Fun PidTimer::get_systemTick = NULL;

// uint8_t PidTimer::update_timeStamp(void)
// {
//     uint32_t now_time=0;

//     //Check get sysClock
//     if(PidTimer::get_systemTick != NULL)
//     {
//         if(last_time == 0)
//         {
//             last_time = PidTimer::get_systemTick();
//             return 1;
//         }
//         now_time = PidTimer::get_systemTick();

//         //overflow 溢出处理--- 什么时候会溢出呢？如果上次时间大于这次时间，说明包溢出的
//         if(now_time < last_time)
//             dt = (float)((now_time + 0xFFFFFFFF) - last_time);  //now_time is 32bit, use 0XFFFFFFFF to avoid overflow
//         else
//             dt = (float)(now_time - last_time);
        
//         last_time = now_time;
        
//         dt *= 0.000001f;

//         return 0;
//     }
//     else
//     {
//         dt = 0;
//         return 1;
//     }
// }

// uint8_t PidTimer::getMicroTick_regist(uint32_t (*getTick_fun)(void))
// {
//     if(getTick_fun != NULL)
//     {
//         PidTimer::get_systemTick = getTick_fun;
//         return 1;
//     }
//     else 
//         return 0;
// }

// float PID::Adjust(void)
// {
//     //get real time failed --- pid控制器对时间要求高，否则会影响积分项和微分项的计算
//     if(update_timeStamp())
//         return 0;
    
//     //calculate error and deadzone
//     error = target - current;
//     if(_tool_Abs(error) < DeadZone)
//     {
//         Out=0;
//         return Out;
//     }
    
//     //lowpass filter, change the trust value to adjust the filter 误差项通过 低通滤波 进行处理
//     error = LowPass_error.f(error);

//     if(Imcreatement_of_Out)     //output increment mode
//         P_Term = Kp * error - Kp * pre_error;
//     else                        //output position mode
//           P_Term = Kp * error;

//     //calculate integral term, if use integral term
//     if(Ki!=0)
//     {
//         if(Imcreatement_of_Out)// 是否启用增量式
//             integral_e = error;
//         else
//             integral_e  += error*dt;
//         _tool_Constrain(&integral_e, -I_Term_Max/Ki, I_Term_Max/Ki);// 限幅
//     }
//     else
//     {
//         integral_e = 0;
//     }

//     //integral separate 积分分离阈值
//     if(_tool_Abs(error) < I_SeparThresh)// 误差是否过大，超过积分阈值
//     {
//         I_Term = Ki * integral_e;
//         _tool_Constrain(&I_Term, -I_Term_Max, I_Term_Max);
//     }
//     else
//     {
//         I_Term = 0;
//     }

//     float d_err = 0;
//     if(D_of_Current) // 是否启用微分先行
//     {
//         if(Imcreatement_of_Out)// 是否启用增量式PID
//             d_err = (current + eriler_Current - 2*pre_Current) / dt;
//         else
//             d_err = (current - pre_Current) / dt;
//     }
//     else
//     {
//         if(Imcreatement_of_Out)// 是否启用增量式PID
//             d_err = (error + eriler_error - 2*pre_error) / dt;
//         else
//             d_err = (error - pre_error) / dt;
//     }

//     d_err = LowPass_d_err.f(d_err);     //进行不完全微分 其实就是对微分项进行低通滤波
//     D_Term = Kd * d_err;

//     eriler_error = pre_error;
//     pre_error = error;
//     eriler_Current = pre_Current;
//     pre_Current = current;
//     if(Imcreatement_of_Out)// 是否使用增量式PID
//         Out = P_Term + I_Term + D_Term + last_out;
//     else
//         Out = P_Term + I_Term + D_Term;
//     last_out = Out;

//     _tool_Constrain(&Out, -Out_Max, Out_Max);// 输出限幅

//     return Out;// 返回输出
// }

// /**
//      * @brief PID参数初始化
//      * 
//      * @param _Kp  比例系数
//      * @param _Ki  积分系数
//      * @param _Kd  微分系数
//      * @param _I_Term_Max  I项限幅
//      * @param _Out_Max  输出限幅
//      * @param DeadZone  死区，fabs(error)小于DeadZone时，输出为0。设为负数不启用死区
//      * @param I_SeparThresh  积分分离阈值，fabs(error)大于该阈值取消积分作用。
//      */
// void PID::PID_Param_Init(float _Kp, float _Ki, float _Kd, float _I_Term_Max, float _Out_Max, float DeadZone)
// {
//     DeadZone = DeadZone;
//     Kp = _Kp;
//     Ki = _Ki;
//     Kd = _Kd;
//     I_Term_Max = _I_Term_Max;
//     Out_Max = _Out_Max;
// }
