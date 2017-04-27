#pragma once

#include <Windows.h>
#include "win_sid_class.hpp"
//#include <UserEnv.h>
//#pragma comment(lib, "userenv.lib")
class WINAPI_TOKEN
{
public:
	HANDLE hToken;
	bool bAutoClose;
	WINAPI_TOKEN() :hToken(INVALID_HANDLE_VALUE), bAutoClose(true)
	{

	}
	WINAPI_TOKEN(HANDLE input_token) :hToken(INVALID_HANDLE_VALUE), bAutoClose(true)
	{
		hToken = input_token;
	}
	WINAPI_TOKEN(const WINAPI_TOKEN & input_token_class) :hToken(INVALID_HANDLE_VALUE), bAutoClose(true)
	{
		hToken = input_token_class.hToken;
	}
	~WINAPI_TOKEN()
	{
		Close();
	}
	WINAPI_TOKEN& operator=(const WINAPI_TOKEN& old_token)
	{
		bAutoClose = false;
		hToken = old_token.hToken;
		return *this;
	}
	WINAPI_TOKEN& operator=(const HANDLE old_token)
	{
		hToken = old_token;
		return *this;
	}
	void Close()
	{
		if (bAutoClose == false)
		{
			return;
		}
		if (hToken != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hToken);
			hToken = INVALID_HANDLE_VALUE;
		}
	}
	void Clear()
	{
		hToken = INVALID_HANDLE_VALUE;
	}
	void SetHandle(const HANDLE input_handle)
	{
		hToken= input_handle;
	}
	HANDLE GetHandle()
	{
		return hToken;
	}
	bool IsInvalidHandle()
	{
		if(hToken == INVALID_HANDLE_VALUE)
		{
			return true;
		}
		return false;
	}
	bool GetProcessToken(DWORD dwPID)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);
		if (hProcess == NULL)
		{
			//error
			return false;
		}
		if (OpenProcessToken(hProcess, MAXIMUM_ALLOWED, &hToken) == FALSE)
		{
			CloseHandle(hProcess);
			return false;
		}
		else
		{
			CloseHandle(hProcess);
			return true;
		}
	}
	bool GetCurrentProcessToken()
	{
		if (OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &hToken) == FALSE)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	///*
	BOOL DuplicateTokenEx(
		_In_ DWORD dwDesiredAccess,
		_In_opt_ LPSECURITY_ATTRIBUTES lpTokenAttributes,
		_In_ SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
		_In_ TOKEN_TYPE TokenType,
		_Outptr_ PHANDLE phNewToken
	)
	{
		return ::DuplicateTokenEx(hToken, dwDesiredAccess, lpTokenAttributes, ImpersonationLevel, TokenType, phNewToken);
	}
	//*/
	HANDLE Duplicate_Token(
		_In_opt_  LPSECURITY_ATTRIBUTES lpTokenAttributes,//安全属性
		_In_  SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,//模拟级别
		_In_ TOKEN_TYPE TokenType//令牌类型
	)//lpTokenAttributes(0),ImpersonationLevel(SecurityImpersonation),TokenType(TokenPrimary)
	{
		HANDLE hNewToken;
		if (::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, lpTokenAttributes,
			ImpersonationLevel, TokenType, &hNewToken) == TRUE)
		{
			return hNewToken;
		}
		else
		{
			//return false;
			return 0;
		}
	}

	bool SetTokenInfo(
		//__in HANDLE TokenHandle,
		__in TOKEN_INFORMATION_CLASS TokenInformationClass,
		__in_bcount(TokenInformationLength) LPVOID TokenInformation,
		__in DWORD TokenInformationLength
	)
	{
		if (SetTokenInformation(hToken, TokenInformationClass
			, TokenInformation, TokenInformationLength) == FALSE)
		{
			return false;	
		}
		else
		{
			return true;
		}
	}

	BOOL GetTokenInfo(
		_In_      TOKEN_INFORMATION_CLASS TokenInformationClass,
		_Out_opt_ void**                  TokenInformation,
		_Out_     PDWORD                  ReturnLength
	)
	{
		//获取需要的缓冲区大小
		GetTokenInformation(hToken, TokenInformationClass, NULL, NULL, ReturnLength);

		*TokenInformation = new char[*ReturnLength];
		if (GetTokenInformation(hToken, TokenInformationClass, *TokenInformation, *ReturnLength, ReturnLength) == FALSE)
		{
			return FALSE;
		}
		return TRUE;
	}
	/*
	BOOL EnableTokenPrivilege(HANDLE hPrivilegeToken, LPCWSTR lpszPrivilegeName, BOOL bEnable)
	{
	HANDLE hNewhPrivilegeTokenToken = hPrivilegeToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	if (hNewhPrivilegeTokenToken == NULL)
	{
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
	TOKEN_QUERY | TOKEN_READ, &hNewhPrivilegeTokenToken))
	return FALSE;
	}
	if (!LookupPrivilegeValue(NULL, lpszPrivilegeName, &luid))
	return TRUE;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;
	AdjustTokenPrivileges(hNewhPrivilegeTokenToken, FALSE, &tp, NULL, NULL, NULL);
	if (hToken == NULL)
	{
	CloseHandle(hNewhPrivilegeTokenToken);
	}
	return (GetLastError() == ERROR_SUCCESS);
	}
	*/
	BOOL SetTokenPrivilege(
		HANDLE hToken,          // access token handle
		LPCWSTR lpszPrivilege,  // name of privilege to enable/disable
		BOOL bEnablePrivilege,   // to enable or disable privilege
		BOOL bRemovePrivilege
	)
	{
		TOKEN_PRIVILEGES tp;
		LUID luid;
		if (!LookupPrivilegeValue(
			NULL,            // lookup privilege on local system
			lpszPrivilege,   // privilege to lookup
			&luid))        // receives LUID of privilege
		{
			//printf("LookupPrivilegeValue error: %u\n", GetLastError() );
			return FALSE;
		}
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if (bEnablePrivilege == TRUE)
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		else if (bRemovePrivilege == TRUE)
			tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
		else
			tp.Privileges[0].Attributes = 0;
		// Enable the privilege or disable all privileges.
		if (!AdjustTokenPrivileges(
			hToken,
			FALSE,
			&tp,
			sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)NULL,
			(PDWORD)NULL))
		{
			//printf("AdjustTokenPrivileges error: %u\n", GetLastError() );
			return FALSE;
		}
		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			//printf("The token does not have the specified privilege. \n");
			return FALSE;
		}
		return TRUE;
	}
	BOOL SetTokenPrivilege(
		LPCWSTR lpszPrivilege,  // name of privilege to enable/disable
		BOOL bEnablePrivilege,   // to enable or disable privilege
		BOOL bRemovePrivilege
	)
	{
		return SetTokenPrivilege(hToken, lpszPrivilege, bEnablePrivilege, bRemovePrivilege);
	}
	BOOL RemoveTokenPrivilege(HANDLE hPrivilegeToken, LPCWSTR lpszPrivilege, BOOL bDisable)
	{
		return SetTokenPrivilege(hToken, lpszPrivilege, FALSE, TRUE);
	}
	BOOL RemoveTokenPrivilege(LPCWSTR lpszPrivilege, BOOL bDisable)
	{	
		return SetTokenPrivilege(hToken, lpszPrivilege, FALSE, TRUE);
	}
	BOOL EnableTokenPrivilege(LPCWSTR lpszPrivilegeName)
	{
		return SetTokenPrivilege(hToken, lpszPrivilegeName,TRUE, FALSE);
	}
	BOOL SetAllPrivilege(BOOL bEnablePrivilege)// to enable or remove privilege
	{
		DWORD dwSetAttributes = SE_PRIVILEGE_ENABLED;
		if (bEnablePrivilege != TRUE)
		{
			dwSetAttributes = SE_PRIVILEGE_REMOVED;
		}
		DWORD dw_length = NULL;
		PTOKEN_PRIVILEGES p_tps = NULL;
		//获取特权信息
		if (GetTokenInfo(TokenPrivileges, (void**)&p_tps, &dw_length) == FALSE)
		{
			return FALSE;
		}
		//设置特权信息
		for (DWORD i = 0; i < p_tps->PrivilegeCount; i++)
		{
			p_tps->Privileges[i].Attributes = dwSetAttributes;
		}
		if (!AdjustTokenPrivileges(hToken, FALSE, p_tps, dw_length, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		{
			return FALSE;
		}
		return TRUE;
	}
	BOOL SetSafePrivilege(BOOL bEnablePrivilege)// to enable or remove privilege
	{
		BOOL bRemovePrivilege = FALSE;
		if (bEnablePrivilege != TRUE)
		{
			bEnablePrivilege = FALSE;
			bRemovePrivilege = TRUE;
		}
		SetTokenPrivilege(SE_CREATE_TOKEN_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_INCREASE_QUOTA_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_TCB_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_SECURITY_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_LOAD_DRIVER_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_SHUTDOWN_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_DEBUG_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_AUDIT_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_REMOTE_SHUTDOWN_NAME, bEnablePrivilege, bRemovePrivilege);
		SetTokenPrivilege(SE_IMPERSONATE_NAME, bEnablePrivilege, bRemovePrivilege);
		return TRUE;
	}
	//设置令牌完整性等级
	bool SetTokenIntegrityLevel(const wstring& wszIntegritySid)
	{
		if (wszIntegritySid.empty() == true)
		{
			return false;
		}
		WIN_SID IntegritySid;
		if (IntegritySid.wstring_to_sid(wszIntegritySid) == false)
		{
			return false;
		}
		TOKEN_MANDATORY_LABEL TIL = { 0 };
		TIL.Label.Attributes = SE_GROUP_INTEGRITY;
		TIL.Label.Sid = IntegritySid.pSid_buffer;
		if (SetTokenInfo(TokenIntegrityLevel, &TIL, sizeof(TOKEN_MANDATORY_LABEL) + IntegritySid.length()) == false)
		{
			return false;
		}
		return true;
	}
	HANDLE DuplicateRestrictedToken(
		_In_ DWORD Flags= DISABLE_MAX_PRIVILEGE| LUA_TOKEN,
		DWORD DisableSidCount = 0,
		PSID_AND_ATTRIBUTES SidsToDisable=NULL
	)
	{
		HANDLE hNewToken = INVALID_HANDLE_VALUE;
		if (::CreateRestrictedToken(hToken, Flags,
			DisableSidCount, SidsToDisable, NULL, NULL, NULL, NULL, &hNewToken) == FALSE)
		{
			return INVALID_HANDLE_VALUE;
		}
		return hNewToken;
	}
	bool GetUserToken(
		__in        LPCWSTR lpszUsername,
		__in_opt    LPCWSTR lpszDomain,
		__in        LPCWSTR lpszPassword,
		__in        DWORD dwLogonType,
		__in        DWORD dwLogonProvider)
	{
		if (wcslen(lpszDomain) == 0)
		{
			lpszDomain = NULL;
		}
		if (LogonUserW(lpszUsername, lpszDomain, lpszPassword,
			dwLogonType, dwLogonProvider
			, &hToken) == FALSE)
		{
			return false;
		}
		else
		{
			EnableTokenPrivilege(SE_RESTORE_NAME);
			EnableTokenPrivilege(SE_BACKUP_NAME);
			//CreateEnvironmentBlock(&lpEnvironment, hToken, TRUE);
			return true;
		}
	}
};