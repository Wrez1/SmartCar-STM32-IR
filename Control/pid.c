#include "myfile.h"

/************************全局变量***************************/
uint8_t Place_Enable, PWM_Enable=1;
int Speed_Out_L, Speed_Out_R, Place_Out=0;
int sensor_err = 0;

// 必须加回这个标志，配合你的 menu.c 使用
uint8_t Start_Flag = 0; 

/************************PID调节区***************************/
int Basic_Speed=0;    
float Turn_factor=2.0; //转向系数
int Left_Speed, Right_Speed=0;

// 5V 供电专用强力参数
// Kp=600(有力), Ki=0(能跑稳), Kd=30(防抖)
int Speed_PID[3] = {250, 0, 40}; 
float Place_PD[2] = {8.0, 25.0}; // 转向参数

// 软启动变量
static int Current_PWM_L = 0;
static int Current_PWM_R = 0;

void Control()
{
    // 1. 停车逻辑
    if (Start_Flag == 0)
    {
        Motor_SetPWM_L(0);
        Motor_SetPWM_R(0);
        Current_PWM_L = 0;
        Current_PWM_R = 0;
        return;
    }

    // 2. 获取传感器误差 (直接用红外，不融合陀螺仪)
    sensor_err = Error_Calcaulate(); 

    // 3. 转向环
    if(Place_Enable)
    {
        Place_Out=(int)Place_Control(sensor_err, 0, Place_PD);
    }
    
    Element_Process();
    Encoder_Read();
    Different_Speed(); // 计算左右目标速度
    
    // 4. 速度环
    Speed_Out_L = PID_Control(Speed_L, Left_Speed, Speed_PID); 
    Speed_Out_R = PID_Control(Speed_R, Right_Speed, Speed_PID);

    // === 修改死区补偿：单独增强右轮 ===
    
    // 左轮：保持 2500 (如果左轮正常)
    if (Speed_Out_L > 0) Speed_Out_L += 3500;
    else if (Speed_Out_L < 0) Speed_Out_L -= 3500;
    
    // 右轮：动力不足，增加到 3500 (甚至 4000，直到它能跟上左轮)
    if (Speed_Out_R > 0) Speed_Out_R += 3500; // <-- 改这里
    else if (Speed_Out_R < 0) Speed_Out_R -= 3500; // <-- 改这里

   
    // 输出限幅 (现在只限制正向最大值即可，因为负数已经被清零了)
    int Target_L = Min_Max(Speed_Out_L, -4000, 7199); // 下限改为 0
    int Target_R = Min_Max(Speed_Out_R, -4000, 7199); // 下限改为 0
    
    // === 6. 5V 供电核心补丁：软启动 (防止重启) ===
    int Step =800; // 步长越小起步越稳，不容易拉崩电压
    
    // 左轮爬坡
    if (Current_PWM_L < Target_L) Current_PWM_L += Step;
    else if (Current_PWM_L > Target_L) Current_PWM_L -= Step;
    
    // 右轮爬坡
    if (Current_PWM_R < Target_R) Current_PWM_R += Step;
    else if (Current_PWM_R > Target_R) Current_PWM_R -= Step;


    // 7. 最终输出 (关键修正：交叉映射)
    if(PWM_Enable)
    {
        // 你的物理左轮接在 B 通道 -> B 通道由 Motor_SetPWM_R 控制
        // 所以要把“左轮计算值(Current_PWM_L)”发给“右轮控制函数(SetPWM_R)”
        Motor_SetPWM_R(Current_PWM_L); 
        
        // 你的物理右轮接在 A 通道 -> A 通道由 Motor_SetPWM_L 控制
        // 所以要把“右轮计算值(Current_PWM_R)”发给“左轮控制函数(SetPWM_L)”
        Motor_SetPWM_L(Current_PWM_R);
    }
}

/************************位置式转向环PD***************************/
float Place_Control(float NowPoint, float SetPoint, float *TURN_PID) 
{
    static float LastError = 0; 
    float KP, KD; 
    float NowError, Out; 
    NowError = SetPoint - NowPoint; 
    KP = *TURN_PID; 
    KD = *(TURN_PID+1); 
    Out = KP * NowError + KD *(NowError-LastError); 
    LastError = NowError; 
    return Out; 
}

/************************位置式速度环PI***************************/
// pid.c -> PID_Control 函数

// 在文件顶部定义的 Integral 需要改为全局变量或者传入指针，
// 但为了简单，我们在函数内部处理“归零”逻辑。

int PID_Control(int NowPoint, int SetPoint, int *TURN_PID) 
{
    static int Integral, LastError = 0;      
    int KP, KI, KD, Out, NowError; 
    
    KP = *TURN_PID; 
    KI = *(TURN_PID+1); 
    KD = *(TURN_PID+2); 

    // === 关键修复：目标速度为 0 时，清空积分 ===
    // 如果目标是停下 (0)，那就彻底把积分清零，防止“积分饱和”导致的乱跑或不动
    if (SetPoint == 0)
    {
        Integral = 0;
        LastError = 0;
        return 0; // 直接返回 0
    }

    NowError = SetPoint - NowPoint;
    
    Integral += NowError;
    
    // 积分抗饱和 (稍微减小限幅，防止积分过大导致反应迟钝)
    // 5V 供电下，积分不需要太大，给 800 够了
    Integral = Min_Max(Integral, -800, 800); 
    
    Out = KP * NowError + KI * Integral + KD * (NowError - LastError);
    
    LastError = NowError; 
    return Out;
}

/************************差速计算***************************/
void Different_Speed() 
{ 
    float k;  
    
    // 计算 k 值
    float abs_k = (Place_Out > 0 ? Place_Out : -Place_Out) * 0.01;
    
    // 限制 k 的上限 (比如限制在 8.0)
    if (abs_k > 8.0) abs_k = 8.0; 
    
    k = abs_k;

    // ★ 关键参数调整 ★
    // 内侧轮系数：调小！让它减速变慢，不容易进入负数区域
    float Inner_Factor = 0.6; 

    // 外侧轮系数：调大！主要靠外轮加速来推头
    float Outer_Factor = 4.0;

    // ★ 新增：最大反转限制 (Anti-Spin Limit) ★
    // 限制内侧轮最慢只能转到基础速度的 -30%
    // 比如基础速度 80，最慢只能是 -24。这样就不会“猛拽”车头
    float Min_Scale = -0.3f; 

    if (Place_Out >= 0) // 车偏左
    {
        // === 左轮(内侧)处理 ===
        float left_scale = 1.0f - k * Inner_Factor;
        
        // 强制限制：如果反转太狠，就锁死在 -0.3
        if (left_scale < Min_Scale) left_scale = Min_Scale;
        
        Left_Speed = Basic_Speed * left_scale;

        // === 右轮(外侧)处理 ===
        Right_Speed = Basic_Speed * (1.0f + k * Outer_Factor);
    } 
    else // 车偏右
    { 
        // === 左轮(外侧)处理 ===
        Left_Speed = Basic_Speed * (1.0f + k * Outer_Factor); 

        // === 右轮(内侧)处理 ===
        float right_scale = 1.0f - k * Inner_Factor;
        
        // 强制限制
        if (right_scale < Min_Scale) right_scale = Min_Scale;
        
        Right_Speed = Basic_Speed * right_scale;
    } 
}