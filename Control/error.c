#include "myfile.h"
#include "sensor.h"

/************************传感器误差计算***************************/
// 新增结构体和宏定义
#define SENSOR_NUM 5				// 传感器数量
#define SCALE_FACTOR 10				// 缩放因子

typedef struct   
{
    float position;  // 传感器位置坐标（线性分布）
    float channel;   // 传感器对应的通道号
} SensorMap;

/* **************** 全局变量 **************** */
int err = 0;

// 根据传感器物理位置定义坐标
// 负值代表左边，正值代表右边
static const SensorMap sensorMap[SENSOR_NUM] =   
{
    {-2.8, 1}, // L2 (最左) -> 对应误差负大
    {-1.3, 2}, // L1
    { 0, 3},   // M  (中间) -> 对应误差 0
    { 1.3, 4}, // R1
    { 2.8, 5}  // R2 (最右) -> 对应误差正大
};

/* **************** 计算传感器偏差 **************** */
int Error_Calcaulate()
{		
	int active_sum = 0;
	int active_count = 0;
	
	for (int i = 0; i < SENSOR_NUM; i++) 		
	{
        // === 关键修改 ===
        // 你的传感器是黑线低电平(0)，白线高电平(1)
        // 所以这里必须判断 "== 0" 才是检测到了线
        // 注意：这里加了 (int) 强制类型转换，因为你的结构体里 channel 是 float
		if (digital((int)sensorMap[i].channel) == 0)  		
		{
			active_sum += sensorMap[i].position; // 累加激活传感器的位置
			active_count++; 					 // 累加激活数量
		}
    }
	
    // 计算平均误差
	err = active_count ? (active_sum * SCALE_FACTOR) / active_count : 0;
	
    // 加上直角放大系数 (如果你不需要直角特殊处理，可以把这行注释掉)
    // err = err * Right_err(); 
	
	return err;           
}

/* **************** 直角放大偏差 **************** */
// 如果你的赛道有直角弯，这个函数可以保留
// 如果只是普通弯道，建议先让它返回 1.0，避免干扰 PID
float Right_err()
{
    // 逻辑检查：
    // R2=0 (右边检测到黑线), 此时应该向右剧烈转弯
    if(R2==0 && M==1 && L1==1 && L2==1)
    {
        return 1.5f; // 建议先改小一点，27.0 太大了，容易让车直接飞出去
    }
    // L2=0 (左边检测到黑线), 此时应该向左剧烈转弯
    else if(L2==0 && M==1 && R1==1 && R2==1)
    {
        return 1.5f; 
    }
    else 
    {
        return 1.0f; // 正常情况不放大
    }
}