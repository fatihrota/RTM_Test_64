/*-----------------------------------------------------------------------------
 * EcDemoApp.cpp
 * Copyright                acontis technologies GmbH, Ravensburg, Germany
 * Response                 Holger Oelhaf
 * Description              EC-Master demo application
 *---------------------------------------------------------------------------*/

/*-INCLUDES------------------------------------------------------------------*/
#include "EcDemoApp.h"
#include "DemoInline.h"

#define myApp
#ifdef myApp
#include "RTM_MainApp.h"
EC_T_VOID mainRun(EC_T_VOID* pvAppContext);
#endif

/*-DEFINES-------------------------------------------------------------------*/
//#define DCM_ENABLE_LOGFILE

#define JOB_ProcessAllRxFrames  0
#define JOB_SendAllCycFrames    1
#define JOB_MasterTimer         2
#define JOB_SendAcycFrames      3
#define JOB_Total               4
#define PERF_CycleTime          5
#define PERF_myAppWorkpd        6
#define PERF_DCM_Logfile        7
#define MAX_JOB_NUM             8

#define PERF_JOB_TOTAL  { if (pAppContext->TscMeasDesc.bMeasEnabled) { \
                              EC_T_TSC_TIME* pTscTime = &pAppContext->TscMeasDesc.aTscTime[JOB_Total]; \
                              if (pTscTime->bMeasReset) { pTscTime->dwCurr = 0; pTscTime->dwAvg = 0; pTscTime->dwMin = (EC_T_DWORD)ULONG_MAX; pTscTime->dwMax = 0; pTscTime->bMeasReset = EC_FALSE; } \
                              else { \
                                  pTscTime->dwCurr =  pAppContext->TscMeasDesc.aTscTime[JOB_ProcessAllRxFrames].dwCurr + pAppContext->TscMeasDesc.aTscTime[JOB_SendAllCycFrames].dwCurr \
                                                    + pAppContext->TscMeasDesc.aTscTime[JOB_MasterTimer].dwCurr + pAppContext->TscMeasDesc.aTscTime[JOB_SendAcycFrames].dwCurr; \
                                  pTscTime->dwAvg =  pAppContext->TscMeasDesc.aTscTime[JOB_ProcessAllRxFrames].dwAvg + pAppContext->TscMeasDesc.aTscTime[JOB_SendAllCycFrames].dwAvg \
                                                    + pAppContext->TscMeasDesc.aTscTime[JOB_MasterTimer].dwAvg + pAppContext->TscMeasDesc.aTscTime[JOB_SendAcycFrames].dwAvg; \
                                  pTscTime->dwMax = EC_MAX(pTscTime->dwMax, pTscTime->dwCurr); \
                                  pTscTime->dwMin = EC_MIN(pTscTime->dwMin, pTscTime->dwCurr); }}}


/*-LOCAL VARIABLES-----------------------------------------------------------*/
static EC_T_CHAR* S_aszMeasInfo[MAX_JOB_NUM] =
{
	(EC_T_CHAR*)"JOB_ProcessAllRxFrames",
	(EC_T_CHAR*)"JOB_SendAllCycFrames  ",
	(EC_T_CHAR*)"JOB_MasterTimer       ",
	(EC_T_CHAR*)"JOB_SendAcycFrames    ",
	(EC_T_CHAR*)"JOB_Total             ",
	(EC_T_CHAR*)"Cycle Time            ",
	(EC_T_CHAR*)"myAppWorkPd           ",
	(EC_T_CHAR*)"Write DCM logfile     "
};

/*-FUNCTION DECLARATIONS-----------------------------------------------------*/
static EC_T_VOID  EcMasterJobTask(EC_T_VOID* pvAppContext);
static EC_T_DWORD EcMasterNotifyCallback(EC_T_DWORD dwCode, EC_T_NOTIFYPARMS* pParms);
#if (defined ATEMRAS_SERVER)
static EC_T_DWORD RasNotifyCallback(EC_T_DWORD dwCode, EC_T_NOTIFYPARMS* pParms);
#endif

/*-MYAPP---------------------------------------------------------------------*/
typedef struct _T_MY_APP_DESC
{
	EC_T_DWORD dwFlashPdBitSize; /* Size of process data memory */
	EC_T_DWORD dwFlashPdBitOffs; /* Process data offset of data */
	EC_T_DWORD dwFlashTimer;
	EC_T_DWORD dwFlashInterval;
	EC_T_BYTE* pbyFlashBuf; /* flash buffer */
	EC_T_BYTE  byFlashVal; /* flash pattern */
	EC_T_DWORD dwFlashBufSize; /* flash buffer */
} T_MY_APP_DESC;
static EC_T_DWORD myAppInit(T_EC_DEMO_APP_CONTEXT* pAppContext);
static EC_T_DWORD myAppPrepare(T_EC_DEMO_APP_CONTEXT* pAppContext);
static EC_T_DWORD myAppSetup(T_EC_DEMO_APP_CONTEXT* pAppContext);
static EC_T_DWORD myAppWorkpd(T_EC_DEMO_APP_CONTEXT* pAppContext);
static EC_T_DWORD myAppDiagnosis(T_EC_DEMO_APP_CONTEXT* pAppContext);
static EC_T_DWORD myAppNotify(EC_T_DWORD dwCode, EC_T_NOTIFYPARMS* pParms);

/*-FUNCTION DEFINITIONS------------------------------------------------------*/

/********************************************************************************/
/** \brief EC-Master demo application.
*
* This is an EC-Master demo application.
*
* \return  Status value.
*/
EC_T_DWORD EcMasterApp(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	EC_T_DWORD             dwRetVal          = EC_E_NOERROR;
	EC_T_DWORD             dwRes             = EC_E_NOERROR;

	T_EC_DEMO_APP_PARMS*   pAppParms         = &pAppContext->AppParms;

	EC_T_VOID*             pvJobTaskHandle   = EC_NULL;

	EC_T_REGISTERRESULTS   RegisterClientResults;
	OsMemset(&RegisterClientResults, 0, sizeof(EC_T_REGISTERRESULTS));

	CEcTimer               oAppDuration;
	EC_T_BOOL              bFirstDcmStatus   = EC_TRUE;
	CEcTimer               oDcmStatusTimer;

#if (defined ATEMRAS_SERVER)
	EC_T_BOOL              bRasServerStarted = EC_FALSE;
	EC_T_VOID*             pvRasServerHandle = EC_NULL;
#endif

#if (defined INCLUDE_PCAP_RECORDER)
	CPcapRecorder*         pPcapRecorder     = EC_NULL;
#endif
	/* check link layer parameter */
	if (EC_NULL == pAppParms->apLinkParms[0])
	{
		dwRetVal = EC_E_INVALIDPARM;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: Missing link layer parameter\n"));
		goto Exit;
	}

	/* check if polling mode is selected */
	if (pAppParms->apLinkParms[0]->eLinkMode != EcLinkMode_POLLING)
	{
		dwRetVal = EC_E_INVALIDPARM;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: Link layer in 'interrupt' mode is not supported by EcMasterDemo. Please select 'polling' mode.\n"));
		goto Exit;
	}

	pAppContext->pNotificationHandler = EC_NEW(CEmNotification(pAppContext->dwInstanceId));
	if (EC_NULL == pAppContext->pNotificationHandler)
	{
		dwRetVal = EC_E_NOMEMORY;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: Cannot create notification handler\n"));
		goto Exit;
	}

	dwRes = pAppContext->pNotificationHandler->InitInstance(pEcLogParms);
	if (EC_E_NOERROR != dwRes)
	{
		dwRetVal = dwRes;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: Cannot initialize notification handler\n"));
		goto Exit;
	}

	pAppContext->pMyAppDesc = (T_MY_APP_DESC*)OsMalloc(sizeof(T_MY_APP_DESC));
	if (EC_NULL == pAppContext->pMyAppDesc)
	{
		dwRetVal = EC_E_NOMEMORY;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: Cannot create myApp descriptor\n"));
		goto Exit;
	}
	OsMemset(pAppContext->pMyAppDesc, 0, sizeof(T_MY_APP_DESC));

	dwRes = myAppInit(pAppContext);
	if (EC_E_NOERROR != dwRes)
	{
		dwRetVal = dwRes;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "myAppInit failed: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		goto Exit;
	}

	/* initalize performance measurement */
	if (pAppParms->bPerfMeasEnabled)
	{
		PERF_MEASURE_JOBS_INIT(MAX_JOB_NUM);
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "OsMeasGet100kHzFrequency(): %d MHz\n", OsMeasGet100kHzFrequency() / 10));
		pAppContext->TscMeasDesc.bMeasEnabled = EC_TRUE;
	}
