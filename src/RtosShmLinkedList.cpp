/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosShmLinkedList.cpp
 * @brief       Implementation of the RtosShmLinkedList class of the library.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-04-28
 *
 * @ingroup     RtosShmLinkedList
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/

#ifndef pEcLogParms
#define pEcLogParms G_pEcLogParms
#endif
#include "RtosShmLinkedList.h"
#include "EcLogging.h"
//#include <RtosLib.h>

/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define SHMDEMO_SHMNAME             "UserShm"

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Module global data                                                         */
/*============================================================================*/

/*============================================================================*/
/* Implementation of functions                                                */
/*============================================================================*/

INT32 RtosShmLinkedList::shmTest()
{
	UINT32  dwRes;
	UINT32  dwShmId;
	UINT32  dwShmSizeTotal;

	dwRes = RtosGetIdByName(SHMDEMO_SHMNAME, RTOS_ID_SHM, &dwShmId);
	if (RTE_SUCCESS != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error (0x%X) querying  shared memory id. Please ensure "
				"a shared memory with this name is configured for read and write by any OS.\r\n", dwRes));
		return dwRes;
	}
	EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Ok - id %u\r\n", dwShmId));

	dwRes = RtosShmTotalSizeGet(dwShmId, &dwShmSizeTotal);
	if (RTE_SUCCESS != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error (0x%X)\r\n", dwRes));
		return dwRes;
	}
	EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Ok - %u (0x%X) bytes\r\n", dwShmSizeTotal, dwShmSizeTotal));

	UINT32  dwShmSizeRequested;
	UINT32  dwShmSizeGranted;
	VOID*   pvShmAddr = NULL;

	volatile UINT32 *dataptr = NULL;
	UINT32 data;
	dwShmSizeRequested = sizeof(data);

	dwRes = RtosShmAddrGet(dwShmId, dwShmSizeRequested, &dwShmSizeGranted, &pvShmAddr);
	if (RTE_SUCCESS != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error (0x%X) requesting %u bytes\r\n",
				dwRes, dwShmSizeRequested));
		return dwRes;
	}
	EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "OK - granted %u bytes\r\n", dwShmSizeGranted));

	dataptr = (UINT32*)pvShmAddr;
	EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DATA : %d\n", *dataptr));
}

/******************************************************************************/
INT32 RtosShmLinkedList::listShm_Init(listShm * listPtr, UINT32 size, UINT32 num, UINT8 init)
{
	UINT32  dwRes;
	UINT64 totalSize=0;
	UINT32 i =0;
	UINT8 *temp = NULL ;

    totalSize=(size * num)+sizeof(linkedListShm)+sizeof(linkedListShm);

    dwRes = RtosGetIdByName( SHMDEMO_SHMNAME, RTOS_ID_SHM, &listPtr->shmid );
    if (RTE_SUCCESS != dwRes)
    {
    	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error (0x%X) querying  shared memory id. Please ensure "
    					"a shared memory with this name is configured for read and write by any OS.\r\n", dwRes));
    	return dwRes;
    }

    UINT32 sizeReturned  = 0;
    listPtr->size=totalSize;

    printf("!!!!!!!TotalSize : %d\n", listPtr->size);
    EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "TotalSize : %d\n", listPtr->size));

    dwRes = RtosShmAddrGet( listPtr->shmid, listPtr->size, &sizeReturned, &listPtr->ptr );
    if (RTE_SUCCESS != dwRes)
    {
    	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "RtosShmAddrGet failed errono\n"));

    	return -1;
    }

    EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "sizeReturned : %d\n", sizeReturned));


    listPtr->pFreeListStruct=(linkedListShm*)(listPtr->ptr);
    listPtr->pList=(linkedListShm*)((UINT8*)(listPtr->pFreeListStruct)+sizeof(linkedListShm));
    listPtr->pStart=(UINT8*)((UINT8*)(listPtr->pList)+sizeof(linkedListShm));

    if(init)
    {
         memset(listPtr->ptr,0,totalSize);
         temp=listPtr->pStart;
         for(i=0;i<num;i++)
         {
        	 listShm_InsertNode(listPtr->pFreeListStruct,(UINT8 *)listPtr->ptr,(nodeShm *)(temp+i*size),FROM_FRONT);
         }
    }
    return 0 ;
}

