/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-10-23
 * @version  V1.0.0
 *
 * @Description  ��Ϣ���з���
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include <string.h>
#include "common.h"

/*
===============================================================================================================================
                                            ****** ���Ͷ���/�궨�� ******
===============================================================================================================================
*/

#define        MQ_SERVICE_MSG_MAX_SIZE     sizeof(MQ_MSG_T)
#define        MQ_SERVICE_MSG_MAX_NUM      8

typedef struct
{
    uint32_t       uiModuleId;          /* ����ģ��ID */
    rt_mq_t        pMQ;                 /* ��Ϣ���� */
    const char    *sMqName;             /* ��Ϣ�������� */
} MQ_ID_MAPP_T;

/* ��Ϣ�빦��ģ��IDӳ��ڵ� */
typedef struct _mq_id_mapp_node
{

    /* ָ����һ���ڵ� */
    struct _mq_id_mapp_node   *pstNext;
    /* ��Ϣidӳ�� */
    MQ_ID_MAPP_T               stMqMapp;
} MQ_ID_MAPP_NODE;

/* ��Ϣ�빦��ģ��IDӳ����Ч���� */
typedef struct
{
    /* ͷ�ڵ� */
    MQ_ID_MAPP_NODE    *pstHead;
    /* β�ڵ� */
    MQ_ID_MAPP_NODE    *pstTail;
} MQ_ID_MAPP_LIST;

/*
===============================================================================================================================
                                            ****** �������� ******
===============================================================================================================================
*/

MQ_ID_MAPP_LIST      staMqMappList = {0};
/*
===============================================================================================================================
                                            ****** ��������/���� ******
===============================================================================================================================
*/

/**
 * @brief ����ģ��id�Ƿ��Ѿ�����
 * @param[in] uiModuleId:����ģ��ID
 * @param[out]
 * @return RT_NULL:����ģ��IDû�а� �ǿ�:����ģ��ID��Ӧ�Ľڵ�ָ��
 * @note
 */
MQ_ID_MAPP_NODE  * mq_service_moduleid_exist(uint32_t uiModuleId)
{
    MQ_ID_MAPP_NODE  *pstNextNode = RT_NULL;

    pstNextNode = staMqMappList.pstHead;

    while(pstNextNode != RT_NULL)
    {
        /* �Ѿ�����  */
        if(pstNextNode->stMqMapp.uiModuleId == uiModuleId)
        {
            return pstNextNode;
        }

        pstNextNode = pstNextNode->pstNext;
    }

    return RT_NULL;
}

/**
 * @brief  ����ģ��id����Ϣ���а�
 * @param[in]
 * @param[out]
 * @return 0 �󶨳ɹ�
 * @note
 */
uint8_t mq_service_bind(uint32_t  uiModuleId,const char *sMqName)
{
    MQ_ID_MAPP_NODE  *pstNode = RT_NULL;
    rt_mq_t          pMq      = RT_NULL;

    /* ģ��ID�Ƿ��Ѿ����� */
    if(mq_service_moduleid_exist(uiModuleId) != RT_NULL)
    {
        rt_kprintf("module id:%d already bound\r\n",uiModuleId);
        return 1;
    }

    /* ����ڵ� */
    pstNode = rt_malloc(sizeof(MQ_ID_MAPP_NODE));
    if(pstNode == RT_NULL)
    {
        rt_kprintf("Failed to allocate node when module ID is bound,module id:%d\r\n",uiModuleId);
        return 2;
    }

    /* ������Ϣ���� */
    pMq = rt_mq_create(sMqName, MQ_SERVICE_MSG_MAX_SIZE, MQ_SERVICE_MSG_MAX_NUM, RT_IPC_FLAG_FIFO);
    if(pMq == RT_NULL)
    {
        rt_free(pstNode);
        rt_kprintf("Failed to allocate mq when module ID is bound,module id:%d\r\n",uiModuleId);
        return 3;
    }

    pstNode->pstNext              = RT_NULL;
    pstNode->stMqMapp.uiModuleId  = uiModuleId;
    pstNode->stMqMapp.sMqName     = sMqName;
    pstNode->stMqMapp.pMQ         = pMq;

    rt_enter_critical();

    /* �����ﻹû����Ч�󶨵Ľڵ� */
    if(staMqMappList.pstHead == RT_NULL)
    {
        /* ֱ�����ӵ�ͷ�ڵ� */
        staMqMappList.pstHead = pstNode;
        staMqMappList.pstTail = pstNode;
        rt_exit_critical();
        return 0;
    }

    /* �������Ѿ��нڵ��ˣ�β�ڵ�Ϊ�����������쳣 */
    if(staMqMappList.pstTail == RT_NULL)
    {
        rt_free(pstNode);
        rt_mq_delete(pMq);
        rt_kprintf("The linked list is abnormal when the module ID is bound ,module id:%d\r\n",uiModuleId);
        rt_exit_critical();
        return 4;
    }

    /* ���ӵ�β�ڵ����� */
    staMqMappList.pstTail->pstNext = pstNode;
    /* β�ڵ�ָ��ǰ�ڵ� */
    staMqMappList.pstTail  = pstNode;
    rt_exit_critical();

    return 0;
}