#ifdef ATEMRAS_SERVER
	/* start RAS server if enabled */
	if (0 != pAppParms->wRasServerPort)
	{
		ATEMRAS_T_SRVPARMS oRemoteApiConfig;

		OsMemset(&oRemoteApiConfig, 0, sizeof(ATEMRAS_T_SRVPARMS));
		oRemoteApiConfig.dwSignature        = ATEMRASSRV_SIGNATURE;
		oRemoteApiConfig.dwSize             = sizeof(ATEMRAS_T_SRVPARMS);
		oRemoteApiConfig.oAddr.dwAddr       = 0; /* INADDR_ANY */
		oRemoteApiConfig.wPort              = pAppParms->wRasServerPort;
		oRemoteApiConfig.dwCycleTime        = ATEMRAS_CYCLE_TIME;
		oRemoteApiConfig.dwWDTOLimit        = (ATEMRAS_MAX_WATCHDOG_TIMEOUT / ATEMRAS_CYCLE_TIME);
		oRemoteApiConfig.cpuAffinityMask    = pAppParms->CpuSet;
		oRemoteApiConfig.dwMasterPrio       = MAIN_THREAD_PRIO;
		oRemoteApiConfig.dwClientPrio       = MAIN_THREAD_PRIO;
		oRemoteApiConfig.pvNotifCtxt        = pAppContext->pNotificationHandler; /* Notification context */
		oRemoteApiConfig.pfNotification     = RasNotifyCallback; /* Notification function for emras Layer */
		oRemoteApiConfig.dwConcNotifyAmount = 100; /* for the first pre-allocate 100 Notification spaces */
		oRemoteApiConfig.dwMbxNotifyAmount  = 50; /* for the first pre-allocate 50 Notification spaces */
		oRemoteApiConfig.dwMbxUsrNotifySize = 3000; /* 3K user space for Mailbox Notifications */
		oRemoteApiConfig.dwCycErrInterval   = 500; /* span between to consecutive cyclic notifications of same type */
		if (1 <= pAppParms->nVerbose)
		{
			OsMemcpy(&oRemoteApiConfig.LogParms, &pAppContext->LogParms, sizeof(EC_T_LOG_PARMS));
			oRemoteApiConfig.LogParms.dwLogLevel = EC_LOG_LEVEL_ERROR;
		}
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Start Remote API Server now\n"));
		dwRes = emRasSrvStart(&oRemoteApiConfig, &pvRasServerHandle);
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: Cannot spawn Remote API Server\n"));
		}
		bRasServerStarted = EC_TRUE;
	}
#endif

	/* initialize EtherCAT master */
	{
		EC_T_INIT_MASTER_PARMS oInitParms;

		OsMemset(&oInitParms, 0, sizeof(EC_T_INIT_MASTER_PARMS));
		oInitParms.dwSignature                   = ATECAT_SIGNATURE;
		oInitParms.dwSize                        = sizeof(EC_T_INIT_MASTER_PARMS);
		oInitParms.pOsParms                      = &pAppParms->Os;
		oInitParms.pLinkParms                    = pAppParms->apLinkParms[0];
		oInitParms.pLinkParmsRed                 = pAppParms->apLinkParms[1];
		oInitParms.dwBusCycleTimeUsec            = pAppParms->dwBusCycleTimeUsec;
		oInitParms.dwMaxBusSlaves                = MASTER_CFG_ECAT_MAX_BUS_SLAVES;
		oInitParms.dwMaxAcycFramesQueued         = MASTER_CFG_MAX_ACYC_FRAMES_QUEUED;
		if (oInitParms.dwBusCycleTimeUsec >= 1000)
		{
			oInitParms.dwMaxAcycBytesPerCycle    = MASTER_CFG_MAX_ACYC_BYTES_PER_CYC;
		}
		else
		{
			oInitParms.dwMaxAcycBytesPerCycle    = 1500;
			oInitParms.dwMaxAcycFramesPerCycle   = 1;
			oInitParms.dwMaxAcycCmdsPerCycle     = 20;
		}
		oInitParms.dwEcatCmdMaxRetries           = MASTER_CFG_MAX_ACYC_CMD_RETRIES;

		OsMemcpy(&oInitParms.LogParms, &pAppContext->LogParms, sizeof(EC_T_LOG_PARMS));
		oInitParms.LogParms.dwLogLevel = pAppParms->dwMasterLogLevel;

		dwRes = ecatInitMaster(&oInitParms);
		if (dwRes != EC_E_NOERROR)
		{
			dwRetVal = dwRes;
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot initialize EtherCAT-Master: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
			goto Exit;
		}
		if (0 != OsStrlen(pAppParms->szLicenseKey))
		{
			dwRes = ecatSetLicenseKey(pAppParms->szLicenseKey);
			if (dwRes != EC_E_NOERROR)
			{
				dwRetVal = dwRes;
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: Cannot set license key: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
				goto Exit;
			}
		}
	}

	/* print MAC address */
	{
		ETHERNET_ADDRESS oSrcMacAddress;

		OsMemset(&oSrcMacAddress, 0, sizeof(ETHERNET_ADDRESS));
		dwRes = ecatGetSrcMacAddress(&oSrcMacAddress);
		if (dwRes != EC_E_NOERROR)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot get MAC address: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		}
		EcLogMsg(EC_LOG_LEVEL_INFO,
			(pEcLogContext, EC_LOG_LEVEL_INFO, "EtherCAT network adapter MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
		    oSrcMacAddress.b[0], oSrcMacAddress.b[1], oSrcMacAddress.b[2], oSrcMacAddress.b[3], oSrcMacAddress.b[4], oSrcMacAddress.b[5]));
	}

	/* EtherCAT traffic logging */
#if (defined INCLUDE_PCAP_RECORDER)
	if (pAppParms->bPcapRecorder)
	{
		pPcapRecorder = EC_NEW(CPcapRecorder(100000, LOG_THREAD_PRIO, pAppContext->dwInstanceId, "EcatTraffic_0.pcap"));
		if (EC_NULL == pPcapRecorder)
		{
			dwRetVal = EC_E_NOMEMORY;
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: Master %d: Creating PcapRecorder failed!\n", pAppContext->dwInstanceId));
			goto Exit;
		}
	}
