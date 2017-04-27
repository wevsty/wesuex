#pragma once
#include "general_define.h"
#include "win_token_class.hpp"
#include "win_sid_class.hpp"
#include "auto_delete_point.hpp"
#include "win_report_error.hpp"
#include <windows.h>
//CreateToolhelp32Snapshot
#include <Tlhelp32.h>
#include <userenv.h>
#pragma comment(lib, "userenv.lib")

//获取进程的PID
DWORD GetProcessPID(const wchar_t *FileName)
{
	HANDLE hProcessSnapshot;
	PROCESSENTRY32W process_entry;
	process_entry.dwSize = sizeof(PROCESSENTRY32);
	DWORD dwRetPID = 0;
	BOOL bRet;
	//进行进程快照
	hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //TH32CS_SNAPPROCESS快照所有进程
																  //开始进程查找
	bRet = Process32First(hProcessSnapshot, &process_entry);
	//循环比较，得出ProcessID
	while (bRet == TRUE)
	{
		if (wcscmp(FileName, process_entry.szExeFile) == 0)
		{
			dwRetPID = process_entry.th32ProcessID;
			break;
		}
		else
		{
			bRet = Process32Next(hProcessSnapshot, &process_entry);
		}
	}
	CloseHandle(hProcessSnapshot);
	return dwRetPID;
}

//设置当前进程令牌权限
bool SetCurrentProcessPrivilege()
{
	WINAPI_TOKEN current_token;
	if (current_token.GetCurrentProcessToken() == false)
	{
		return false;
	}
	//current_token.SetAllPrivilege(TRUE);
	current_token.SetTokenPrivilege(SE_DEBUG_NAME, TRUE, FALSE);
	current_token.SetTokenPrivilege(SE_IMPERSONATE_NAME, TRUE, FALSE);
	current_token.SetTokenPrivilege(SE_ASSIGNPRIMARYTOKEN_NAME, TRUE, FALSE);
	current_token.SetTokenPrivilege(SE_INCREASE_QUOTA_NAME, TRUE, FALSE);
	current_token.SetAllPrivilege(TRUE);
	return true;
}


bool CreateNewProcess_As_User(HANDLE hToken,
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	//WCHAR wszProcessName[MAX_PATH] = L"C:\\Windows\\System32\\Taskmgr.exe";
	PROCESS_INFORMATION ProcInfo = { 0 };
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof(STARTUPINFO);
	GetStartupInfoW(&StartupInfo);
	StartupInfo.wShowWindow = SW_SHOWNORMAL;
	const wchar_t* wsz_porcess_path = NULL;
	if (wstr_porcess_path.empty() == false)
	{
		wsz_porcess_path = wstr_porcess_path.c_str();
	}
	wchar_t* wsz_command_dup = NULL;
	if (wstr_command.length() != 0)
	{
		wsz_command_dup = _wcsdup(wstr_command.c_str());
	}
	const wchar_t* wsz_current_directory = NULL;
	if (wstr_current_directory.empty() == false)
	{
		wsz_current_directory = wstr_current_directory.c_str();
	}
	//模拟用户
	//ImpersonateLoggedOnUser(hToken);
	BOOL bRet = CreateProcessAsUserW(hToken, wsz_porcess_path,
		wsz_command_dup, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		NULL, wsz_current_directory, &StartupInfo, &ProcInfo);
	//恢复模拟
	//RevertToSelf();
	if (bRet == 0)
	{
		//delete[] wsz_command_dup;
		free(wsz_command_dup);
		return false;
	}
	if (ProcInfo.hProcess != 0)
	{
		CloseHandle(ProcInfo.hProcess);
	}
	if (ProcInfo.hThread != 0)
	{
		CloseHandle(ProcInfo.hThread);
	}
	free(wsz_command_dup);
	return true;
}

