/*
@Module DemoInline
@StartOf
@File


@file   DemoInline.h
@brief  Inline header file

Inline header file for the example programs

@note
@n       Copyright (C) 2018
@n       acontis technologies GmbH, Germany. All Rights Reserved.

*/

/***************************************************************************************************
*                                 INCLUDES
*/
#include <rtosLib.h>
#include <stdio.h>


/*******************************************************************************
*                                 DEFINES
*/                    
#define DEMO_LOCAL_READY        "DEMO_RTOS_READY"
#define DEMO_REMOTE_READY       "DEMO_HOST_READY"
#define DEMO_PRINTF             printf

#if !defined( TEXT )
#define TEXT( A )               A
#endif

#if !defined( MAX_PATH )
#define MAX_PATH                260
#endif


/*******************************************************************************
*                                 MACROS
*/                    

/*******************************************************************************
*                                 TYPEDEFS
*/                    

/*******************************************************************************
*                                 GLOBALS
*/

/*******************************************************************************
*                                 LOCALS
*/

/*******************************************************************************
*                           FORWARD DECLARATIONS
*/

/*******************************************************************************
*                            GLOBAL FUNCTIONS
*/

UINT32
DemoRtosLibInit(
    BOOL bInit
    )
{
    UINT32 dwRetVal;
    static BOOL s_bRtosLibInitDone = FALSE;

    if( bInit )
    {
        /* Initialize RtosLib */
        DEMO_PRINTF( TEXT("%-30s : "), TEXT("Initialize RtosLibrary") );
        dwRetVal = RtosLibInit();
        if( RTE_SUCCESS != dwRetVal )
        {
            DEMO_PRINTF( TEXT("Error(0x%X)\n"), dwRetVal );
            goto Exit;
        }
        s_bRtosLibInitDone = TRUE;
        DEMO_PRINTF( TEXT("Ok\n") );
    }
    else
    {
        /* De-Initialize RtosLib */
        if( TRUE == s_bRtosLibInitDone )
        {
            DEMO_PRINTF( TEXT("%-30s : "), TEXT("De-initialize RtosLibrary") );
            dwRetVal = RtosLibDeinit();
            if( RTE_SUCCESS != dwRetVal )
            {
                DEMO_PRINTF( TEXT("Error 0x%X\n"), dwRetVal );
                goto Exit;
            }
            DEMO_PRINTF( TEXT("Ok\n") );
            s_bRtosLibInitDone = FALSE;
        }
    }

    dwRetVal = RTE_SUCCESS;

Exit:
    return dwRetVal;
}


#if defined( DEMOFUNCTIONNAME )
UINT32
DemoRun(
    VOID
    )
{
    UINT32 dwRetVal;
    UINT32 dwRes;
    RTOSLIB_HANDLE hEventLocalReady = NULL;
    RTOSLIB_HANDLE hEventRemoteReady = NULL;
    BOOL bShutdown = FALSE;


    /* Wait for demo counterpart */
    DEMO_PRINTF( TEXT("  %-28s : "), TEXT("Waiting for demo counterpart") );
    dwRetVal = RtosCreateEvent( FALSE, FALSE, TEXT(DEMO_LOCAL_READY), &hEventLocalReady );
    if( RTE_SUCCESS != dwRetVal )
    {
        DEMO_PRINTF( TEXT("RtosCreateEvent %s error 0x%X\n"), TEXT(DEMO_LOCAL_READY), dwRetVal );
        goto Exit;
    }
    dwRetVal = RtosCreateEvent( FALSE, FALSE, TEXT(DEMO_REMOTE_READY), &hEventRemoteReady );
    if( RTE_SUCCESS != dwRetVal )
    {
        DEMO_PRINTF( TEXT("RtosCreateEvent %s error 0x%X\n"), TEXT(DEMO_REMOTE_READY), dwRetVal );
        goto Exit;
    }
    dwRetVal = RtosSetEvent( hEventLocalReady );
    if( RTE_SUCCESS != dwRetVal )
    {
        DEMO_PRINTF( TEXT("RtosSetEvent %s error 0x%X\n"), TEXT(DEMO_LOCAL_READY), dwRetVal );
        goto Exit;
    }
    do 
    {
        dwRetVal = RtosWaitForEvent( hEventRemoteReady, RTOS_WAIT_INFINITE );
        switch( dwRetVal )
        {
        case RTE_SUCCESS:
            DEMO_PRINTF( TEXT("Ok\n") );
            break;
        case RTE_ERROR_TIMEOUT:
            break;
        default:
            DEMO_PRINTF( TEXT("RtosWaitForEvent %s error 0x%X\n"), TEXT(DEMO_REMOTE_READY), dwRetVal );
            goto Exit;
            break;
        }
    } while( RTE_ERROR_TIMEOUT == RTE_ERROR_GET_ERROR(dwRetVal) );


    DEMO_PRINTF( TEXT("\n") );
    dwRetVal = DEMOFUNCTIONNAME( &bShutdown );
    DEMO_PRINTF( TEXT("\n") );
    if( RTE_SUCCESS != dwRetVal )
    {
        goto Exit;
    }

    dwRetVal = RTE_SUCCESS;

Exit:
    if( NULL != hEventRemoteReady )
    {
        dwRes = RtosCloseEvent( hEventRemoteReady );
        if( RTE_SUCCESS != dwRes )
        {
            DEMO_PRINTF( TEXT("RtosCloseEvent %s error 0x%X\n"), TEXT(DEMO_REMOTE_READY), dwRes );
            if( RTE_SUCCESS == dwRetVal )
            {
                dwRetVal = dwRes;
            }
        }
    }
    if( NULL != hEventLocalReady )
    {
        dwRes = RtosCloseEvent( hEventLocalReady );
        if( RTE_SUCCESS != dwRes )
        {
            DEMO_PRINTF( TEXT("RtosCloseEvent %s error 0x%X\n"), TEXT(DEMO_LOCAL_READY), dwRes );
            if( RTE_SUCCESS == dwRetVal )
            {
                dwRetVal = dwRes;
            }
        }
    }

    return dwRetVal;
}
#endif /* DEMOFUNCTIONNAME */