#endif

	/* Slave statistics polling for error diagnostic */
	{
		EC_T_DWORD dwPeriodMs = 1000;

		dwRes = ecatIoCtl(EC_IOCTL_SET_SLVSTAT_PERIOD, (EC_T_BYTE*)&dwPeriodMs, sizeof(EC_T_DWORD), EC_NULL, 0, EC_NULL);
		if (dwRes != EC_E_NOERROR)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ecatIoControl(EC_IOCTL_SET_SLVSTAT_PERIOD) returns with error=0x%x\n", dwRes));
			goto Exit;
		}
	}

	/* create cyclic task to trigger jobs */
	{
		CEcTimer oTimeout(2000);

		pAppContext->bJobTaskRunning = EC_FALSE;
		pAppContext->bJobTaskShutdown = EC_FALSE;
		pvJobTaskHandle = OsCreateThread((EC_T_CHAR*)"EcMasterJobTask",
			EcMasterJobTask,
			pAppParms->CpuSet,
			JOBS_THREAD_PRIO,
			JOBS_THREAD_STACKSIZE,
			(EC_T_VOID*)pAppContext);

		/* wait until thread is running */
		while (!oTimeout.IsElapsed() && !pAppContext->bJobTaskRunning)
		{
			OsSleep(10);
		}
		if (!pAppContext->bJobTaskRunning)
		{
			dwRetVal = EC_E_TIMEOUT;
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Timeout starting JobTask\n"));
			goto Exit;
		}
	}

	/* configure master */
	dwRes = ecatConfigureMaster(pAppParms->eCnfType, pAppParms->pbyCnfData, pAppParms->dwCnfDataLen);
	if (dwRes != EC_E_NOERROR)
	{
		dwRetVal = dwRes;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot configure EtherCAT-Master: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		goto Exit;
	}

	/* register client */
	dwRes = ecatRegisterClient(EcMasterNotifyCallback, pAppContext, &RegisterClientResults);
	if (dwRes != EC_E_NOERROR)
	{
		dwRetVal = dwRes;
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot register client: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		goto Exit;
	}
	pAppContext->pNotificationHandler->SetClientID(RegisterClientResults.dwClntId);

	/* configure DC/DCM master is started with ENI */
	if (EC_NULL != pAppParms->pbyCnfData)
	{
		/* configure DC */
		{
			EC_T_DC_CONFIGURE oDcConfigure;

			OsMemset(&oDcConfigure, 0, sizeof(EC_T_DC_CONFIGURE));
			oDcConfigure.dwTimeout          = ETHERCAT_DC_TIMEOUT;
			oDcConfigure.dwDevLimit         = ETHERCAT_DC_DEV_LIMIT;
			oDcConfigure.dwSettleTime       = ETHERCAT_DC_SETTLE_TIME;
			if (eDcmMode_MasterRefClock == pAppParms->eDcmMode)
			{
				oDcConfigure.dwTotalBurstLength = 10000;
				oDcConfigure.dwBurstBulk        = 1;
			}
			else
			{
				oDcConfigure.dwTotalBurstLength = ETHERCAT_DC_ARMW_BURSTCYCLES;
				if (pAppParms->dwBusCycleTimeUsec < 1000)
				{
					/* if the cycle time is below 1000 usec, we have to reduce the number of frames sent within one cycle */
					oDcConfigure.dwBurstBulk = ETHERCAT_DC_ARMW_BURSTSPP / 2;
				}
				else
				{
					oDcConfigure.dwBurstBulk = ETHERCAT_DC_ARMW_BURSTSPP;
				}
			}
#if (defined INCLUDE_DCX)
			if (eDcmMode_Dcx == pAppParms->eDcmMode)
			{
				oDcConfigure.bAcycDistributionDisabled = EC_FALSE; /* Enable acyclic distribution if cycle time is above 1000 usec to get DCX in sync */
			}
			else
			{
				oDcConfigure.bAcycDistributionDisabled = EC_TRUE;
			}
#else
			oDcConfigure.bAcycDistributionDisabled = EC_TRUE;
#endif
			dwRes = ecatDcConfigure(&oDcConfigure);
			if (dwRes != EC_E_NOERROR)
			{
				dwRetVal = dwRes;
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot configure DC! (Result = 0x%x)\n", dwRes));
				goto Exit;
			}
		}
		/* configure DCM */
		if (pAppParms->bDcmLogEnabled && !pAppParms->bDcmConfigure)
		{
			EC_T_BOOL bBusShiftConfiguredByEni = EC_FALSE;
			dwRes = ecatDcmGetBusShiftConfigured(&bBusShiftConfiguredByEni);
			if (dwRes != EC_E_NOERROR)
			{
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot check if BusShift is configured  (Result = 0x%x)\n", dwRes));
			}
			if (bBusShiftConfiguredByEni)
			{
				pAppParms->bDcmConfigure = EC_TRUE;
				pAppParms->eDcmMode = eDcmMode_BusShift;
			}
		}
		if (pAppParms->bDcmConfigure)
		{
			EC_T_INT        nCtlSetVal  = ((pAppParms->dwBusCycleTimeUsec * 2) / 3) * 1000; /* set value in nanosec, 66% of the bus cycle */
			EC_T_DCM_CONFIG oDcmConfig;
			OsMemset(&oDcmConfig, 0, sizeof(EC_T_DCM_CONFIG));

			switch (pAppParms->eDcmMode)
			{
			case eDcmMode_Off:
				oDcmConfig.eMode = eDcmMode_Off;
				break;
			case eDcmMode_BusShift:
				oDcmConfig.eMode = eDcmMode_BusShift;
				oDcmConfig.u.BusShift.nCtlSetVal    = nCtlSetVal;
				oDcmConfig.u.BusShift.dwInSyncLimit = (pAppParms->dwBusCycleTimeUsec * 1000) / 5; /* 20 % limit in nsec for InSync monitoring */
				oDcmConfig.u.BusShift.bLogEnabled = pAppParms->bDcmLogEnabled;
				if (pAppParms->bDcmControlLoopDisabled)
				{
					EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM control loop disabled for diagnosis!\n"));
					oDcmConfig.u.BusShift.bCtlOff = EC_TRUE;
				}
				break;
			case eDcmMode_MasterShift:
				oDcmConfig.eMode = eDcmMode_MasterShift;
				oDcmConfig.u.MasterShift.nCtlSetVal    = nCtlSetVal;
				oDcmConfig.u.MasterShift.dwInSyncLimit = (pAppParms->dwBusCycleTimeUsec * 1000) / 5; /* 20 % limit in nsec for InSync monitoring */
				oDcmConfig.u.MasterShift.bLogEnabled = pAppParms->bDcmLogEnabled;
				if (pAppParms->bDcmControlLoopDisabled)
				{
					EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM control loop disabled for diagnosis!\n"));
					oDcmConfig.u.MasterShift.bCtlOff = EC_TRUE;
				}
				break;
			case eDcmMode_MasterRefClock:
				oDcmConfig.eMode = eDcmMode_MasterRefClock;
				oDcmConfig.u.MasterRefClock.nCtlSetVal  = nCtlSetVal;
				oDcmConfig.u.MasterRefClock.bLogEnabled = pAppParms->bDcmLogEnabled;
				break;
			case eDcmMode_LinkLayerRefClock:
				oDcmConfig.eMode = eDcmMode_LinkLayerRefClock;
				oDcmConfig.u.LinkLayerRefClock.nCtlSetVal = nCtlSetVal;
				oDcmConfig.u.LinkLayerRefClock.bLogEnabled = pAppParms->bDcmLogEnabled;
				break;
#if (defined INCLUDE_DCX)
			case eDcmMode_Dcx:
				oDcmConfig.eMode = eDcmMode_Dcx;
				/* Mastershift */
				oDcmConfig.u.Dcx.MasterShift.nCtlSetVal = nCtlSetVal;
				oDcmConfig.u.Dcx.MasterShift.dwInSyncLimit = (pAppParms->dwBusCycleTimeUsec * 1000) / 5; /* 20 % limit in nsec for InSync monitoring */
				oDcmConfig.u.Dcx.MasterShift.bLogEnabled = pAppParms->bDcmLogEnabled;
				/* Dcx Busshift */
				oDcmConfig.u.Dcx.nCtlSetVal = nCtlSetVal;
				oDcmConfig.u.Dcx.dwInSyncLimit = (pAppParms->dwBusCycleTimeUsec * 1000) / 5; /* 20 % limit in nsec for InSync monitoring */
				oDcmConfig.u.Dcx.bLogEnabled = pAppParms->bDcmLogEnabled;
				oDcmConfig.u.Dcx.dwExtClockTimeout = 1000;
				oDcmConfig.u.Dcx.wExtClockFixedAddr = 0; /* 0 only when clock adjustment in external mode configured by EcEngineer */
				if (pAppParms->bDcmControlLoopDisabled)
				{
					EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM control loop disabled for diagnosis!\n"));

					oDcmConfig.u.Dcx.MasterShift.bCtlOff = EC_TRUE;
					oDcmConfig.u.Dcx.bCtlOff = EC_TRUE;
				}
				break;
#endif
			default:
				dwRetVal = EC_E_NOTSUPPORTED;
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "DCM mode is not supported!"));
				goto Exit;

			}
			dwRes = ecatDcmConfigure(&oDcmConfig, 0);
			switch (dwRes)
			{
			case EC_E_NOERROR:
				break;
			case EC_E_FEATURE_DISABLED:
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot configure DCM mode!"));
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Start with -dcmmode off to run the DC demo without DCM, or prepare the ENI file to support the requested DCM mode"));
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "In ET9000 for example, select under ""Advanced settings\\Distributed clocks"" ""DC in use"" and ""Slave Mode"""));
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "to support BusShift and MasterRefClock modes."));
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Please refer to the class A manual for further information"));
				dwRetVal = dwRes;
				goto Exit;
			default:
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot configure DCM mode! %s (Result = 0x%x)", ecatGetText(dwRes), dwRes));
				dwRetVal = dwRes;
				goto Exit;
			}
		}
	}

	/* print found slaves */
	if (pAppParms->dwAppLogLevel >= EC_LOG_LEVEL_VERBOSE)
	{
		dwRes = ecatScanBus(ETHERCAT_SCANBUS_TIMEOUT);
		pAppContext->pNotificationHandler->ProcessNotificationJobs();
		switch (dwRes)
		{
		case EC_E_NOERROR:
		case EC_E_BUSCONFIG_MISMATCH:
		case EC_E_LINE_CROSSED:
			PrintSlaveInfos(pAppContext);
			break;
		default:
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot scan bus: %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
			break;
		}
		if (dwRes != EC_E_NOERROR)
		{
			dwRetVal = dwRes;
			goto Exit;
		}
	}

	/* set master to INIT */
	dwRes = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_INIT);
	pAppContext->pNotificationHandler->ProcessNotificationJobs();
	if (dwRes != EC_E_NOERROR)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot start set master state to INIT: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		dwRetVal = dwRes;
		goto Exit;
	}

	dwRes = myAppPrepare(pAppContext);
	if (EC_E_NOERROR != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "myAppPrepare failed: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		dwRetVal = dwRes;
		goto Exit;
	}

	/* set master to PREOP */
	dwRes = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_PREOP);
	pAppContext->pNotificationHandler->ProcessNotificationJobs();
	if (dwRes != EC_E_NOERROR)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot start set master state to PREOP: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		dwRetVal = dwRes;
		goto Exit;
	}

	/* skip this step if demo started without ENI */
	if (EC_NULL != pAppParms->pbyCnfData)
	{
		dwRes = myAppSetup(pAppContext);
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "myAppSetup failed: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
			dwRetVal = dwRes;
			goto Exit;
		}

		/* set master to SAFEOP */
		dwRes = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_SAFEOP);
		pAppContext->pNotificationHandler->ProcessNotificationJobs();
		if (dwRes != EC_E_NOERROR)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot start set master state to SAFEOP: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));

			/* most of the time SAFEOP is not reachable due to DCM */
			if ((eDcmMode_Off != pAppParms->eDcmMode) && (eDcmMode_LinkLayerRefClock != pAppParms->eDcmMode))
			{
				EC_T_DWORD dwStatus = 0;
				EC_T_INT   nDiffCur = 0, nDiffAvg = 0, nDiffMax = 0;

				dwRes = ecatDcmGetStatus(&dwStatus, &nDiffCur, &nDiffAvg, &nDiffMax);
				if (dwRes == EC_E_NOERROR)
				{
					if (dwStatus != EC_E_NOERROR)
					{
						EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "DCM Statusyyy: %s (0x%08X)\n", ecatGetText(dwStatus), dwStatus));
					}
				}
				else
				{
					EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot get DCM status! %s (0x%08X)\n", ecatGetText(dwRes), dwRes));
				}
			}
			dwRetVal = dwRes;
			goto Exit;
		}

		/* set master to OP */
		dwRes = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_OP);
		pAppContext->pNotificationHandler->ProcessNotificationJobs();
		if (dwRes != EC_E_NOERROR)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot start set master state to OP: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
			dwRetVal = dwRes;
			goto Exit;
		}
	}
	else
	{
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "No ENI file provided. EC-Master started with generated ENI file.\n"));
	}

	if (pAppParms->bPerfMeasEnabled)
	{
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "\nJob times during startup <INIT> to <%s>:\n", ecatStateToStr(ecatGetMasterState())));
		PERF_MEASURE_JOBS_SHOW();
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "\n"));
		/* clear job times of startup phase */
		PERF_MEASURE_JOBS_RESET();
	}

