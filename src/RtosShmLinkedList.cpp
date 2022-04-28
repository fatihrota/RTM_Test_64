/*
 * RtosShmLinkedList.cpp
 *
 *  Created on: Apr 26, 2022
 *      Author: fatih
 */

#include "RtosShmLinkedList.h"
#include <RtosLib.h>

#define SHMDEMO_SHMNAME             "UserShm"

int shmTest()
{
	UINT32  dwRetVal;
	UINT32  dwRes;
	UINT32  dwShmId;
	UINT32  dwShmSizeTotal;

	dwRes = RtosGetIdByName(SHMDEMO_SHMNAME, RTOS_ID_SHM, &dwShmId);
	if (RTE_SUCCESS != dwRes)
	{
		printf("Error (0x%X) querying  shared memory id. Please ensure a shared memory with this name is configured for read and write by any OS.\r\n", dwRes);
		dwRetVal = dwRes;
		return -1;
	}
	printf("Ok - id %u\r\n", dwShmId);

	dwRes = RtosShmTotalSizeGet(dwShmId, &dwShmSizeTotal);
	if (RTE_SUCCESS != dwRes)
	{
		printf("Error (0x%X)\r\n", dwRes);
		dwRetVal = dwRes;
		return -1;
	}
	printf("Ok - %u (0x%X) bytes\r\n", dwShmSizeTotal, dwShmSizeTotal);

	UINT32  dwShmSizeRequested;
	UINT32  dwShmSizeGranted;
	VOID*   pvShmAddr = NULL;

	volatile uint32_t *dataptr = NULL;
	uint32_t data;
	dwShmSizeRequested = sizeof(data);

	dwRes = RtosShmAddrGet(dwShmId, dwShmSizeRequested, &dwShmSizeGranted, &pvShmAddr);
	if (RTE_SUCCESS != dwRes)
	{
		printf("Error (0x%X) requesting %u bytes\r\n",
			dwRes, dwShmSizeRequested);
		dwRetVal = dwRes;
		return -1;
	}
	printf("OK - granted %u bytes\r\n", dwShmSizeGranted);

	dataptr = (uint32_t*)pvShmAddr;
	//*dataptr = 88;
	printf("DATA : %d\n", *dataptr);
}

int list_in_shm_init(list_in_shm_handle_t * h,uint32_t size,uint32_t num,uint8_t init )
{
	UINT32  dwRes;
    uint64_t totalSize=0;
    uint32_t i =0;
    uint8_t *temp = NULL ;

    totalSize=(size * num)+sizeof(sm_list_t)+sizeof(sm_list_t);

    dwRes = RtosGetIdByName( SHMDEMO_SHMNAME, RTOS_ID_SHM, &h->shmid );
    //h->shmid = shmget(h->key,totalSize,IPC_CREAT);
    if (RTE_SUCCESS != dwRes)
    {
    	printf("\nRtosGetIdByName failed errono\n");
    	return -1;
    }

    uint32_t sizeReturned  = 0;
    h->size=totalSize;
    printf("TotalSize : %d\n", h->size);
    dwRes = RtosShmAddrGet( h->shmid, h->size, &sizeReturned, &h->ptr );
    printf("sizeReturned : %d\n", sizeReturned);
    //h->ptr=(uint8_t *)shmat(h->shmid,NULL,0);
    if (RTE_SUCCESS != dwRes)
    {
    	printf("\RtosShmAddrGet failed errono\n");
    	return -1;
    }


    h->pFreeListStruct=(sm_list_t*)(h->ptr);
    h->pList=(sm_list_t*)((uint8_t*)(h->pFreeListStruct)+sizeof(sm_list_t));
    h->pStart=(uint8_t*)((uint8_t*)(h->pList)+sizeof(sm_list_t));

    if(init)
    {
         memset(h->ptr,0,totalSize);
         temp=h->pStart;
         for(i=0;i<num;i++)
         {
            list_in_shm_insert_node(h->pFreeListStruct,(uint8_t *)h->ptr,(node_sm_t *)(temp+i*size),FROM_FRONT);
         }
    }
    return 0 ;
}

