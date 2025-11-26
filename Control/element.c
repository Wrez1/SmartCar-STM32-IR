#include "myfile.h"

/************************元素***************************/

//变量定义
uint8_t Element_Flag=0;
uint8_t Ten_Flag=0;
uint8_t Ring_Flag=0;
uint8_t Noline_Flag=0;
uint8_t Stop_Flag = 0;
uint8_t K=0;

// [0]:弯道/基础速度, [1]:停车, [2]:直道冲刺速度
// 建议: 80, 0, 120 (差值大一点效果才明显)
int Speed_Choice[3]={50,0,80}; 

// 直道判断计数器
float Straight_Distance = 0; 

/************************元素控制台***************************/
void Element_Process()
{
    // 只有在没有处理特殊元素时，才进行直道/弯道速度切换
    if(Element_Flag==0)
    {
        Element_Normal();   // 基础控制
       // Element_Straight(); // 直道加速判断 (新加的)
    }
    
    //Element_Ring();     // 圆环
    Element_Noline();   // 丢线处理
    Element_Stop();     // 停车
    Element_Ten();      // 十字识别
}

// 基础模式
void Element_Normal()
{
    if(Element_Flag==0)
    {
        Place_Enable=1;
        PWM_Enable=1;
        // 默认给低速 (弯道安全速度)
        // 如果满足直道条件，Element_Straight 会覆盖这个值
        Basic_Speed=Speed_Choice[0]; 
    }
}

// ★★★ 新增：直道加速逻辑 ★★★
/*void Element_Straight()
{
    // 判断条件：中间灯亮(0)，两边灯灭(1)，且误差很小
    // 这意味着车子正对直道
    if( M==0 && L1==1 && R1==1 && (sensor_err > -2 && sensor_err < 2) )
    {
        // 累加直线行驶的里程 (Location 是累加的，我们需要一个差值，这里简化用累加计数)
        // 更好的做法是在这里自增一个距离变量，利用 Encoder_Read 里的增量
        // 这里为了简单，假设每次循环车走了一点点，累计次数
        Straight_Distance += 0.5f; // 这是一个虚拟计数，根据实际调试调整
    }
    else
    {
        // 一旦遇到弯道（误差变大），立即清零计数，并降速
        Straight_Distance = 0;
    }

    // 如果保持直行了一段距离 (比如计数超过 20)，认为进入长直道
    if(Straight_Distance > 20)
    {
        // 切换到高速档
        Basic_Speed = Speed_Choice[2]; 
        
        // 限制计数器防溢出
        if(Straight_Distance > 100) Straight_Distance = 100;
    }
}
*/

// 圆环 (保持原样)
void Element_Ring()
{
    // ... (你原来的代码，未修改) ...
    if(Element_Flag==0&&Ring_Flag==0&&Noline_Flag==4&&Ten_Flag==1)
	{
			if(R2==1&&M==1&&(L1==0||R1==0)&&L2==0)
		{		
				Ring_Flag=1;	
				Clear_Location();			
		}
	}
	if(Ring_Flag==1&&Location>=10.5)
	{		
		if((L1==0&&M==1&&R1==0)||((L1==1||R1==1)&&M==1))
		{
			Ring_Flag=2;
			Clear_Location();	
			Element_Flag=1;	
			Place_Enable=0;
			Place_Out=-15; 
		}
		else{Ring_Flag=0;}
	}
	if(Ring_Flag==2&&Location>30)
	{
		Place_Enable=1;
		Ring_Flag=3;
		Basic_Speed=Speed_Choice[2]; // 入环后可以用高速
	}
	if(Ring_Flag==3&&Location>380)
	{
		if(L2==1&&M==1&&R2==0)
		{
			Clear_Location();	
			Place_Enable=0;
			Place_Out=-15;	
			Ring_Flag=4;			
		}
	}
	if(Ring_Flag==4&&Location>50)
	{
		Element_Flag=0;
		K=1;
		Ring_Flag=5;
		Clear_Location();
	}
}

// ★★★ 修复：丢线逻辑 (改为全白) ★★★
void Element_Noline()
{
    if (Element_Flag == 0 && Noline_Flag == 0)
    {
        // 修正：丢线是跑到白地上，所以应该是全 1 (高电平)
        if (L2 == 1 && L1 == 1 && M == 1 && R1 == 1 && R2 == 1) 
        { 	  
            Noline_Flag = 1;
            Clear_Location(); 
        }  								
    }
    
    if(Noline_Flag==1)	
    {
        // 修正：确认全白
        if(L2==1 && L1==1 && M==1 && R1==1 && R2==1)
        {
            if(Location > 19)
            {
                Element_Flag=2;
                Place_Enable=0;							
                Place_Out= -100; // 大幅度转向找线
                Noline_Flag=2;	
                Clear_Location();
            }		
        }
        else
        {
            Clear_Location();	
            Noline_Flag=0;
        }	
    }
    
    // ... (后面的 Noline 逻辑保持不变) ...
    if(Noline_Flag==2&&Location>25)
	{
		Noline_Flag=3;
		PWM_Enable=0;
		Motor_SetPWM_L(6000);
		Motor_SetPWM_R(6000);  	
	}	
	// === 修改为：自动复位 ===
	if(Noline_Flag==3 && Location>=50 && (L2==0||L1==0||M==0||R1==0||R2==0))
	{
		Noline_Flag = 4; // 先标记为4 (配合十字路口的检测条件)
		Element_Flag = 0;
		Clear_Location();	
	}

	// 新增：在 FN=4 之后，再跑一小段距离（比如 10cm），就彻底重置为 0
	// 这样车子就有能力应对下一次丢线了
	if (Noline_Flag == 4 && Location > 10.0f)
	{
		Noline_Flag = 0; // 复活！可以再次检测丢线了
	}
}

// 停车 (保持原样，注意不要和丢线搞混)
void Element_Stop()
{
    // 只有在跑完圆环后才开启停车检测，防止误判
    if (Element_Flag == 0 && Stop_Flag == 0 && Ring_Flag == 5) 
    {
        // 停车线如果是黑色横线，应该是全 0
        // 停车区如果是白色，则是全 1
        // 这里假设终点是全白区
        if (L2 == 1 && L1 == 1 && M == 1 && R1 == 1 && R2 == 1)
        { 
            Stop_Flag = 1;  
            Clear_Location();						
        }
    }
    if (Stop_Flag == 1 && Location >= 30) 
    {
        Place_Enable=0;
        Basic_Speed=Speed_Choice[1]; // 速度 0
        //PWM_Enable = 0; // 彻底关断 PWM
        //Motor_SetPWM_L(0);
       // Motor_SetPWM_R(0);
    }
}

// 十字路口 (使用我之前给你的修正版)
void Element_Ten()
{
    if(Element_Flag==0 && Ring_Flag==0 && Ten_Flag==0)
    {
        // 全黑检测 (0)
        if (L2 == 0 && L1 == 0 && M == 0 && R1 == 0 && R2 == 0)
        {
            Ten_Flag = 1;
            Element_Flag = 1;
            Place_Enable = 0;
            Place_Out = 0;
            Clear_Location();
        }
    }

    if(Ten_Flag == 1)
    {
        // 闭眼直行 15cm
        if (Location > 15.0f) 
        {
            Ten_Flag = 2;
            Element_Flag = 0;
            Place_Enable = 1;
            Clear_Location();
        }
    }
    
    if (Ten_Flag == 2)
    {
        if (Location > 20.0f) Ten_Flag = 0; 
    }
}