#if (defined DEBUG) && (defined EC_VERSION_XENOMAI)
	/* Enabling mode switch warnings for shadowed task (mallocs before may have switched mode) */
	dwRes = rt_task_set_mode(0, T_WARNSW, NULL);
	if (0 != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "EnableRealtimeEnvironment: rt_task_set_mode returned error 0x%lx\n", dwRes));
		OsDbgAssert(EC_FALSE);
	}
#endif

	/* run the demo */
	if (pAppParms->dwDemoDuration != 0)
	{
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "%s will stop in %ds...\n", EC_DEMO_APP_NAME, pAppParms->dwDemoDuration / 1000));
		oAppDuration.Start(pAppParms->dwDemoDuration);
	}

	pAppParms->bPerfMeasShowCyclic = true;
	while (bRun && (!oAppDuration.IsStarted() || !oAppDuration.IsElapsed()))
	{
		if (pAppParms->bPerfMeasShowCyclic)
		{
			PERF_MEASURE_JOBS_SHOW();
		}
		/* check if demo shall terminate */
		bRun = !OsTerminateAppRequest();

		myAppDiagnosis(pAppContext);

		if (EC_NULL != pAppParms->pbyCnfData)
		{
			if ((eDcmMode_Off != pAppParms->eDcmMode) && (eDcmMode_LinkLayerRefClock != pAppParms->eDcmMode))
			{
				EC_T_DWORD dwStatus = 0;
				EC_T_BOOL  bWriteDiffLog = EC_FALSE;
				EC_T_INT   nDiffCur = 0, nDiffAvg = 0, nDiffMax = 0;

				if (!oDcmStatusTimer.IsStarted() || oDcmStatusTimer.IsElapsed())
				{
					bWriteDiffLog = EC_TRUE;
					oDcmStatusTimer.Start(5000);
				}

				dwRes = ecatDcmGetStatus(&dwStatus, &nDiffCur, &nDiffAvg, &nDiffMax);
				if (dwRes == EC_E_NOERROR)
				{
					if (bFirstDcmStatus)
					{
						EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM during startup (<INIT> to <%s>)", ecatStateToStr(ecatGetMasterState())));
					}
					if ((dwStatus != EC_E_NOTREADY) && (dwStatus != EC_E_BUSY) && (dwStatus != EC_E_NOERROR))
					{
						EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM Statusxxx: %s (0x%08X)\n", ecatGetText(dwStatus), dwStatus));
					}
					if (bWriteDiffLog && pAppParms->bDcmLogEnabled)
					{
						EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCM Diff (cur/avg/max) [nsec]: %7d/ %7d/ %7d", nDiffCur, nDiffAvg, nDiffMax));
					}
				}
				else
				{
					if ((eEcatState_OP == ecatGetMasterState()) || (eEcatState_SAFEOP == ecatGetMasterState()))
					{
						EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot get DCM status! %s (0x%08X)\n", ecatGetText(dwRes), dwRes));
					}
				}
#if (defined INCLUDE_DCX)
				if (eDcmMode_Dcx == pAppParms->eDcmMode && EC_E_NOERROR == dwRes)
				{
					EC_T_INT64 nTimeStampDiff = 0;
					dwRes = ecatDcxGetStatus(&dwStatus, &nDiffCur, &nDiffAvg, &nDiffMax, &nTimeStampDiff);
					if (EC_E_NOERROR == dwRes)
					{
						if (bFirstDcmStatus)
						{
							EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCX during startup (<INIT> to <%s>)", ecatStateToStr(ecatGetMasterState())));
						}
						if ((dwStatus != EC_E_NOTREADY) && (dwStatus != EC_E_BUSY) && (dwStatus != EC_E_NOERROR))
						{
							EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCX Status: %s (0x%08X)\n", ecatGetText(dwStatus), dwStatus));
						}
						if (bWriteDiffLog && pAppParms->bDcmLogEnabled)
						{
							EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "DCX Diff(ns): Cur=%7d, Avg=%7d, Max=%7d, TimeStamp=%7d", nDiffCur, nDiffAvg, nDiffMax, nTimeStampDiff));
						}
					}
					else
					{
						if ((eEcatState_OP == ecatGetMasterState()) || (eEcatState_SAFEOP == ecatGetMasterState()))
						{
							EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot get DCX status! %s (0x%08X)\n", ecatGetText(dwRes), dwRes));
						}
					}
				}
#endif
				if (bFirstDcmStatus && (EC_E_NOERROR == dwRes))
				{
					bFirstDcmStatus = EC_FALSE;
					ecatDcmResetStatus();
				}
			}
		}
		/* process notification jobs */
		pAppContext->pNotificationHandler->ProcessNotificationJobs();

		OsSleep(5);
	}

	if (pAppParms->dwAppLogLevel >= EC_LOG_LEVEL_VERBOSE)
	{
		EC_T_DWORD dwCurrentUsage = 0;
		EC_T_DWORD dwMaxUsage = 0;
		dwRes = ecatGetMemoryUsage(&dwCurrentUsage, &dwMaxUsage);
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: Cannot read memory usage of master: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
			goto Exit;
		}
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Memory Usage Master     (cur/max) [bytes]: %u/%u\n", dwCurrentUsage, dwMaxUsage));

