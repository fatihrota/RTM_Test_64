/*-----------------------------------------------------------------------------
 * EcDemoParms.h
 * Copyright                acontis technologies GmbH, Ravensburg, Germany
 * Response                 Holger Oelhaf
 * Description              Application specific settings for EC-Master demo
 *---------------------------------------------------------------------------*/

#ifndef INC_ECDEMOPARMS_H
#define INC_ECDEMOPARMS_H 1

/*-DEFINES-------------------------------------------------------------------*/
#if !(defined EC_DEMO_TINY)
#define MASTER_CFG_ECAT_MAX_BUS_SLAVES       256    /* max number of pre-allocated bus slave objects */
#define MASTER_CFG_MAX_ACYC_FRAMES_QUEUED     32    /* max number of acyc frames queued, 127 = the absolute maximum number */
#define MASTER_CFG_MAX_ACYC_BYTES_PER_CYC   4096    /* max number of bytes sent during eUsrJob_SendAcycFrames within one cycle */
#else
#define MASTER_CFG_ECAT_MAX_BUS_SLAVES         8    /* max number of pre-allocated bus slave objects */
#define MASTER_CFG_MAX_ACYC_FRAMES_QUEUED     32    /* max number of acyc frames queued, 127 = the absolute maximum number */
#define MASTER_CFG_MAX_ACYC_BYTES_PER_CYC    1400    /* max number of bytes sent during eUsrJob_SendAcycFrames within one cycle */
#endif /* EC_DEMO_TINY */
#define MASTER_CFG_MAX_ACYC_CMD_RETRIES        3

#define ETHERCAT_STATE_CHANGE_TIMEOUT      15000    /* master state change timeout in ms */
#define ETHERCAT_SCANBUS_TIMEOUT           10000    /* scanbus timeout in ms, see also EC_SB_DEFAULTTIMEOUT */

#define COMMAND_LINE_BUFFER_LENGTH 1400
#define MAX_LINKLAYER   5

#if (defined EC_SIMULATOR_DS402)
#define DEMO_MAX_NUM_OF_AXIS                   4    /* maximum number of axes that can be used in DS402 Demos */
#endif