/**
 * @brief ������Ϣ��ָ���Ĺ���ģ��
 * @param[in]  uiSrcMoudleId  ����ϢԴͷ�Ĺ���ģ��
 *             uiDestMoudleId ����Ҫ���͵��Ĺ���ģ��
 *             uiMsgCode      ����Ϣ��
 *             uiMsgVar       ����Ϣ����
 *             uiLoadLen      ����Ϣ��Ч�غɳ���
 *             pucLoad        ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0: ���ͳɹ�  ��0: ����ʧ��
 * @note �ڲ�����
 */
uint8_t mq_service_send_msg(uint32_t uiSrcMoudleId, uint32_t  uiDestMoudleId, uint32_t  uiMsgCode,
                            uint32_t uiMsgVar, uint32_t  uiLoadLen, uint8_t  *pucLoad)
{
    MQ_ID_MAPP_NODE  *pstNode = RT_NULL;
    MQ_MSG_T         stMsg    = {0};
    uint8_t          *pucBuf  = RT_NULL;
    rt_err_t         rtErrT   = RT_EOK;

    pstNode = mq_service_moduleid_exist(uiDestMoudleId);

    /* Ŀ�깦��ģ�黹δ����Ϣ���� */
    if(pstNode == RT_NULL)
    {
        rt_kprintf("Message sending failed, the target module ID has not been bound to the message queue\r\n");
        return 1;
    }

    if((pucLoad != RT_NULL) && (uiLoadLen > 0))
    {
        pucBuf = rt_malloc(uiLoadLen);

        if(pucBuf == RT_NULL)
        {
            rt_kprintf("Message sending failed, failed to allocate buffer \r\n");
            return 2;
        }
        memcpy(pucBuf,pucLoad,uiLoadLen);
    }

    stMsg.uiSrcMoudleId    = uiSrcMoudleId;
    stMsg.uiDestMoudleId   = uiDestMoudleId;
    stMsg.uiMsgCode        = uiMsgCode;
    stMsg.uiMsgVar         = uiMsgVar;
    stMsg.uiLoadLen        = uiLoadLen;
    stMsg.pucLoad          = pucBuf;

    rtErrT = rt_mq_send(pstNode->stMqMapp.pMQ,&stMsg,sizeof(MQ_MSG_T));

    /* ���ò���ϵͳ��Ϣ������Ϣ����ʧ�� */
    if(rtErrT != RT_EOK)
    {
        if(pucBuf != RT_NULL)
        {
            rt_free(pucBuf);
        }
        rt_kprintf("uiSrcMoudleId: %d send msg to uiDestMoudleId : %d failed,\
		   call the os mq to send the message API to feedback the sending failure,err code:%d\r\n",uiSrcMoudleId,uiDestMoudleId,rtErrT) ;

        return 3;
    }
    return 0;
}
/**
 * @brief   ָ���Ĺ���ģ�������Ϣ
 * @param[in]  uiSrcMoudleId ����Ҫ������ϢԴͷ�Ĺ���ģ��
 *             uiBufSize     ������buf�ĳߴ�
 *             rtTimeout     ���ȴ���ʱ��ʱ��
 * @param[out] pstMsg �� ָ��洢���յ�����Ϣ������
*              pucMsgBuf��ָ��洢���յ���Ϣ����Ч�غ�
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_recv_msg(uint32_t uiSrcMoudleId,MQ_MSG_T *pstMsg,uint8_t *pucMsgBuf,uint32_t uiBufSize,rt_int32_t rtTimeout)
{
    MQ_ID_MAPP_NODE  *pstNode = RT_NULL;
    uint32_t          uiRxValidLen = 0;

    RT_ASSERT(pstMsg    != RT_NULL);
    RT_ASSERT(pucMsgBuf != RT_NULL);
    RT_ASSERT(uiBufSize > 0);

    pstNode = mq_service_moduleid_exist(uiSrcMoudleId);
    if(pstNode == RT_NULL)                                         	/* �ù���ģ�黹δ����Ϣ���� */
    {
        rt_kprintf("Message sending failed, the target module ID has not been bound to the message queue\r\n");
        return 1;
    }

    if (rt_mq_recv(pstNode->stMqMapp.pMQ, pstMsg, sizeof(MQ_MSG_T) , rtTimeout) == RT_EOK)          /*������Ϣ */
    {
        if(pstMsg->pucLoad != RT_NULL)        /* ����Ч�غɣ���Ҫcopy��Ӧ�ò��ṩ�Ļ�����,��Ϊ��Ϣ����ʹ�ö�̬�ڴ���䣬��Ҫ�ͷ� */
        {
            uiRxValidLen  = CM_DATA_GET_MIN(uiBufSize,pstMsg->uiLoadLen);

            memcpy(pucMsgBuf,pstMsg->pucLoad,uiRxValidLen);
            rt_free(pstMsg->pucLoad);

            pstMsg->pucLoad    = pucMsgBuf;
            pstMsg->uiLoadLen  = uiRxValidLen;
        }

        return 0;
    }

    return 2;
}
/**
 * @brief ����ģ���ڲ�������Ϣ������ģ���Լ�
 * @param[in]  uiMoudleId ������ģ��ID
 *             uiMsgCode  ����Ϣ��
 *             uiMsgVar   ����Ϣ����
 *             uiLoadLen  ����Ϣ��Ч�غɳ���
 *             pucLoad    ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_xxx_send_msg_to_xxx(uint32_t uiMoudleId, uint32_t  uiMsgCode,
                                       uint32_t  uiMsgVar, uint32_t  uiLoadLen, uint8_t  *pucLoad)
{
    return mq_service_send_msg(uiMoudleId,uiMoudleId,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad);
}

/**
 * @brief   ������Ϣ���������ģ��
 * @param[in]  uiSrcMoudleId ����ϢԴͷ�Ĺ���ģ��
 *             uiMsgCode     ����Ϣ��
 *             uiMsgVar      ����Ϣ����
 *             uiLoadLen     ����Ϣ��Ч�غɳ���
 *             pucLoad       ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
* @return 0:���ͳɹ�  ��0:����ʧ��
 * @note
 */