#if (defined ATEMRAS_SERVER)
		if (bRasServerStarted)
		{
			dwRes = emRasGetMemoryUsage(pvRasServerHandle, &dwCurrentUsage, &dwMaxUsage);
			if (EC_E_NOERROR != dwRes)
			{
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: Cannot read memory usage of RAS: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
				goto Exit;
			}
			EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Memory Usage RAS Server (cur/max) [bytes]: %u/%u\n", dwCurrentUsage, dwMaxUsage));
		}
#endif
	}


Exit:
	/* set master state to INIT */
	if (eEcatState_UNKNOWN != ecatGetMasterState())
	{
		if (pAppParms->bPerfMeasEnabled)
		{
			EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "\nJob times before shutdown\n"));
			PERF_MEASURE_JOBS_SHOW();
		}

		dwRes = ecatSetMasterState(ETHERCAT_STATE_CHANGE_TIMEOUT, eEcatState_INIT);
		pAppContext->pNotificationHandler->ProcessNotificationJobs();
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot stop EtherCAT-Master: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		}
	}

#if (defined INCLUDE_PCAP_RECORDER)
	SafeDelete(pPcapRecorder);
#endif /* INCLUDE_PCAP_RECORDER */

	/* unregister client */
	if (0 != RegisterClientResults.dwClntId)
	{
		dwRes = ecatUnregisterClient(RegisterClientResults.dwClntId);
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot unregister client: %s (0x%lx))\n", ecatGetText(dwRes), dwRes));
		}
	}
#if (defined DEBUG) && (defined EC_VERSION_XENOMAI)
	/* disable PRIMARY to SECONDARY MODE switch warning */
	dwRes = rt_task_set_mode(T_WARNSW, 0, NULL);
	if (dwRes != 0)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "rt_task_set_mode returned error %d\n", dwRes));
		OsDbgAssert(EC_FALSE);
	}
#endif

	/* shutdown JobTask */
	{
		CEcTimer oTimeout(2000);
		pAppContext->bJobTaskShutdown = EC_TRUE;
		while (pAppContext->bJobTaskRunning && !oTimeout.IsElapsed())
		{
			OsSleep(10);
		}
		if (EC_NULL != pvJobTaskHandle)
		{
			OsDeleteThreadHandle(pvJobTaskHandle);
		}
	}

	/* deinitialize master */
	dwRes = ecatDeinitMaster();
	if (EC_E_NOERROR != dwRes)
	{
		EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Cannot de-initialize EtherCAT-Master: %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
	}

#if (defined ATEMRAS_SERVER)
	/* stop RAS server */
	if (bRasServerStarted)
	{
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "Stop Remote Api Server\n"));
		dwRes = emRasSrvStop(pvRasServerHandle, 2000);
		if (EC_E_NOERROR != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Remote API Server shutdown failed\n"));
		}
	}
#endif
	if (pAppParms->bPerfMeasEnabled)
	{
		PERF_MEASURE_JOBS_DEINIT();
	}
	SafeDelete(pAppContext->pNotificationHandler);
	SafeOsFree(pAppContext->pMyAppDesc);

	return dwRetVal;
}

