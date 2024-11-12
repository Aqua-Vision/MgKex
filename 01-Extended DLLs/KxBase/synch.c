///////////////////////////////////////////////////////////////////////////////
//
// Module Name:
//
//     synch.c
//
// Abstract:
//
//     Contains functions related to thread synchronization.
//
// Author:
//
//     vxiiduu (11-Feb-2022)
//
// Environment:
//
//     Win32 mode.
//
// Revision History:
//
//     vxiiduu              11-Feb-2022  Initial creation.
//     lordarathres2        12/11/2024 - Implemented GetOverlappedResultEx from Shorthorn Project, with permission
//
///////////////////////////////////////////////////////////////////////////////

#include "buildcfg.h"
#include "kxbasep.h"

//
// This function is a wrapper around (Kex)RtlWaitOnAddress.
//
KXBASEAPI BOOL WINAPI WaitOnAddress(
	IN	VOLATILE VOID	*Address,
	IN	PVOID			CompareAddress,
	IN	SIZE_T			AddressSize,
	IN	DWORD			Milliseconds OPTIONAL)
{
	NTSTATUS Status;
	PLARGE_INTEGER TimeOutPointer;
	LARGE_INTEGER TimeOut;

	TimeOutPointer = BaseFormatTimeOut(&TimeOut, Milliseconds);

	Status = KexRtlWaitOnAddress(
		Address,
		CompareAddress,
		AddressSize,
		TimeOutPointer);

	BaseSetLastNTError(Status);
	
	if (NT_SUCCESS(Status) && Status != STATUS_TIMEOUT) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL KXBASEAPI GetOverlappedResultEx(
	IN HANDLE hFile,
	IN OVERLAPPED *lpOverlapped,
	OUT DWORD *lpNumberOfBytesTransferred,
	IN DWORD dwMilliseconds,
	IN BOOL bAlertable)
{
	NTSTATUS status;
	DWORD ret;

	status = (NTSTATUS)lpOverlapped->Internal;
	if (status == STATUS_PENDING)
	{
		if (!dwMilliseconds)
		{
			SetLastError(ERROR_IO_INCOMPLETE);
			return FALSE;
		}
		ret = WaitForSingleObjectEx(
			lpOverlapped->hEvent ? 
			lpOverlapped->hEvent : 
			hFile, dwMilliseconds, bAlertable);
		if (ret == WAIT_FAILED)
			return FALSE;
		else if (ret)
		{
			SetLastError(ret);
			return FALSE;
		}

		status = (NTSTATUS)lpOverlapped->Internal;
		if (status == STATUS_PENDING) status = STATUS_SUCCESS;
	}

	*lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;
	
	if (!NT_SUCCESS(status))
	{
		BaseSetLastNTError(status);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
