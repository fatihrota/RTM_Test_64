#pragma once
/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosShmLinkedList.h
 * @brief       RtosShmLinkedList is class to create and manage double linked list
 * 				on shared memory.
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

#include <sys/types.h>
#include <sys/ipc.h>
#include "stdint.h"
#include "errno.h"
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "stdio.h"
#include <string.h>
#include <RtosLib.h>

/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define GET_OFFSET(baseptr,ptr) ((uint8_t *)ptr-(uint8_t *)baseptr)
#define GET_PTR(baseptr,offset) ((uint8_t *)baseptr+offset)
#define FROM_FREE_LIST 0x01
#define FROM_LIST      0x02
#define FROM_FRONT     0x10
#define FROM_BACK      0x20


/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Declarations                                                               */
/*============================================================================*/


class RtosShmLinkedList
{
public:
	RtosShmLinkedList(){};
	~RtosShmLinkedList(){};

	typedef struct
	{
		UINT32 nextOff;
		UINT32 prevOff;
		UINT32 count ;
	}linkedListShm;

	typedef struct
	{
		UINT32 prevOff;
		UINT32 nextOff;
	}nodeShm;

	typedef struct
	{
		UINT32 shmid;
		UINT64 size;
		UINT32  dwOwner;
	    VOID* ptr ;
	    linkedListShm *pFreeListStruct;
	    linkedListShm *pList;
	    UINT8 *pStart;

	}listShm;

	listShm defaultList;

	INT32 shmTest();
	INT32 listShm_Init(listShm * listPtr, UINT32 size, UINT32 num, UINT8 init);
	INT32 listShm_InsertNode(linkedListShm * plst, UINT8 * basePtr, nodeShm * node, UINT8 front);
	nodeShm* listShm_GetNode(linkedListShm * plst, UINT8 * basePtr, UINT8 front);
	nodeShm* getNodeFromShm(listShm * listPtr, UINT8 flags);
	INT32 putNodeToShm(listShm * listPtr, nodeShm *node, UINT8 flags);
	void listShm_Finish(listShm * listPtr);

private:

};


/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
