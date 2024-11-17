///////////////////////////////////////////////////////////////////////////////
//
// Module Name:
//
//     timezoneapi.c
//
// Abstract:
//
//     Functions related to timezones
//
// Author:
//
//     lordarathres2 (11-17-2024)
//
// Revision History:
//
//     lordarathres2               11-17-2024  Initial creation.
//
///////////////////////////////////////////////////////////////////////////////

#include "buildcfg.h"
#include "kexdllp.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0])) 
// This is for added compatibility with the Wine functions, 
// probably the only time it'll be used is here

KEXAPI DWORD WINAPI EnumDynamicTimeZoneInformation(
	IN	DWORD index,
	OUT	DYNAMIC_TIME_ZONE_INFORMATION *info)
{
	DYNAMIC_TIME_ZONE_INFORMATION tz;
	LSTATUS ret;
	DWORD size;
	static HKEY tz_key;

	if (!info) return ERROR_INVALID_PARAMETER;

	size = ARRAY_SIZE(tz.TimeZoneKeyName);
	ret = RegEnumKeyExW( tz_key, index, tz.TimeZoneKeyName, &size, NULL, NULL, NULL, NULL );
	if (ret) return ret;

	tz.DynamicDaylightTimeDisabled = TRUE;
	if (!GetTimeZoneInformationForYear( 0, &tz, (TIME_ZONE_INFORMATION *)info )) return GetLastError();

	StringCchCopyW(info->TimeZoneKeyName, ARRAY_SIZE(info->TimeZoneKeyName), tz.TimeZoneKeyName);
	info->DynamicDaylightTimeDisabled = FALSE;
	return 0;
}

KEXAPI DWORD WINAPI GetDynamicTimeZoneInformationEffectiveYears(
	IN	const DYNAMIC_TIME_ZONE_INFORMATION *info,
	OUT	DWORD *first, DWORD *last)
{
	HKEY key, dst_key = 0;
	DWORD type, count, ret = ERROR_FILE_NOT_FOUND;
	static HKEY tz_key;

	if (RegOpenKeyExW( tz_key, info->TimeZoneKeyName, 0, KEY_ALL_ACCESS, &key )) return ret;

	if (RegOpenKeyExW( key, L"Dynamic DST", 0, KEY_ALL_ACCESS, &dst_key )) goto done;
	count = sizeof(DWORD);
	if (RegQueryValueExW( dst_key, L"FirstEntry", NULL, &type, (BYTE *)first, &count )) goto done;
	if (type != REG_DWORD) goto done;
	count = sizeof(DWORD);
	if (RegQueryValueExW( dst_key, L"LastEntry", NULL, &type, (BYTE *)last, &count )) goto done;
	if (type != REG_DWORD) goto done;
	ret = 0;

done:
	RegCloseKey( dst_key );
	RegCloseKey( key );
	return ret;
}