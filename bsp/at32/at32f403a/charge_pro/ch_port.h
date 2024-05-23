/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#ifndef _CH_PORT_H_
#define _CH_PORT_H_

#include <stdint.h>
#include <time.h>
#include <data_def.h>


typedef struct
{
    /* 1:����     2:ֹͣ ���������� */
    uint8_t   ucCtl;
    /* 0:app����  1:ˢ������ */
    uint8_t   ucChStartMode;
    /* 0:�Զ����� 1:������� 2:��ʱ���� 3:��������� */
    uint8_t   ucChMode;
    uint32_t  uiStopParam;
} CH_CTL_T;

enum ch_start_failed_reason
{
    START_SUCCESS = 0,
    DEVICE_ = 0x4100,
    NO_GUN_DETECTED,
    SYSTEM_ERROR = 0x4201,
    SERVIE_PAUSED,
    METER_OFFLINE,
};

typedef enum
{
    NORMAL = 0,				/* 0������ */
    R_POWER_DOWN,			/* 1: ��⵽���� */
    LEAKAGE_FAULT,			/* 2��©����� */
    SPD_FAULT,				/* 3�����׹��� */
    OTHER_FAULT,            /* 4: �Ŵ� */
    DOOR_OPEN,				/* 5: �Ŵ� */
    EM_STOP,				/* 6: ��ͣ���� */
    OVER_VOLT,				/* 7����ѹ */
    OVER_CURRENT,			/* 8������ */
    READY_TIMEOUT,			/* 9����������ʱ,9V->6Vʧ�� */
    CP_LINK_ABN,			/* 10��cp�����쳣 */
    UNPLUG,				    /* 11���Ƿ���ǹ */
    END_CONDITION,			/* 12��������������� */
    NO_CURRENT,				/* 13���޵��� */
    E_END_BY_APP,			/* 14��app���� */
    E_END_BY_CARD,			/* 15��ˢ������ */
    UNDER_VOLT,				/* 16��Ƿѹ */
	STOP_CHARGEFULL,		/* 17:����*/
    STOP_OTHEN,				/* 18������ */
	  E_END_APP_BALANC, /*19 app����*/
	  STOP_MAX,         /*19:���*/
} STOP_REASON;				//ֹͣԭ��

typedef struct
{
    uint16_t  usMsec;
    uint8_t   ucMin : 6;
    uint8_t   ucRes0: 1;
    uint8_t   ucIV  : 1;
    uint8_t   ucHour:5;
    uint8_t   ucRes1:2;
    uint8_t   ucSU  :1;
    uint8_t   ucDay: 5;
    uint8_t   ucWeek:3;
    uint8_t   ucMon: 4;
    uint8_t   ucRes2:4;
    uint8_t   ucYear:7;
    uint8_t   ucRes3:1;
} CP56TIME2A_T;




typedef struct
{
    /* 1:����    2:ֹͣ ���������� */
    uint8_t   ucCtl;
    /* 0:app���� 1:ˢ������ ���������� */
    uint8_t   ucChStartMode;
    /* 0:�ɹ�    1:ʧ�� ���������� */
    uint8_t   ucResult;
    /* ��Э�� */
    uint8_t   ucResultCode;
} CHCTL_ACK_T;


void localtime_to_cp56time(time_t tTime,CP56TIME2A_T *pstCp56);

void send_up_chinfo_result(uint8_t ucResult);


//uint8_t get_ch_rate(RATE_T *pstRate);
//uint8_t save_ch_info(CHINFO_T *pstChInfo);

//uint8_t up_ch_info(CHINFO_T *pstChInfo);

#endif

