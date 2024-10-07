// #pragma once
// #ifdef __cplusplus

// #include <stddef.h>
// #include <limits.h>
// #include <stdint.h>
// #include "filter.h"
// #include "user_tool.h"

// typedef uint32_t (*SystemTick_Fun)(void);

// /* 
//     PID控制系统基类
 
//  */
// class PidTimer : public Tools
// {
// public:
//     static uint8_t getMicroTick_regist(uint32_t (*getTick_fun)(void));

// protected:
//     static SystemTick_Fun get_systemTick;
//     float  dt;
//     uint32_t last_time;
//     uint8_t update_timeStamp();
// };

// /* 
//     PID控制器基类
//  */
// class PID : public PidTimer 
// {
// public:
//     PID(){};
//     float Adjust();
//     void PID_Param_Init(float _Kp, float _Ki, float _Kd, float _I_Term_Max, float _Out_Max, float DeadZone);

//     /**
//      * @brief PID模式初始化
//      * @param DeadZone 死区，fabs(error)小于DeadZone时，输出为0。
//      * @param I_SeparThresh 积分分离阈值，fabs(error)大于该阈值取消积分作用。
//      * @param D_of_Current 是否启用微分先行
//      * @param Imcreatement_of_Out true:输出增量模式，false:输出位置模式。
//      * @param LowPass_error 误差低通滤波器截止频率
//      * @param LowPass_d_err 微分项低通滤波器截止频率,不完全微分，1为不启用
//      * @retval None
//     */
//     void PID_Mode_Init( float LowPass_error, float LowPass_d_err, bool D_of_Current, bool Imcreatement_of_Out)
//     {
//         this->D_of_Current = D_of_Current;
//         this->Imcreatement_of_Out = Imcreatement_of_Out;
//         this->LowPass_error.Trust = LowPass_error;
//         this->LowPass_d_err.Trust = LowPass_d_err;
//     }

//     float current=0,target=0,Out=0;
//     float DeadZone = 0; 		        /*!< 死区，需为整数，fabs(error)小于DeadZone时，输出为0。 */
//     float I_SeparThresh = 400;          /*!< 积分分离阈值，需为正数。fabs(error)大于该阈值取消积分作用。*/
//     bool D_of_Current = false;          /*!< 启用微分先行，文献中Current多译作Process Variable(PV)。 */
//     bool Imcreatement_of_Out = false;   /*!< 输出增量模式。 */

//     LowPassFilter LowPass_error = LowPassFilter(1);
//     LowPassFilter LowPass_d_err = LowPassFilter(1); /*!< 不完全微分。 */

// private:
//     const uint8_t ID = 0;
//     float error=0; 
//     float Kp = 0, Ki = 0, Kd = 0;
//     float I_Term_Max = 0;               /*<! I项限幅 */
//     float Out_Max = 0;                  /*<! 输出限幅 */

//     float pre_error = 0;        /*!< 上一次的error。 */
//     float eriler_error = 0;     /*!< 上上次的error。 */
//     float pre_Current = 0;      /*!< 上一次的Current。 */
//     float eriler_Current = 0;   /*!< 上上次的Current。 */
//     float integral_e = 0;       /*!< 积分器输入 */
//     float I_Term = 0;			/* 积分器输出 */
//     float P_Term = 0;			/* 比例器输出 */
//     float D_Term = 0;			/* 微分器输出 */
//     float last_out = 0;         /*!< 上一次的输出 */
// };

// #endif
