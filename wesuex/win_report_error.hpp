#pragma once
#include <windows.h>
#include <wchar.h>

// Formats a message string using the specified message and variable
// list of arguments.
// must be LocalFree(pBuffer);
LPWSTR GetSystemErrorMessage(DWORD dw_error_code)
{
	LPWSTR pBuffer = NULL;
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		dw_error_code,
		0,
		(LPWSTR)&pBuffer,
		0,
		NULL);

	return pBuffer;
}

void report_error(LPWSTR pszFunction, DWORD dwError = GetLastError())
{
	wchar_t szMessage[4096] = { 0 };
	const wchar_t * p_system_error = GetSystemErrorMessage(dwError);
	int len = swprintf_s(szMessage, ARRAYSIZE(szMessage),
		L"%s failed w/err %d\n%s", pszFunction, dwError, p_system_error);
	//GetFormattedMessage
	MessageBoxW(NULL, (len != -1) ? szMessage : pszFunction,L"An error occurred!", MB_ICONERROR);
	if (p_system_error != NULL)
	{
		LocalFree((void*)p_system_error);
	}
}