bool CreateNewProcess_With_Token(HANDLE hToken,
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	PROCESS_INFORMATION ProcInfo = { 0 };
	STARTUPINFO StartupInfo = { 0 };
	StartupInfo.cb = sizeof(STARTUPINFO);
	
	GetStartupInfoW(&StartupInfo);
	StartupInfo.wShowWindow = SW_SHOWNORMAL;
	const wchar_t* wsz_porcess_path = NULL;
	if (wstr_porcess_path.empty() == false)
	{
		wsz_porcess_path = wstr_porcess_path.c_str();
	}
	wchar_t* wsz_command_dup = NULL;
	if (wstr_command.length() != 0)
	{
		wsz_command_dup = _wcsdup(wstr_command.c_str());
	}
	const wchar_t* wsz_current_directory = NULL;
	if (wstr_current_directory.empty() == false)
	{
		wsz_current_directory = wstr_current_directory.c_str();
	}
	//模拟用户
	//ImpersonateLoggedOnUser(hToken);
	BOOL bRet = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, wsz_porcess_path,
		wsz_command_dup, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
		NULL, wsz_current_directory, &StartupInfo, &ProcInfo);
	//恢复模拟
	//RevertToSelf();
	if (bRet == 0)
	{
		//delete[] wsz_command_dup;
		free(wsz_command_dup);
		return false;
	}
	if (ProcInfo.hProcess != 0)
	{
		CloseHandle(ProcInfo.hProcess);
	}
	if (ProcInfo.hThread != 0)
	{
		CloseHandle(ProcInfo.hThread);
	}
	//delete[] wsz_command_dup;
	free(wsz_command_dup);
	return true;
}
bool CreateProcess_Used_Token(HANDLE hToken,
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	if (CreateNewProcess_As_User(hToken, wstr_porcess_path, wstr_command, wstr_current_directory) == true)
	{
		return true;
	}
	if (CreateNewProcess_With_Token(hToken, wstr_porcess_path, wstr_command, wstr_current_directory) == true)
	{
		return true;
	}
	return false;
}

HANDLE GetSystemToken()
{
	DWORD dwWinLogonPID = GetProcessPID(L"winlogon.exe");
	WINAPI_TOKEN winlogon_token;
	if (winlogon_token.GetProcessToken(dwWinLogonPID) == false)
	{
		return INVALID_HANDLE_VALUE;
	}
	SECURITY_ATTRIBUTES SecAttr;
	SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecAttr.bInheritHandle = TRUE;//设定安全权限为继承
	SecAttr.lpSecurityDescriptor = NULL;

	WINAPI_TOKEN hNewProcessToken = winlogon_token.Duplicate_Token(&SecAttr, SecurityImpersonation, TokenPrimary);
	if (hNewProcessToken.hToken == 0)
	{
		return INVALID_HANDLE_VALUE;
	}
	hNewProcessToken.SetAllPrivilege(TRUE);
	//CreateNewProcess_With_Token(hNewProcessToken.hToken);
	hNewProcessToken.bAutoClose = false;
	return hNewProcessToken.hToken;
}