/********************************************************************************/
/** \brief  Trigger jobs to drive master, and update process data.
*
* \return N/A
*/
static EC_T_VOID EcMasterJobTask(EC_T_VOID* pvAppContext)
{
	EC_T_DWORD dwRes = EC_E_ERROR;
	EC_T_INT   nOverloadCounter = 0; /* counter to check if cycle time is to short */
	T_EC_DEMO_APP_CONTEXT* pAppContext = (T_EC_DEMO_APP_CONTEXT*)pvAppContext;
	T_EC_DEMO_APP_PARMS*   pAppParms   = &pAppContext->AppParms;

	EC_T_USER_JOB_PARMS oJobParms;
	OsMemset(&oJobParms, 0, sizeof(EC_T_USER_JOB_PARMS));

	/* demo loop */
	pAppContext->bJobTaskRunning = EC_TRUE;
	do
	{
		/* wait for next cycle (event from scheduler task) */
		OsWaitForEvent(pAppContext->pvJobTaskEvent, EC_WAITINFINITE);

		PERF_JOB_END(PERF_CycleTime);
		PERF_JOB_START(PERF_CycleTime);

		/* process all received frames (read new input values) */
		PERF_JOB_START(JOB_ProcessAllRxFrames);
		dwRes = ecatExecJob(eUsrJob_ProcessAllRxFrames, &oJobParms);
		if (EC_E_NOERROR != dwRes && EC_E_INVALIDSTATE != dwRes && EC_E_LINK_DISCONNECTED != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: ecatExecJob( eUsrJob_ProcessAllRxFrames): %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
		}
		PERF_JOB_END(JOB_ProcessAllRxFrames);

		if (EC_E_NOERROR == dwRes)
		{
			if (!oJobParms.bAllCycFramesProcessed)
			{
				/* it is not reasonable, that more than 5 continuous frames are lost */
				nOverloadCounter += 10;
				if (nOverloadCounter >= 50)
				{
					EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Error: System overload: Cycle time too short or huge jitter!\n"));
				}
				else
				{
					EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "eUsrJob_ProcessAllRxFrames - not all previously sent frames are received/processed (frame loss)!\n"));
				}
				if (pAppParms->bPerfMeasEnabled)
				{
					EcLogMsg(EC_LOG_LEVEL_INFO,
						(pEcLogContext, EC_LOG_LEVEL_INFO, "PerfMsmt '%s' (current) [usec]: %3d.%d\n",
					    S_aszMeasInfo[PERF_CycleTime], pAppContext->TscMeasDesc.aTscTime[PERF_CycleTime].dwCurr / 10, pAppContext->TscMeasDesc.aTscTime[PERF_CycleTime].dwCurr % 10));
				}
			}
			else
			{
				/* everything o.k.? If yes, decrement overload counter */
				if (nOverloadCounter > 0)    nOverloadCounter--;
			}
		}
		/* Handle DCM logging */
#ifdef DCM_ENABLE_LOGFILE
		if (pAppParms->bDcmLogEnabled)
		{
			EC_T_CHAR* pszLog = EC_NULL;

			PERF_JOB_START(PERF_DCM_Logfile);
			ecatDcmGetLog(&pszLog);
			if ((EC_NULL != pszLog))
			{
				((CAtEmLogging*)pEcLogContext)->LogDcm(pszLog);
			}
			PERF_JOB_END(PERF_DCM_Logfile);
		}
#endif

		PERF_JOB_START(PERF_myAppWorkpd);
		{
			EC_T_STATE eMasterState = ecatGetMasterState();

			if ((eEcatState_SAFEOP == eMasterState) || (eEcatState_OP == eMasterState))
			{
				myAppWorkpd(pAppContext);
			}
		}
		PERF_JOB_END(PERF_myAppWorkpd);

		/* write output values of current cycle, by sending all cyclic frames */
		PERF_JOB_START(JOB_SendAllCycFrames);
		dwRes = ecatExecJob(eUsrJob_SendAllCycFrames, &oJobParms);
		if (EC_E_NOERROR != dwRes && EC_E_INVALIDSTATE != dwRes && EC_E_LINK_DISCONNECTED != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ecatExecJob( eUsrJob_SendAllCycFrames,    EC_NULL ): %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
		}
		PERF_JOB_END(JOB_SendAllCycFrames);

		/* remove this code when using licensed version */
		if (EC_E_EVAL_EXPIRED == dwRes)
		{
			bRun = EC_FALSE;
		}

		/* execute some administrative jobs. No bus traffic is performed by this function */
		PERF_JOB_START(JOB_MasterTimer);
		dwRes = ecatExecJob(eUsrJob_MasterTimer, EC_NULL);
		if (EC_E_NOERROR != dwRes && EC_E_INVALIDSTATE != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ecatExecJob(eUsrJob_MasterTimer, EC_NULL): %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
		}
		PERF_JOB_END(JOB_MasterTimer);

		/* send queued acyclic EtherCAT frames */
		PERF_JOB_START(JOB_SendAcycFrames);
		dwRes = ecatExecJob(eUsrJob_SendAcycFrames, EC_NULL);
		if (EC_E_NOERROR != dwRes && EC_E_INVALIDSTATE != dwRes && EC_E_LINK_DISCONNECTED != dwRes)
		{
			EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ecatExecJob(eUsrJob_SendAcycFrames, EC_NULL): %s (0x%lx)\n", ecatGetText(dwRes), dwRes));
		}
		PERF_JOB_END(JOB_SendAcycFrames);

		PERF_JOB_TOTAL;
#if !(defined NO_OS)
	} while (!pAppContext->bJobTaskShutdown);

	pAppContext->bJobTaskRunning = EC_FALSE;
#else
	/* in case of NO_OS the job task function is called cyclically within the timer ISR */
} while (EC_FALSE) ;
pAppContext->bJobTaskRunning = !pAppContext->bJobTaskShutdown;
#endif

return;
}

/********************************************************************************/
/** \brief  Handler for master notifications
*
* \return  Status value.
*/
static EC_T_DWORD EcMasterNotifyCallback(
    EC_T_DWORD         dwCode,
	/**< [in]   Notification code */
    EC_T_NOTIFYPARMS*  pParms   /**< [in]   Notification parameters */)
{
	EC_T_DWORD dwRetVal = EC_E_NOERROR;
	CEmNotification* pNotificationHandler = EC_NULL;

	if ((EC_NULL == pParms) || (EC_NULL == pParms->pCallerData))
	{
		dwRetVal = EC_E_INVALIDPARM;
		goto Exit;
	}

	pNotificationHandler = ((T_EC_DEMO_APP_CONTEXT*)pParms->pCallerData)->pNotificationHandler;

	if ((dwCode >= EC_NOTIFY_APP) && (dwCode <= EC_NOTIFY_APP + EC_NOTIFY_APP_MAX_CODE))
	{
		/* notification for application */
		dwRetVal = myAppNotify(dwCode - EC_NOTIFY_APP, pParms);
	}
	else
	{
		/* default notification handler */
		dwRetVal = pNotificationHandler->ecatNotify(dwCode, pParms);
	}

Exit:
	return dwRetVal;
}