int list_in_shm_insert_node(sm_list_t * plst, uint8_t * basePtr, node_sm_t * node, uint8_t front)
{
	node_sm_t* temp = NULL;

	if (NULL == plst || NULL == basePtr || NULL == node)
	{
		printf("list_in_shm_insert_node plst is null");
		return -1;
	}


	if (0 == plst->p_off && 0 == plst->n_off)
	{
		plst->p_off = GET_OFFSET(basePtr, node);
		plst->n_off = GET_OFFSET(basePtr, node);
		plst->c = 1;
		return 0;
	}

	if (FROM_FRONT & front)
	{
		temp = (node_sm_t*)(GET_PTR(basePtr, plst->n_off));
		node->n_off = plst->n_off;
		temp->p_off = GET_OFFSET(basePtr, node);
		node->p_off = 0;
		plst->n_off = GET_OFFSET(basePtr, node);
		(plst->c)++;
	}
	else
	{
		temp = (node_sm_t*)(GET_PTR(basePtr, plst->p_off));
		temp->n_off = GET_OFFSET(basePtr, node);
		node->p_off = plst->p_off;
		node->n_off = 0;
		plst->p_off = GET_OFFSET(basePtr, node);
		(plst->c)++;
	}
	return 0;
}

node_sm_t *list_in_shm_get_node(sm_list_t * plst, uint8_t * basePtr, uint8_t front)
{
	node_sm_t * temp = NULL;
	node_sm_t * temp2 = NULL;
	uint32_t offtemp = 0;
	if (NULL == plst || NULL == basePtr || (0 == plst->c))
	{
		printf("list_in_shm_get_node plst null ");
		return NULL;
	}

	if (1 == plst->c)
	{
		temp = (node_sm_t *)(GET_PTR(basePtr, plst->n_off));
		plst->c = 0;
		plst->n_off = 0;
		plst->p_off = 0;
		return temp;
	}
	if (FROM_FRONT & front)
	{
		temp = (node_sm_t *)(GET_PTR(basePtr, plst->n_off));
		offtemp = temp->n_off;
		plst->n_off = offtemp;
		temp2 = (node_sm_t *)(GET_PTR(basePtr, plst->n_off));
		temp2->p_off = 0;
		(plst->c)--;
	}
	else
	{
		temp = (node_sm_t *)(GET_PTR(basePtr, plst->p_off));
		offtemp = temp->p_off;
		plst->p_off = offtemp;
		temp2 = (node_sm_t *)(GET_PTR(basePtr, offtemp));
		temp2->n_off = 0;
		(plst->c)--;
	}

	return temp;
}

node_sm_t * get_node_from_shm(list_in_shm_handle_t * h, uint8_t flags)
{
	sm_list_t *lst = NULL;
	node_sm_t * temp = NULL;
	int ret = 0;
	if (NULL == h)
	{
		printf("Get node from shm h is NULL\n");
		return NULL;
	}

	if (FROM_FREE_LIST & flags)
	{
		lst = h->pFreeListStruct;
	}
	else
	{
		lst = h->pList;
	}
	if (0 == lst || 0 == lst->c)
	{
		printf("Get node from shm lst is NULL\n");
		return NULL;
	}
	temp = list_in_shm_get_node(lst, (uint8_t *)h->ptr, flags);
	return temp;
}

int put_node_in_list(list_in_shm_handle_t * h, node_sm_t *node, uint8_t flags)
{
	sm_list_t *lst = NULL;
	int ret = 0;
	if (NULL == h || NULL == node)
	{
		printf("put_node_in_list node is null");
		return -1;
	}

	if (FROM_FREE_LIST & flags)
	{
		lst = h->pFreeListStruct;
	}
	else
	{
		lst = h->pList;
	}

	ret = list_in_shm_insert_node(lst, (uint8_t *)h->ptr, node, flags);
	return 0;
}

void list_in_shm_finish(list_in_shm_handle_t * h)
{

}