//以当前进程创建受限制令牌
HANDLE CreateCurrentProcessRestrictedToken(const vector<WELL_KNOWN_SID_TYPE>& vec_disable_sid)
{
	AUTO_DELETE_POINT auto_delete;
	WINAPI_TOKEN current_token;
	if (current_token.GetCurrentProcessToken() == false)
	{
		return INVALID_HANDLE_VALUE;
	}
	//生成要拒绝的组
	DWORD dw_disable_num = (DWORD)vec_disable_sid.size();
	PSID_AND_ATTRIBUTES p_disable_sid_attr = new SID_AND_ATTRIBUTES[dw_disable_num];
	WIN_SID* p_disable_sid_array = new WIN_SID[dw_disable_num];

	for (DWORD n=0;n<dw_disable_num;n++)
	{
		p_disable_sid_array[n].Create_SID(vec_disable_sid[n]);
		p_disable_sid_attr[n].Sid= p_disable_sid_array[n].pSid_buffer;
		p_disable_sid_attr[n].Attributes = 0;
	}
	auto_delete.push_back(p_disable_sid_array);
	auto_delete.push_back(p_disable_sid_attr);

	//current_token.SetAllPrivilege(TRUE);
	//current_token.SetTokenPrivilege(SE_DEBUG_NAME, TRUE, FALSE);
	WINAPI_TOKEN NewToken = 
		current_token.DuplicateRestrictedToken(
			DISABLE_MAX_PRIVILEGE | LUA_TOKEN,
			dw_disable_num,
			p_disable_sid_attr
		);
	if (NewToken.IsInvalidHandle() == true)
	{
		return INVALID_HANDLE_VALUE;
	}
	
	DWORD ret_length = NULL;
	PTOKEN_USER pTokenUser = NULL;
	//获取令牌对应的用户账户SID
	if (NewToken.GetTokenInfo(TokenUser, (void**)&pTokenUser, &ret_length) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}
	WIN_SID user_sid;
	user_sid.load_sid(pTokenUser->User.Sid);
	auto_delete.push_back(pTokenUser);

	//设置Owner为当前用户
	TOKEN_OWNER token_owner;
	token_owner.Owner=pTokenUser->User.Sid;
	if (NewToken.SetTokenInfo(TokenOwner, &token_owner, sizeof(TOKEN_OWNER)) == false)
	{
		return INVALID_HANDLE_VALUE;
	}	

	//获取令牌的DACL
	PACL pAcl = NULL;
	PTOKEN_DEFAULT_DACL pTokenDacl = NULL;
	if (NewToken.GetTokenInfo(TokenDefaultDacl, (void**)&pTokenDacl, &ret_length) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}
	pAcl = pTokenDacl->DefaultDacl;
	auto_delete.push_back(pTokenDacl);

	//计算新ACL大小
	DWORD New_ACL_Length= ret_length;
	New_ACL_Length += user_sid.length();
	New_ACL_Length += sizeof(ACCESS_ALLOWED_ACE);

	//创建ACL
	PACL pNewAcl = (PACL)new char[New_ACL_Length];
	memset(pNewAcl, 0, New_ACL_Length);
	auto_delete.push_back(pNewAcl);
	//if (RtlCreateAcl(pNewAcl, New_ACL_Length, pAcl->AclRevision) == FALSE)
	if (InitializeAcl(pNewAcl, New_ACL_Length, pAcl->AclRevision)==FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}
	//添加允许ACE
	if (AddAccessAllowedAce(pNewAcl, pAcl->AclRevision, GENERIC_ALL, pTokenUser->User.Sid) == FALSE)
	{
		return INVALID_HANDLE_VALUE;
	}
	//复制允许ACE
	PACCESS_ALLOWED_ACE pTempAce = NULL;
	for (int i = 0; GetAce(pAcl, i, (PVOID*)&pTempAce)!=FALSE; i++)
	{
		//if (RtlEqualSid(AdminSid, &pTempAce->SidStart)) continue;
		if (IsWellKnownSid(&pTempAce->SidStart, WinBuiltinAdministratorsSid) == TRUE)
		{
			continue;
		}
		AddAce(pNewAcl, pAcl->AclRevision, 0,
			pTempAce, pTempAce->Header.AceSize);
	}
	//设置令牌DACL
	New_ACL_Length += sizeof(TOKEN_DEFAULT_DACL);
	TOKEN_DEFAULT_DACL NewTokenDacl = { 0 };
	NewTokenDacl.DefaultDacl = pNewAcl;
	if (NewToken.SetTokenInfo(TokenDefaultDacl, &NewTokenDacl, New_ACL_Length) == false)
	{
		return INVALID_HANDLE_VALUE;
	}
	//NewToken.SetTokenIntegrityLevel(L"S-1-16-8192");

	//CreateNewProcess_With_Token(NewToken.hToken);
	//防止自动关闭句柄
	NewToken.bAutoClose = false;
	return NewToken.hToken;
}

//vec_disable_sid用来标识需要限制的SID
//vector<WELL_KNOWN_SID_TYPE> vec_disable_group;
//vec_disable_group.push_back(WinBuiltinAdministratorsSid);
//vec_disable_group.push_back(WinAuthenticatedUserSid);
bool CreateRestrictedProcess(
	const vector<WELL_KNOWN_SID_TYPE>& vec_disable_sid,
	const wstring& wstr_integrity_level_sid,
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	WINAPI_TOKEN Restricted_Token = CreateCurrentProcessRestrictedToken(vec_disable_sid);
	Restricted_Token.SetTokenIntegrityLevel(wstr_integrity_level_sid);

	if (CreateProcess_Used_Token(Restricted_Token.hToken, wstr_porcess_path, wstr_command, wstr_current_directory) == false)
	{
		report_error(L"CreateRestrictedProcess");
		exit(0);
	}
	return true;
}

