#include "myfile.h"
#include "sensor.h"

/************************传感器误差计算***************************/
// 全局变量
int err = 0;

/* **************** 计算传感器偏差 **************** */
int Error_Calcaulate()
{		
    int final_error = 0;
    
    // === 优先级逻辑 (Priority Logic) ===
    // 只要最外侧触发，误差就是最大的，完全忽略内侧传感器的状态
    // 我们直接返回放大后的整数值 (Scale Factor = 10 已经包含在数值里了)
    
    // 1. 最危急情况：最外侧 (L2 或 R2) 检测到黑线 (0)
    if (L2 == 0) 
    {
        final_error = -80; // 极度偏左，给最大负误差 (原-7 * 10)
        // 甚至可以给更狠一点，比如 -80，强制反转
    }
    else if (R2 == 0)
    {
        final_error = 80;  // 极度偏右，给最大正误差
    }
    
    // 2. 次危急情况：次外侧 (L1 或 R1) 检测到黑线
    else if (L1 == 0)
    {
        final_error = -12; // 稍微偏左
    }
    else if (R1 == 0)
    {
        final_error = 12;  // 稍微偏右
    }
    
    // 3. 正常情况：中间 (M) 检测到黑线
    else if (M == 0)
    {
        final_error = 0;  // 居中
    }
    
    // 4. 特殊情况：全白 (丢线)
    // 保持上一次的误差，利用 err 全局变量锁死方向
    else 
    {
        final_error = err; 
    }

    // 更新全局变量
    err = final_error;
    
    return final_error;           
}

/* **************** 直角放大偏差 **************** */
// 既然采用了优先级逻辑，这个函数已经没用了
// 为了兼容你的 pid.c 调用，保留它但返回 1.0
float Right_err()
{
    return 1.0f; 
}