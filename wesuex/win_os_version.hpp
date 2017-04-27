#pragma once
#include <sdkddkver.h>
#include <windows.h>
#include <VersionHelpers.h>
//exit()
#include <stdlib.h>
#include <stdio.h>

bool is_supported_os_version()
{
	if (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1) == false)
	{
		printf("error : OS Version Not Supported.You need at least Windows 7 with SP1\n");
		exit(1);
		return false;
	}
	return true;
}