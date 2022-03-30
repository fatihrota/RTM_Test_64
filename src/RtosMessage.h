#pragma once
/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RtosMessage.h
 * @brief       The RTOS Message class that holds different message types and
 *              structures.
 *
 * @author      Mehmet Fatih Özay
 * @date        2022-03-30
 *
 * @ingroup     RtosMessage
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <rtosLib.h>
/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define NRTM_MESSAGE_TOTAL_SIZE		8000
#define NRTM_MSG_SIZE				NRTM_MESSAGE_TOTAL_SIZE - sizeof(msg_header)/* - sizeof(msg_footer)*/

#define NONE	0

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Declarations                                                               */
/*============================================================================*/

class RtosMessage
{
public:
	RtosMessage() {};
	~RtosMessage() {};

	typedef enum
	{
		HELLO = 1,
		HELLOACK,
		TESTINFO
	} cmdType;

	typedef struct
	{
		UINT8 startByte;	/* 0x03*/
		UINT8 nrtmID;		/* Non Real Time ID (could be n)*/
		UINT8 rtmID;		/* Real Time ID (could be n)*/
		UINT8 testID;		/* Message Related Test ID (could be n, start from 1, NONE is irrelevant)*/
		UINT8 ccID;			/* Message Related CC ID (could be n, start from 1, NONE is irrelevant)*/
		UINT8 msgID;		/* Randomly generated message ID, it will be used for ACK check*/
		UINT8 isACK;		/* NONE is ACK not required
							   1 = Message is ACK
							   2 = ACK required message */
		UINT8 isFragment;	/* NONE message is fit to single transaction, 1 is fragmented*/
		UINT8 fragmentNo;	/* Fragment no if message is fragmented*/
		UINT16 cmd;			/* Type of message */
		UINT16 dataLen;		/* Length of the payload(msg) */
		UINT64 timestamp;	/* Timestamp of a message (Epoch time in miliseconds)*/
	} msg_header;

	typedef struct
	{
		UINT16 crc;
		UINT8 endByte;
		UINT8 reserved;
	}msg_footer;

	typedef struct
	{
		msg_header header;
		UINT8 *msg;			/* Message in JSON format */
		//msg_footer footer;
	} rtosMsg;

	INT32 createHeader(msg_header *hdr, UINT8 nrtmID, UINT8 rtmID, UINT8 testID, UINT8 ccID,
		UINT8 isACK, UINT8 isFragment, UINT8 fragmentNo, cmdType cmd);


	INT32 createHelloAckMsg(UINT8 *dataArray, INT32 *datalen);
	INT32 parseReceivedMsg(void *msgData);
	INT32 createTestInfoMsg(void);
	INT32 parseHello(void *msgData, INT32 datalen);
	INT32 parseTestInfo(void *msgData, INT32 datalen);

private:
};

/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/

