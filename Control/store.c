#include "stm32f10x.h"
#include "myfile.h"
#include "Store.h" 

// STM32F103C8T6 (64KB) 最后一页地址: 0x0800FC00
#define FLASH_SAVE_ADDR  0x0800FC00 
#define STORE_FLAG       0xA5A5      

extern int Speed_PID[3];   
extern float Place_PD[2];  
extern int Speed_Choice[3];

// ★★★ 修复关键：在此处添加函数声明（告诉编译器有这个函数） ★★★
uint16_t FLASH_ReadHalfWord(uint32_t Address);

void Store_Init(void)
{
    // 检查 Flash 里的标志位
    if (FLASH_ReadHalfWord(FLASH_SAVE_ADDR) != STORE_FLAG)
    {
        // 第一次使用，Flash 里没有数据
        OLED_ShowString(0, 0, "No Save Data", OLED_8X16);
        OLED_Update();
        Delay_ms(500); 
        OLED_Clear();
        return; 
    }
    
    // 读取数据
    Speed_PID[0] = FLASH_ReadHalfWord(FLASH_SAVE_ADDR + 2);
    Speed_PID[2] = FLASH_ReadHalfWord(FLASH_SAVE_ADDR + 4);
    Place_PD[0] = (float)FLASH_ReadHalfWord(FLASH_SAVE_ADDR + 6) / 100.0f;
    Place_PD[1] = (float)FLASH_ReadHalfWord(FLASH_SAVE_ADDR + 8) / 100.0f;
    Speed_Choice[0] = FLASH_ReadHalfWord(FLASH_SAVE_ADDR + 10);
    
    // 显示读取成功
    OLED_ShowString(0, 0, "Data Loaded!", OLED_8X16);
    OLED_Update();
    Delay_ms(500);
    OLED_Clear();
}

void Store_Save(void)
{
    // 1. 解锁
    FLASH_Unlock();
    
    // 2. 清除标志位
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
    
    // 3. 擦除页
    FLASH_Status status = FLASH_ErasePage(FLASH_SAVE_ADDR);
    if(status != FLASH_COMPLETE)
    {
        OLED_Clear();
        OLED_ShowString(0, 0, "Erase Error!", OLED_8X16);
        OLED_Update();
        while(1); 
    }
    
    // 4. 写入数据
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR, STORE_FLAG);
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 2, (uint16_t)Speed_PID[0]);
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 4, (uint16_t)Speed_PID[2]);
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 6, (uint16_t)(Place_PD[0] * 100));
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 8, (uint16_t)(Place_PD[1] * 100));
    FLASH_ProgramHalfWord(FLASH_SAVE_ADDR + 10, (uint16_t)Speed_Choice[0]);
    
    // 5. 上锁
    FLASH_Lock();
}

// 读取函数定义
uint16_t FLASH_ReadHalfWord(uint32_t Address)
{
    return *(__IO uint16_t*)Address;
}