bool CreateSystemProcess(
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	WINAPI_TOKEN System_Token = GetSystemToken();

	if (CreateProcess_Used_Token(System_Token.hToken, wstr_porcess_path, wstr_command, wstr_current_directory) == false)
	{
		report_error(L"CreateRestrictedProcess");
		exit(0);
	}
	return true;
}
bool CreateNewProcess_With_Logon(
	const wstring& wstr_domain_name, 
	const wstring& wstr_user_name, 
	const wstring& wstr_password,
	const wstring& wstr_porcess_path,
	const wstring& wstr_command,
	const wstring& wstr_current_directory
)
{
	bool bret = false;
	HANDLE hUserToken = INVALID_HANDLE_VALUE;

	const wchar_t *wsz_apppath = NULL;
	wchar_t *wsz_command = NULL;
	const wchar_t *wsz_current_dir = NULL;

	const wchar_t *wsz_domain_name = NULL;
	const wchar_t *wsz_user_name = NULL;
	const wchar_t *wsz_password = NULL;

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.lpDesktop = NULL;
	si.cb = sizeof(STARTUPINFO);
	LPVOID lpvEnv = NULL;
	if (wstr_domain_name.empty() != true)
	{
		wsz_domain_name = wstr_domain_name.c_str();
	}
	if (wstr_user_name.empty() != true)
	{
		wsz_user_name = wstr_user_name.c_str();
	}
	if (wstr_password.empty() != true)
	{
		wsz_password = wstr_password.c_str();
	}
	//wsz_domain_name = wstr_domain_name.c_str();
	if (!LogonUser(wstr_user_name.c_str(), wstr_domain_name.c_str(), wstr_password.c_str(),
		LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hUserToken))
	{
		bret = false;
		report_error(L"LogonUser");
		goto GO_LOGON_CLEAN_UP;
		//return false;
	}
	
	if (!CreateEnvironmentBlock(&lpvEnv, hUserToken, TRUE))
	{
		bret = false;
		report_error(L"CreateEnvironmentBlock");
		goto GO_LOGON_CLEAN_UP;
	}
	wchar_t wszUserProfile[MAX_PATH];
	DWORD dwSize = ARRAY_SIZE(wszUserProfile);
	if (!GetUserProfileDirectoryW(hUserToken, wszUserProfile, &dwSize))
	{
		//
		report_error(L"GetUserProfileDirectory");
		bret = false;
		goto GO_LOGON_CLEAN_UP;
		//return false;
	}


	if (wstr_porcess_path.empty() != true)
	{
		wsz_apppath = wstr_porcess_path.c_str();
	}
	if (wstr_command.empty() != true)
	{
		wsz_command = _wcsdup(wstr_command.c_str());
	}
	if (wstr_current_directory.empty() != true)
	{
		wsz_current_dir = wstr_current_directory.c_str();
	}

	if (!CreateProcessWithLogonW(wsz_user_name, wsz_domain_name, wsz_password,
		LOGON_WITH_PROFILE, wsz_apppath, wsz_command,
		CREATE_UNICODE_ENVIRONMENT, lpvEnv, wszUserProfile, &si, &pi))
	{
		report_error(L"CreateProcessWithLogonW");
		goto GO_LOGON_CLEAN_UP;
	}
	bret = true;
GO_LOGON_CLEAN_UP:
	if (hUserToken!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hUserToken);
		hUserToken = INVALID_HANDLE_VALUE;
	}
	if (wsz_command != NULL)
	{
		free(wsz_command);
		wsz_command = NULL;
	}
	if (lpvEnv != NULL)
	{
		if (!DestroyEnvironmentBlock(lpvEnv))
		{
			bret = false;
			report_error(L"DestroyEnvironmentBlock");
		}
		lpvEnv = NULL;
	}
	if (pi.hProcess != INVALID_HANDLE_VALUE)
	{
		CloseHandle(pi.hProcess);
		pi.hProcess = INVALID_HANDLE_VALUE;
	}
	if (pi.hThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(pi.hThread);
		pi.hThread = INVALID_HANDLE_VALUE;
	}
	return bret;
}
bool CreateOtherUserProcess(const wstring& domain_name, const wstring& user_name, const wstring& password,
	const wstring& filepath, const wstring& command, const wstring& current_directory)
{
	return CreateNewProcess_With_Logon(domain_name, user_name, password,
		filepath, command, current_directory);
}