uint8_t mq_service_xxx_send_msg_to_chtask(uint32_t uiSrcMoudleId, uint32_t  uiMsgCode,
        uint32_t uiMsgVar, uint32_t uiLoadLen, uint8_t  *pucLoad)
{
    return mq_service_send_msg(uiSrcMoudleId,CM_CHTASK_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad);
}



/**
 * @brief   ˢ����������Ϣ����ʾ����
 * @param[in]  uiMsgCode ����Ϣ��
 *             uiMsgVar  ����Ϣ����
 *             uiLoadLen ����Ϣ��Ч�غɳ���
 *             pucLoad   ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_card_send_disp(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_CARD_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}



/**
* @brief   ���ģ�鷢�͸���ʾ����
 * @param[in]  uiMsgCode ��ֹͣԭ��
 *             uiMsgVar  ����Ϣ����
 *             uiLoadLen ����Ϣ��Ч�غɳ���
 *             pucLoad   ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_ch_send_dip(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_CHTASK_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}


/**
* @brief   ���͸�4G��������
 * @param[in]  uiMsgCode ����Ϣ��
 *             uiMsgVar  ����Ϣ����
 *             uiLoadLen ����Ϣ��Ч�غɳ���
 *             pucLoad   ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_send_to_4gsend(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_UNDEFINE_ID,CM_4GSEND_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}

/**
* @brief   ���Ľ��������͵���ʾ����
 * @param[in]  uiMsgCode ����Ϣ��
 *             uiMsgVar  ����Ϣ����
 *             uiLoadLen ����Ϣ��Ч�غɳ���
 *             pucLoad   ��ָ����Ҫ���͵���Ч�غɵ�ַ
 * @param[out]
 * @return 0 �����ͳɹ�  ��0������ʧ��
 * @note
 */
uint8_t mq_service_dwinrecv_send_disp(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_DWINRECV_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}