/********************************************************************************/
/** \brief  Handler for master RAS notifications
*
*
* \return  Status value.
*/
#ifdef ATEMRAS_SERVER
static EC_T_DWORD RasNotifyCallback(
    EC_T_DWORD         dwCode,
	EC_T_NOTIFYPARMS*  pParms)
{
	EC_T_DWORD dwRetVal = EC_E_NOERROR;
	CEmNotification* pNotificationHandler = EC_NULL;

	if ((EC_NULL == pParms) || (EC_NULL == pParms->pCallerData))
	{
		dwRetVal = EC_E_INVALIDPARM;
		goto Exit;
	}

	pNotificationHandler = (CEmNotification*)pParms->pCallerData;
	dwRetVal = pNotificationHandler->emRasNotify(dwCode, pParms);

Exit:
	return dwRetVal;
}
#endif

/*-MYAPP---------------------------------------------------------------------*/

/***************************************************************************************************/
/**
\brief  Initialize Application

\return EC_E_NOERROR on success, error code otherwise.
*/
static EC_T_DWORD myAppInit(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	//mainApp->createTestThreads(pAppContext);
#if 0
	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	UINT32 dwRetVal;

	printf("My app Init");
	EC_T_VOID*             pvJobTaskHandle   = EC_NULL;

	mainApp->createMsgQueueWithNRTM();

	pvJobTaskHandle = OsCreateThread((EC_T_CHAR*)"RTM_MainApp",
				mainRun,
				pAppContext->AppParms.CpuSet+1,
				RTM_THREAD_PRIO,
				JOBS_THREAD_STACKSIZE,
				(EC_T_VOID*)pAppContext);
#endif
	return EC_E_NOERROR;
}

/***************************************************************************************************/
/**
\brief  Initialize Slave Instance.

Find slave parameters.
\return EC_E_NOERROR on success, error code otherwise.
*/
static EC_T_DWORD myAppPrepare(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	EC_T_DWORD          dwRes      = EC_E_NOERROR;
	T_MY_APP_DESC*      pMyAppDesc = pAppContext->pMyAppDesc;
	EC_T_CFG_SLAVE_INFO oCfgSlaveInfo;
	OsMemset(&oCfgSlaveInfo, 0, sizeof(EC_T_CFG_SLAVE_INFO));

	if (pAppContext->AppParms.bFlash)
	{
		EC_T_WORD wFlashSlaveAddr = pAppContext->AppParms.wFlashSlaveAddr;

		/* check if slave address is provided */
		if (wFlashSlaveAddr > 0)
		{
			/* now get the offset of this device in the process data buffer and some other infos */
			dwRes = ecatGetCfgSlaveInfo(EC_TRUE, wFlashSlaveAddr, &oCfgSlaveInfo);
			if (dwRes != EC_E_NOERROR)
			{
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ERROR: ecatGetCfgSlaveInfo() returns with error=0x%x, slave address=%d\n", dwRes, wFlashSlaveAddr));
			}
			else
			{
				if (oCfgSlaveInfo.dwPdSizeOut != 0)
				{
					pMyAppDesc->dwFlashPdBitSize = oCfgSlaveInfo.dwPdSizeOut;
					pMyAppDesc->dwFlashPdBitOffs = oCfgSlaveInfo.dwPdOffsOut;
				}
				else
				{
					EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Slave address=%d has no outputs, therefore flashing not possible\n", wFlashSlaveAddr));
				}
			}
		}
		else
		{
			/* get complete process data output size */
			EC_T_MEMREQ_DESC oPdMemorySize;
			OsMemset(&oPdMemorySize, 0, sizeof(EC_T_MEMREQ_DESC));

			dwRes = ecatIoCtl(EC_IOCTL_GET_PDMEMORYSIZE, EC_NULL, 0, (EC_T_VOID*)&oPdMemorySize, sizeof(EC_T_MEMREQ_DESC), EC_NULL);
			if (dwRes != EC_E_NOERROR)
			{
				EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "ecatIoControl(EC_IOCTL_GET_PDMEMORYSIZE) returns with error=0x%x\n", dwRes));
				goto Exit;
			}
			pMyAppDesc->dwFlashPdBitSize = oPdMemorySize.dwPDOutSize * 8;
		}
		if (pMyAppDesc->dwFlashPdBitSize > 0)
		{
			pMyAppDesc->dwFlashInterval = 20000; /* flash every 20 msec */
			pMyAppDesc->dwFlashBufSize = BIT2BYTE(pMyAppDesc->dwFlashPdBitSize);
			pMyAppDesc->pbyFlashBuf = (EC_T_BYTE*)OsMalloc(pMyAppDesc->dwFlashBufSize);
			OsMemset(pMyAppDesc->pbyFlashBuf, 0, pMyAppDesc->dwFlashBufSize);
		}
	}
	printf("pMyAppDesc->dwFlashPdBitSize : %d - pMyAppDesc->dwFlashBufSize : %d", pMyAppDesc->dwFlashPdBitSize, pMyAppDesc->dwFlashBufSize);
	

Exit:
	return EC_E_NOERROR;
}

/***************************************************************************************************/
/**
\brief  Setup slave parameters (normally done in PREOP state

  - SDO up- and Downloads
  - Read Object Dictionary

\return EC_E_NOERROR on success, error code otherwise.
*/


#define EVENTDEMO_EVENT_TX      TEXT("EVENTDEMO_RTOS2WIN")
#define EVENTDEMO_EVENT_RX      TEXT("EVENTDEMO_WIN2RTOS")

#define DEMO_LOCAL_READY        "DEMO_RTOS_READY"
#define DEMO_REMOTE_READY       "DEMO_HOST_READY"


static EC_T_DWORD myAppSetup(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	/*dwRetVal = RtosLibInit();
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("Error(0x%X)\r\n"), dwRetVal);
	}*/


#if 0	
	RTOSLIB_HANDLE hEventLocalReady = NULL;
	RTOSLIB_HANDLE hEventRemoteReady = NULL;
	
	/* Wait for demo counterpart */
	DEMO_PRINTF(TEXT("  %-28s : "), TEXT("Waiting for demo counterpart"));
	dwRetVal = RtosCreateEvent(FALSE, FALSE, TEXT(DEMO_LOCAL_READY), &hEventLocalReady);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosCreateEvent %s error 0x%X\n"), TEXT(DEMO_LOCAL_READY), dwRetVal);
	}
	dwRetVal = RtosCreateEvent(FALSE, FALSE, TEXT(DEMO_REMOTE_READY), &hEventRemoteReady);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosCreateEvent %s error 0x%X\n"), TEXT(DEMO_REMOTE_READY), dwRetVal);
	}
	dwRetVal = RtosSetEvent(hEventLocalReady);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosSetEvent %s error 0x%X\n"), TEXT(DEMO_LOCAL_READY), dwRetVal);
	}
	do 
	{
		dwRetVal = RtosWaitForEvent(hEventRemoteReady, RTOS_WAIT_INFINITE);
		switch (dwRetVal)
		{
		case RTE_SUCCESS:
			DEMO_PRINTF(TEXT("Ok\n"));
			break;
		case RTE_ERROR_TIMEOUT:
			break;
		default:
			DEMO_PRINTF(TEXT("RtosWaitForEvent %s error 0x%X\n"), TEXT(DEMO_REMOTE_READY), dwRetVal);
			break;
		}
	} while (RTE_ERROR_TIMEOUT == RTE_ERROR_GET_ERROR(dwRetVal));
	
	

	RTOSLIB_HANDLE hEventRx = NULL;
	RTOSLIB_HANDLE hEventTx = NULL;
	dwRetVal = RtosCreateEvent(FALSE, FALSE, EVENTDEMO_EVENT_TX, &hEventTx);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosCreateEvent %s error 0x%X\n"), EVENTDEMO_EVENT_TX, dwRetVal);
	}
	dwRetVal = RtosCreateEvent(FALSE, FALSE, EVENTDEMO_EVENT_RX, &hEventRx);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosCreateEvent %s error 0x%X\n"), EVENTDEMO_EVENT_RX, dwRetVal);
	}
	dwRetVal = RtosSetEvent(hEventTx);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosSetEvent %s error 0x%X\n"), EVENTDEMO_EVENT_TX, dwRetVal);
	}
	
	do 
	{
		printf("Wait for event\n");
		dwRetVal = RtosWaitForEvent(hEventRx, 1000);
		switch (dwRetVal)
		{
		case RTE_SUCCESS:
			DEMO_PRINTF(TEXT("Ok\n"));
			break;
		case RTE_ERROR_TIMEOUT:
			DEMO_PRINTF(TEXT("Timeout\n"));
			break;
		default:
			DEMO_PRINTF(TEXT("RtosWaitForEvent %s error 0x%X\n"), EVENTDEMO_EVENT_RX, dwRetVal);
			break;
		}
	} while (RTE_ERROR_TIMEOUT == RTE_ERROR_GET_ERROR(dwRetVal));
	
	dwRetVal = RtosSetEvent(hEventTx);
	if (RTE_SUCCESS != dwRetVal)
	{
		DEMO_PRINTF(TEXT("RtosSetEvent %s error 0x%X\n"), EVENTDEMO_EVENT_TX, dwRetVal);
	}
	
	dwRetVal = RtosCloseEvent(hEventTx);
	dwRetVal = RtosCloseEvent(hEventRx);