VMF_INLINE
VOID
DemoPrintResult(
    UINT32 dwValue
    )
{
    TCHAR  *tszMessage = NULL;
    UINT32 dwMessageSize;
    UINT32 dwMessageSizeUsed;
    UINT32 dwRes;


    DEMO_PRINTF( TEXT("\n") );

    if( RTE_SUCCESS == dwValue )
    {
        DEMO_PRINTF( TEXT("Finished successfully (0x%X)\n"), dwValue );
        goto Exit;
    }

    DEMO_PRINTF( TEXT("Finished with error 0x%X\n"), dwValue );

    /* Allocate memory for error string */
    dwMessageSize = sizeof(TCHAR) * MAX_PATH;
    tszMessage    = (TCHAR*) malloc( dwMessageSize );
    if( NULL == tszMessage )
    {
        DEMO_PRINTF( TEXT("Error - allocating system memory failed for printing details about error 0x%X !\n"), dwValue );
        goto Exit;
    }
    tszMessage[(dwMessageSize/sizeof(TCHAR))-1] = TEXT('\0'); /* Set string end */

    do 
    {
        /* Query text for result code */
        dwMessageSizeUsed = dwMessageSize - sizeof(TCHAR); /* String end is guaranteed */
        dwRes = RtosResultGetModule( dwValue, tszMessage, &dwMessageSizeUsed );
        switch( RTE_ERROR_GET_ERROR(dwRes) )
        {
        case RTE_SUCCESS:
            {
                if( 0 < dwMessageSizeUsed )
                {
                    DEMO_PRINTF( TEXT("Error Module: %s\n"), tszMessage );
                    break;
                }
            } /* no break here */
        case RTE_ERROR_NOT_FOUND:
            {
                DEMO_PRINTF( TEXT("Error Module: -\n") );
            } break;
        case RTE_ERROR_NODATAEXCHANGEBUFFER:
            {
                /* Re-allocate memory for error string */
                dwMessageSize += sizeof(TCHAR) * MAX_PATH;
                tszMessage    = (TCHAR*) realloc( tszMessage, dwMessageSize );
                if( NULL == tszMessage )
                {
                    DEMO_PRINTF( TEXT("Error - reallocating system memory failed for printing details about error 0x%X !\n"), dwValue );
                    goto Exit;
                }
                tszMessage[(dwMessageSize/sizeof(TCHAR))-1] = TEXT('\0'); /* set string end */
            } break;
        default:
            {
                DEMO_PRINTF( TEXT("Unexpected error 0x%X while looking for error 0x%X module!\n"), dwRes, dwValue );
            } break;
        }
    } while ( RTE_ERROR_NODATAEXCHANGEBUFFER == RTE_ERROR_GET_ERROR(dwRes) );


    do 
    {
        /* Query text for result code */
        dwMessageSizeUsed = dwMessageSize - sizeof(TCHAR); /* String end is guaranteed */
        dwRes = RtosResultGetText( dwValue, tszMessage, &dwMessageSizeUsed );
        switch( RTE_ERROR_GET_ERROR(dwRes) )
        {
        case RTE_SUCCESS:
            {
                if( 0 < dwMessageSizeUsed )
                {
                    DEMO_PRINTF( TEXT("Error Text  : %s\n"), tszMessage );
                }
                else
                {
                    DEMO_PRINTF( TEXT("Error Text  : -\n") );
                }
            } break;
        case RTE_ERROR_NOT_FOUND:
            {
                DEMO_PRINTF( TEXT("Error Text  : unknown 0x%X!\n"), dwValue );
            } break;
        case RTE_ERROR_NODATAEXCHANGEBUFFER:
            {
                /* Re-allocate memory for error string */
                dwMessageSize += sizeof(TCHAR) * MAX_PATH;
                tszMessage    = (TCHAR*) realloc( tszMessage, dwMessageSize );
                if( NULL == tszMessage )
                {
                    DEMO_PRINTF( TEXT("Error - reallocating system memory failed for printing details about error 0x%X !\n"), dwValue );
                    goto Exit;
                }
                tszMessage[(dwMessageSize/sizeof(TCHAR))-1] = TEXT('\0'); /* set string end */
            } break;
        default:
            {
                DEMO_PRINTF( TEXT("Unexpected error 0x%X while looking for error 0x%X text!\n"), dwRes, dwValue );
            } break;
        }
    } while ( RTE_ERROR_NODATAEXCHANGEBUFFER == RTE_ERROR_GET_ERROR(dwRes) );

Exit:
    if( NULL != tszMessage )
    {
        free( tszMessage );
        tszMessage = NULL;
    }
    return;
}


/*******************************************************************************
*                            LOCAL FUNCTIONS
*/

/** @endFile */
