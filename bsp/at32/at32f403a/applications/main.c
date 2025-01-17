/*******************************************************************************
 *          Copyright (c) 2020-2050, wanzhuangwulian Co., Ltd.
 *                              All Right Reserved.
 * @file
 * @note
 * @brief
 *
 * @author
 * @date
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <time.h>
#include <board.h>
#include <fal.h>
#include <string.h>
#include "user_lib.h"
#include "easyflash.h"
#include "user_lib.h"
#include "ch_port.h"
#include "mfrc522.h"
#include "w25qxx.h"
#include "4GMain.h"


extern int wdt_sample();

extern _m1_card_info m1_card_info;

SYSTEM_RTCTIME gs_SysTime;

CP56TIME2A_T gsCP56Time;

/* 用于获取RTC毫秒 */
int32_t gsi_RTC_Counts;

uint32_t gui_RTC_millisecond;

//extern S_DEVICE_CFG gs_DevCfg;
//extern S_APP_CHARGE gs_AppCharge;
extern long list_mem(void);

/* UNIQUE_ID[31: 0] */
uint32_t Unique_ID1;
/* UNIQUE_ID[63:32] */
uint32_t Unique_ID2;
/* UNIQUE_ID[95:63] */
uint32_t Unique_ID3;


uint32_t test = 0;

typedef void (*rt_fota_app_func)(void);
static rt_fota_app_func app_func = RT_NULL;
#define USER_APP_START_ADD 0x08032000//开始地址  
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		:
* Description	: 跳转到boot
* Input			:
* Output		:
* Note(s)		:
* Contributor	:
***********************************************************************************************/
void GotoUserApplication(void)
{   /* 这个函数在GD提供的库中没有真正开启起来，在这里没有起到关全局中断的作用 */
    __disable_irq();

    for (IRQn_Type irq = WWDT_IRQn; irq <= UART8_IRQn; irq++)
    {
        nvic_irq_disable(irq);
    }
    //rcu_deinit();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    //用户代码区第二个字为程序开始地址(复位地址)
    app_func = (rt_fota_app_func)*(__IO uint32_t *)(USER_APP_START_ADD + 4);

    /* Configure main stack */
    __set_MSP(*(__IO uint32_t *)USER_APP_START_ADD);
    //重定向中断向量表
    SCB->VTOR = USER_APP_START_ADD;

    /* jump to application */
    app_func();

}

/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
int main(void)
{
    uint8_t i=0;
    rt_device_t device;

    struct tm *Net_time;

    time_t time_now = 0;

    uint32_t timeout = rt_tick_get();

    uint32_t mem_timeout = rt_tick_get();

    device = rt_device_find("rtc");

    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }

    RT_ASSERT(fal_init() > 0);

    //rt_kprintf("The current version of APP firmware is %s\n", FIRMWARE_VERSION);
//    if(rt_memcmp((const char *)&(item_info_node->uc_SN[2]),(const char *)ucDev_Sn,6)==0)

    /* 获取MCU唯一ID */
    Unique_ID1 = *(uint32_t *)(0x1FFFF7E8);
    Unique_ID1 = SW32(Unique_ID1);
    Unique_ID2 = *(uint32_t *)(0x1FFFF7EC);
    Unique_ID2 = SW32(Unique_ID2);
    Unique_ID3 = *(uint32_t *)(0x1FFFF7F0);
    Unique_ID3 = SW32(Unique_ID3);

    rt_kprintf("MCU Id:%08x %08x %08x\n",Unique_ID1,Unique_ID2,Unique_ID3);

    memcpy(&m1_card_info.MCUID[0],&Unique_ID1, 4);
    memcpy(&m1_card_info.MCUID[4],&Unique_ID2, 4);
    memcpy(&m1_card_info.MCUID[8],&Unique_ID3, 4);
    for(i = 0; i < 12; i++)    //字节开始为0，对比函数就不比较了，所以让字节0  赋值成aa 就是可对比
    {
        if(m1_card_info.MCUID[i]  == 0x00)
        {
            m1_card_info.MCUID[i] = 0xaa;
        }
    }


#if(doguser)
    wdt_sample();  //看门狗程序20s   喂狗函数在空闲线程里面       //===测试远程升级暂时关闭，后期开启
#endif
	
	
    while (1)
    {
        /* 每隔1000个滴答(1000ms),更新一下gs_SysTime,使其保持为最新值 */
        if(rt_tick_get()>=(100+timeout))
        {
            timeout = rt_tick_get();
            rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time_now);

            //time_now = time_now + 8*60*60;
            Net_time = localtime(&time_now);
            gs_SysTime.ucYear = Net_time->tm_year;
            gs_SysTime.ucMonth = (Net_time->tm_mon)+1;
            gs_SysTime.ucDay = Net_time->tm_mday;
            gs_SysTime.ucWeek = Net_time->tm_wday;
            gs_SysTime.ucHour = Net_time->tm_hour;
            gs_SysTime.ucMin = Net_time->tm_min;
            gs_SysTime.ucSec = Net_time->tm_sec;

            //list_thread();  //查看线程栈使用率
            localtime_to_cp56time((time_now+28800), &gsCP56Time);

//			rt_kprintf("NTP Time:%04d-%02d-%02d-%02d %02d:%02d:%02d\r\n",(Net_time->tm_year)+1900,\
//                    (Net_time->tm_mon)+1, Net_time->tm_mday, Net_time->tm_wday, Net_time->tm_hour,Net_time->tm_min,gs_SysTime.ucSec);
        }

		
		//系统时间复位到1970年后，  主要是更改时间，防止存储记录查询不准确
        static uint8_t  NUM = 1;
        if((gs_SysTime.ucYear == 70) && (NUM==1))
        {
            gs_SysTime.ucYear = 123;  //这是年，123标示是23年
            set_time(8, 0, 0);
            set_date(2023, 11, 6);
            NUM = 0;
        }

		
	
		
//        /* 每隔30秒,查询一下内存信息 */
//		if(rt_tick_get()>=(30000+mem_timeout))
//		{
//			mem_timeout = rt_tick_get();
//			list_mem();
//		}

        rt_thread_mdelay(100);
    }
}
