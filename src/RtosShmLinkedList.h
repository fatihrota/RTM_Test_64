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


typedef struct sm_list_s
{
  uint32_t n_off;
  uint32_t p_off;
  uint32_t c ;
  uint32_t mc;
} sm_list_t;


typedef struct node_sm_s
{
    uint32_t p_off ;
    uint32_t n_off ;
}node_sm_t ;

typedef struct list_in_shm_handle_s {
   uint32_t shmid;
   uint64_t size;
   uint32_t  dwOwner;
   void * ptr ;
   sm_list_t *pFreeListStruct;
   sm_list_t *pList;
   uint8_t *pStart;

}list_in_shm_handle_t;


#define GET_OFFSET(baseptr,ptr) ((uint8_t *)ptr-(uint8_t *)baseptr)
#define GET_PTR(baseptr,offset) ((uint8_t *)baseptr+offset)
#define FROM_FREE_LIST 0x01
#define FROM_LIST      0x02
#define FROM_FRONT     0x10
#define FROM_BACK      0x20

int shmTest();
int list_in_shm_init(list_in_shm_handle_t * h, uint32_t size,uint32_t num,uint8_t init );
int list_in_shm_insert_node(sm_list_t * plst, uint8_t * basePtr , node_sm_t * node ,uint8_t front);
node_sm_t *list_in_shm_get_node(sm_list_t * plst,uint8_t * basePtr,uint8_t front);
node_sm_t * get_node_from_shm(list_in_shm_handle_t * h,uint8_t flags );
int put_node_in_list(list_in_shm_handle_t * h,node_sm_t *node,uint8_t flags);
void list_in_shm_finish(list_in_shm_handle_t * h);
