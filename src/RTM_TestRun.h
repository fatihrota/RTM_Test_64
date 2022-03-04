#pragma once
/******************************************************************************
 *
 *   Copyright (C) Rota Teknik 2019 All Rights Reserved. Confidential
 *
 **************************************************************************//**
 * @file        RTM_TestRun.h
 * @brief       The TestRun class
 *
 * @author      Mehmet Fatih Ozay
 * @date        2022-02-25
 *
 * @ingroup     RTM_TestRun
 * @{
 *****************************************************************************/

 /*============================================================================*/
 /* Includes                                                                   */
 /*============================================================================*/
#include <iostream>
#include <stdint.h>
#include <vector>
#include "EcDemoApp.h"
#include <rtosLib.h>
#include "RTM_CompactController.h"

/*============================================================================*/
/* Forward declarations                                                       */
/*============================================================================*/

/*============================================================================*/
/* Constants and macros                                                       */
/*============================================================================*/

#define MAX_SHM_SIZE	200

/*============================================================================*/
/* Type definitions                                                           */
/*============================================================================*/

/*============================================================================*/
/* Global data                                                                */
/*============================================================================*/

/*============================================================================*/
/* Declarations                                                               */
/*============================================================================*/

class RTM_TestRun
{
public:
	RTM_TestRun();
	~RTM_TestRun();

	EC_T_VOID*	testEvent;
	EC_T_INT 	testId;

	EC_T_VOID parseReceivedEthercatBuffer(EC_T_VOID);
	EC_T_VOID commandPlayer(EC_T_VOID);
	EC_T_VOID parseReceivedCommand(EC_T_VOID);

	EC_T_VOID createProcess(EC_T_VOID);
	EC_T_VOID trigerrProcess(EC_T_VOID);
	EC_T_VOID waitForProcess(EC_T_VOID);

	sem_t *sem_id_w;
	sem_t *sem_id_r;
	char semName_w[20];
	char semName_r[20];

	pid_t pid;
	key_t key;
	int shmid;
	void* shm;

	std::vector<RTM_CompactController> ccVector;	/* Holds CC information */
private:
};

/******************************************************************************/
/*   Copyright (C) Rota Teknik 2021,  All Rights Reserved. Confidential.      */
/******************************************************************************/