#endif


	return EC_E_NOERROR;
}

/***************************************************************************************************/
int count = 0;
int sendCount = 0;
/**
\brief  demo application working process data function.

  This function is called in every cycle after the the master stack is started.

*/
static EC_T_DWORD myAppWorkpd(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	T_MY_APP_DESC* pMyAppDesc = pAppContext->pMyAppDesc;
	EC_T_BYTE*     pbyPdOut   = ecatGetProcessImageOutputPtr();
	EC_T_BYTE*     pbyPdIn   = ecatGetProcessImageInputPtr();
	
	struct timespec t;
	OsMemset(&t, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t);

	RTM_MainApp *mainApp = RTM_MainApp::getInstance();
	mainApp->copyRcvdEthercatMsgToBuffer(pbyPdIn);
	mainApp->takeDataFromMsgQueue();
	mainApp->triggerTests();
	mainApp->copySendBufferToEthercat(pbyPdOut);

	struct timespec t2;
	OsMemset(&t2, 0, sizeof(struct timespec));
	/* get current time */
	clock_gettime(CLOCK_MONOTONIC, &t2);

	if ((t2.tv_sec-t.tv_sec) > 0 || (t2.tv_nsec-t.tv_nsec) > 100000)
		printf("->%ld\n", (t2.tv_nsec-t.tv_nsec));

#if 0

	/* demo code flashing */
	if (pMyAppDesc->dwFlashPdBitSize != 0)
	{
		pMyAppDesc->dwFlashTimer += pAppContext->AppParms.dwBusCycleTimeUsec;
		if (pMyAppDesc->dwFlashTimer >= pMyAppDesc->dwFlashInterval)
		{
			pMyAppDesc->dwFlashTimer = 0;

			/* flash with pattern */
			pMyAppDesc->byFlashVal++;
			OsMemset(pMyAppDesc->pbyFlashBuf, pMyAppDesc->byFlashVal, pMyAppDesc->dwFlashBufSize);

			/* update PdOut */
			EC_COPYBITS(pbyPdOut, pMyAppDesc->dwFlashPdBitOffs, pMyAppDesc->pbyFlashBuf, 0, pMyAppDesc->dwFlashPdBitSize);
		}
	}
#endif
	return EC_E_NOERROR;
}

/***************************************************************************************************/
/**
\brief  demo application doing some diagnostic tasks

  This function is called in sometimes from the main demo task
*/
static EC_T_DWORD myAppDiagnosis(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	EC_UNREFPARM(pAppContext);
	return EC_E_NOERROR;
}

/********************************************************************************/
/** \brief  Handler for application notifications
*
*  !!! No blocking API shall be called within this function!!!
*  !!! Function is called by cylic task                    !!!
*
* \return  Status value.
*/
static EC_T_DWORD myAppNotify(
    EC_T_DWORD        dwCode,
	/* [in] Application notification code */
    EC_T_NOTIFYPARMS* pParms  /* [in] Notification parameters */)
{
	EC_T_DWORD dwRetVal = EC_E_ERROR;
	T_EC_DEMO_APP_CONTEXT* pAppContext = (T_EC_DEMO_APP_CONTEXT*)pParms->pCallerData;

	/* dispatch notification code */
	switch (dwCode)
	{
	case 1:
		EcLogMsg(EC_LOG_LEVEL_INFO, (pEcLogContext, EC_LOG_LEVEL_INFO, "myAppNotify: Notification code=0x%lx received\n", dwCode));
		/* dwRetVal = EC_E_NOERROR; */
		break;
	case 2:
		break;
	default:
		break;
	}

	return dwRetVal;
}

EC_T_VOID ShowSyntaxAppUsage(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	const EC_T_CHAR* szAppUsage = "<LinkLayer> [-f ENI-FileName] [-t time] [-b time] [-a affinity] [-v lvl] [-perf] [-log prefix] [-flash address]"
#if (defined ATEMRAS_SERVER)
	        " [-sp [port]]"
#endif
#if (defined AUXCLOCK_SUPPORTED)
	                " [-auxclk period]"
#endif
	                        " [-dcmmode mode] [-ctloff]"
	                        ;
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "%s V%s for %s %s\n", EC_DEMO_APP_NAME, ATECAT_FILEVERSIONSTR, ATECAT_PLATFORMSTR, ATECAT_COPYRIGHT));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "Syntax:\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "%s %s", EC_DEMO_APP_NAME, szAppUsage));
}

EC_T_VOID ShowSyntaxApp(T_EC_DEMO_APP_CONTEXT* pAppContext)
{
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -flash            Flash outputs\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     address         0=all, >0 = slave station address\n"));
#if (defined ATEMRAS_SERVER)
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -sp               Server port binding\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     port            port (default = %d)\n", ATEMRAS_DEFAULT_PORT));
#endif
#if (defined AUXCLOCK_SUPPORTED)
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -auxclk           use auxiliary clock\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     period          clock period in usec\n"));
#endif
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -dcmmode          Set DCM mode\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     off                Off\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     busshift           BusShift mode (default)\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     mastershift        MasterShift mode\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     masterrefclock     MasterRefClock mode\n"));
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     linklayerrefclock  LinkLayerRefClock mode\n"));
#if (defined INCLUDE_DCX)
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "     dcx                External synchronization mode\n"));
#endif
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -ctloff           Disable DCM control loop for diagnosis\n"));
#if (defined INCLUDE_PCAP_RECORDER)
	EcLogMsg(EC_LOG_LEVEL_ERROR, (pEcLogContext, EC_LOG_LEVEL_ERROR, "   -rec              Record network traffic to pcap file\n"));
#endif
}

/*-END OF SOURCE FILE--------------------------------------------------------*/