/******************************************************************************/
int RtosShmLinkedList::listShm_InsertNode(linkedListShm * plst, UINT8 * basePtr, nodeShm * node, UINT8 front)
{
	nodeShm* temp = NULL;

	if (NULL == plst || NULL == basePtr || NULL == node)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "listShm_InsertNode linkedListShm is null or nodeShm is null\n"));
		return -1;
	}


	if (0 == plst->prevOff && 0 == plst->nextOff)
	{
		plst->prevOff = GET_OFFSET(basePtr, node);
		plst->nextOff = GET_OFFSET(basePtr, node);
		plst->count = 1;
		return 0;
	}

	if (FROM_FRONT & front)
	{
		temp = (nodeShm*)(GET_PTR(basePtr, plst->nextOff));
		node->nextOff = plst->nextOff;
		temp->prevOff = GET_OFFSET(basePtr, node);
		node->prevOff = 0;
		plst->nextOff = GET_OFFSET(basePtr, node);
		(plst->count)++;
	}
	else
	{
		temp = (nodeShm*)(GET_PTR(basePtr, plst->prevOff));
		temp->nextOff = GET_OFFSET(basePtr, node);
		node->prevOff = plst->prevOff;
		node->nextOff = 0;
		plst->prevOff = GET_OFFSET(basePtr, node);
		(plst->count)++;
	}
	return 0;
}

/******************************************************************************/
RtosShmLinkedList::nodeShm* RtosShmLinkedList::listShm_GetNode(linkedListShm * plst, UINT8 * basePtr, UINT8 front)
{
	nodeShm * temp = NULL;
	nodeShm * temp2 = NULL;
	UINT32 offtemp = 0;
	if (NULL == plst || NULL == basePtr || (0 == plst->count))
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "listShm_GetNode linkedListShm is null\n"));
		return NULL;
	}

	if (1 == plst->count)
	{
		temp = (nodeShm *)(GET_PTR(basePtr, plst->nextOff));
		plst->count = 0;
		plst->nextOff = 0;
		plst->prevOff = 0;
		return temp;
	}
	if (FROM_FRONT & front)
	{
		temp = (nodeShm *)(GET_PTR(basePtr, plst->nextOff));
		offtemp = temp->nextOff;
		plst->nextOff = offtemp;
		temp2 = (nodeShm *)(GET_PTR(basePtr, plst->nextOff));
		temp2->prevOff = 0;
		(plst->count)--;
	}
	else
	{
		temp = (nodeShm *)(GET_PTR(basePtr, plst->prevOff));
		offtemp = temp->prevOff;
		plst->prevOff = offtemp;
		temp2 = (nodeShm *)(GET_PTR(basePtr, offtemp));
		temp2->nextOff = 0;
		(plst->count)--;
	}

	return temp;
}

/******************************************************************************/
RtosShmLinkedList::nodeShm* RtosShmLinkedList::getNodeFromShm(listShm * listPtr, UINT8 flags)
{
	linkedListShm *lst = NULL;
	nodeShm * temp = NULL;
	INT32 ret = 0;
	if (NULL == listPtr)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "getNodeFromShm listShm is NULL\n"));
		return NULL;
	}

	if (FROM_FREE_LIST & flags)
	{
		lst = listPtr->pFreeListStruct;
	}
	else
	{
		lst = listPtr->pList;
	}
	if (0 == lst || 0 == lst->count)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "getNodeFromShm linkedListShm is NULL\n"));
		return NULL;
	}
	temp = listShm_GetNode(lst, (UINT8 *)listPtr->ptr, flags);
	return temp;
}

/******************************************************************************/
INT32 RtosShmLinkedList::putNodeToShm(listShm * listPtr, nodeShm *node, UINT8 flags)
{
	linkedListShm *lst = NULL;
	int ret = 0;
	if (NULL == listPtr || NULL == node)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "putNodeToShm listShm is NULL\n"));
		return -1;
	}

	if (FROM_FREE_LIST & flags)
	{
		lst = listPtr->pFreeListStruct;
	}
	else
	{
		lst = listPtr->pList;
	}

	ret = listShm_InsertNode(lst, (UINT8 *)listPtr->ptr, node, flags);
	return 0;
}

/******************************************************************************/
void RtosShmLinkedList::listShm_Finish(listShm * listPtr)
{

}

/**@}*/
/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/