/*-TYPEDEFS------------------------------------------------------------------*/
/* demo application parameters */
typedef struct _T_EC_DEMO_APP_PARMS
{
    EC_T_OS_PARMS       Os;                             /* operating system parameter */
    EC_T_DWORD          dwCpuIndex;                     /* CPU index */
    EC_T_CPUSET         CpuSet;                         /* CPU-set for SMP systems */
    /* link layer */
    EC_T_LINK_PARMS*    apLinkParms[MAX_LINKLAYER];     /* link layer parameter */
    EC_T_DWORD          dwNumLinkLayer;                 /* number of link layers */
    EC_T_LINK_TTS       TtsParms;                       /* time triggered send parameter */
    /* configuration */
    EC_T_CNF_TYPE       eCnfType;                       /* type of configuration data provided */
    EC_T_BYTE*          pbyCnfData;                     /* filename/configuration data */
    EC_T_DWORD          dwCnfDataLen;                   /* length of configuration data in bytes */
    EC_T_CHAR           szENIFilename[256];             /* ENI filename string */
    EC_T_CHAR           szLicenseKey[64];               /* license key string */
    /* timing */
    EC_T_BOOL           bUseAuxClock;                   /* enabled use of auxiliary clock  */
    EC_T_DWORD          dwBusCycleTimeUsec;             /* bus cycle time in usec */
    EC_T_DWORD          dwDemoDuration;                 /* demo duration in msec */
    /* logging */
    EC_T_INT            nVerbose;                       /* verbosity level */
    EC_T_DWORD          dwAppLogLevel;                  /* demo application log level (derived from verbosity level) */
    EC_T_DWORD          dwMasterLogLevel;               /* master stack log level (derived from verbosity level) */
    EC_T_CHAR           szLogFileprefix[64];            /* log file prefix string */
    /* RAS */
    EC_T_WORD           wRasServerPort;                 /* remote access server port */
    EC_T_BYTE           abyRasServerIpAddress[4];       /* remote access server IP address */
    EC_T_BOOL           bRasAccessControlEnabled;       /* remote access server access control enabled*/
    EC_T_DWORD          dwRasAccessLevel;               /* remote access server access control level */
    /* mailbox gateway server */
    EC_T_WORD           wMbxGatewayServerPort;          /* mailbox gateway server port */
    /* DCM */
    EC_T_BOOL           bDcmConfigure;                  /* DCM configuration enabled */
    EC_T_DCM_MODE       eDcmMode;                       /* DCM mode */
    EC_T_BOOL           bDcmControlLoopDisabled;        /* DCM control loop disabled */
    EC_T_BOOL           bDcmLogEnabled;                 /* DCM logging enabled */
    /* master redundancy */
    EC_T_BOOL           bMasterRedPermanentStandby;     /* master redundancy instance in permanent standby */
    EC_T_BOOL           bPcapRecorder;                  /* EtherCAT packet capture in pcap format (wireshark) enabled */
    /* additional parameters for the different demos */
    EC_T_DWORD          dwMasterInstanceId;             /* master instance id */
    EC_T_BOOL           bPerfMeasEnabled;               /* performance measurement for jobs enabled */
    EC_T_BOOL           bPerfMeasShowCyclic;            /* show performance values cyclically  */
    EC_T_WORD           bFlash;                         /* flashing output */
    EC_T_WORD           wFlashSlaveAddr;                /* flashing output slave station address */
    EC_T_BOOL           bReadMasterOd;                  /* read master object directory example enabled */
    struct {
        EC_T_DWORD dwOffset;                            /* process data bit offset */
        EC_T_DWORD dwSize;                              /* process data bit size */
        EC_T_DWORD dwValue;                             /* process data to set */
        EC_T_DWORD dwDuration;                          /* duration of how long process data should be set in msec. 0 == forever */
    }                   SetProcessDataBits;             /* process data access helper struct */
    EC_T_DWORD          dwNotifyCode;                   /* notification code for ecatNotifyApp */
    EC_T_NOTIFYPARMS    NotifyParms;                    /* notification parameter for ecatNotifyApp */
    /* EC-Daq */
    EC_T_BOOL           bDaqRecorder;                   /* recorder config file enabled */
    EC_T_CHAR           szDaqRecorder[256];             /* recorder config filename string */
    /* EC-Simulator */
    EC_T_DWORD          dwSimulatorInstanceId;          /* simulator instance id */
    EC_T_BOOL           bDisableProcessDataImage;       /* don't allocate Process Data Image at EC-Simulator (Master ENI / Simulator ENI mismatch support) */
    EC_T_BOOL           bConnectHcGroups;               /* auto-connect floating Hot Connect groups / disconnect all Hot Connect groups */
    /* DS402 */
#if (defined EC_SIMULATOR_DS402)
    EC_T_DWORD          dwDS402NumSlaves;               /* number of DS402 simulated slaves */
    EC_T_WORD           awDS402SlaveAddr[DEMO_MAX_NUM_OF_AXIS]; /* station fixed adresses of DS402 simulated slaves*/
#endif
} T_EC_DEMO_APP_PARMS;

/* demo application context */
typedef struct _T_EC_DEMO_APP_CONTEXT
{
    T_EC_DEMO_APP_PARMS       AppParms;                 /* demo application parameter */
    EC_T_LOG_PARMS            LogParms;                 /* logging parameter */
    EC_T_DWORD                dwInstanceId;             /* instance id */
    EC_T_VOID*                pvJobTaskEvent;           /* job task event */
    EC_T_BOOL                 bJobTaskRunning;          /* job task running flag */
    EC_T_BOOL                 bJobTaskShutdown;         /* job task shutdown request flag */
    class CEmNotification*    pNotificationHandler;     /* notification handler */
    EC_T_VOID*                pvCycFrameReceivedEvent;  /* cyclic frame received event */
    EC_T_TSC_MEAS_DESC        TscMeasDesc;              /* performance measurement descriptor */
    struct _T_MY_APP_DESC*    pMyAppDesc;               /* my app descriptor */
    struct _T_MASTER_RED_DEMO_PARMS* pMasterRedParms;   /* master redundancy parameter */
} T_EC_DEMO_APP_CONTEXT;

/*-GLOBAL VARIABLES-----------------------------------------------------------*/
extern volatile EC_T_BOOL  bRun;                        /* global demo run flag */

/*-FUNCTION DECLARATION------------------------------------------------------*/
EC_T_VOID  ResetAppParms(T_EC_DEMO_APP_CONTEXT* pAppContext, T_EC_DEMO_APP_PARMS* pAppParms);
EC_T_DWORD SetAppParmsFromCommandLine(T_EC_DEMO_APP_CONTEXT* pAppContext, EC_T_CHAR* szCommandLine, T_EC_DEMO_APP_PARMS* pAppParms);
EC_T_VOID  ShowSyntaxCommon(T_EC_DEMO_APP_CONTEXT* pAppContext);

#endif /* INC_ECDEMOPARMS_H */

/*-END OF SOURCE FILE--------------------------------------------------------*/
