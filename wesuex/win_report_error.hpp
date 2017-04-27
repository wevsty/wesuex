#pragma once
#include <windows.h>
#include <wchar.h>
void report_error(LPWSTR pszFunction, DWORD dwError = GetLastError())
{
	wchar_t szMessage[1024] = { 0 };
	int len = swprintf_s(szMessage, ARRAYSIZE(szMessage),L"%s failed w/err 0x%08lx", pszFunction, dwError);
	MessageBox(NULL, (len != -1) ? szMessage : pszFunction,L"An error occurred!", MB_ICONERROR);
}