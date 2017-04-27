#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
//exit()
#include <stdlib.h>
#include <tchar.h>
#include <string>
using namespace std;
#include "win_token_class.hpp"
#include "win_sid_class.hpp"
#include "win_run_process.hpp"
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "win_ini_config.hpp"
#include "general_define.h"
#include "win_os_version.hpp"

//#pragma comment( linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"")
//设置程序为无窗口
#pragma comment(linker, "/subsystem:\"console\" /entry:\"wmainCRTStartup\"")

WCHAR wszProcessName[MAX_PATH] = L"C:\\Windows\\System32\\notepad.exe";
WCHAR wszIntegritySid[] = L"S-1-16-4096";

bool CreateLowProcess()
{
	WINAPI_TOKEN hCurrentProcessToken;
	WINAPI_TOKEN hNewProcessToken;
	if(hCurrentProcessToken.GetCurrentProcessToken()==false)
	{
		return false;
	}
	SECURITY_ATTRIBUTES SecAttr;
	SecAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecAttr.bInheritHandle = TRUE;//设定安全权限为继承
	SecAttr.lpSecurityDescriptor = NULL;
	//DuplicateTokenEx(hCurrentProcessToken.hToken, MAXIMUM_ALLOWED, &SecAttr,
	//	SecurityImpersonation, TokenPrimary, &hNewProcessToken.hToken);
	hNewProcessToken = hCurrentProcessToken.Duplicate_Token(&SecAttr, SecurityImpersonation, TokenPrimary);
	if (hNewProcessToken.hToken == 0)
	{
		return false;
	}
	WIN_SID IntegritySid;
	if (IntegritySid.wstring_to_sid(wszIntegritySid) == false)
	{
		return false;
	}
	TOKEN_MANDATORY_LABEL TIL = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	STARTUPINFO StartupInfo = { 0 };
	ULONG ExitCode = 0;
	TIL.Label.Attributes = SE_GROUP_INTEGRITY;
	TIL.Label.Sid = IntegritySid.pSid_buffer;

	if (hNewProcessToken.SetTokenInfo(TokenIntegrityLevel, &TIL,
		sizeof(TOKEN_MANDATORY_LABEL) + IntegritySid.length())
		== false)
	{
		return false;
	}
	BOOL bRet = CreateProcessAsUser(hNewProcessToken.hToken, NULL,
		wszProcessName, NULL, NULL, FALSE,
		0, NULL, NULL, &StartupInfo, &ProcInfo);
	if (bRet==0)
	{
		return false;
	}
	return true;
}

//stdio.h
bool PathIsExistFile(const wstring &wstr_path)
{
	if (wstr_path.empty() == true)
	{
		return false;
	}
	bool bret = true;
	FILE* fp = NULL;
	errno_t err = _wfopen_s(&fp, wstr_path.c_str(), L"r");
	if (err != 0)
	{
		bret = false;
	}
	if (fp != NULL)
	{
		fclose(fp);
	}
	return bret;
}
bool PathIsExistFile(const string &str_path)
{
	if (str_path.empty() == true)
	{
		return false;
	}
	bool bret = true;
	FILE* fp = NULL;
	errno_t err = fopen_s(&fp, str_path.c_str(), "r");
	if (err != 0)
	{
		bret = false;
	}
	if (fp != NULL)
	{
		fclose(fp);
	}
	return bret;
}
class COMMAND_OPTIONS
{
public:
	wstring m_wstr_shell_execute;
	wstring m_wstr_program_path;
	wstring m_wstr_program_command;
	wstring m_wstr_program_current_directory;
	wstring m_wstr_config_filepath;
	wstring m_wstr_config_start_mode;
	wstring m_wstr_config_low_integrity_sid;
	wstring m_wstr_config_user_name;
	wstring m_wstr_config_user_domain;
	wstring m_wstr_config_user_password;
	vector<WELL_KNOWN_SID_TYPE> m_vec_disable_group;
	INICONFIG m_config;
	COMMAND_OPTIONS(int argc, wchar_t *argv[], wchar_t *envp[])
	{
		load_command(argc, argv, envp);
	}
	void load_command(int argc, wchar_t *argv[], wchar_t *envp[])
	{

		if (argc > 1)
		{
			for (int n_options = 1; n_options < argc; n_options++)
			{
				if (wcsncmp(argv[n_options], L"-h", STRING_LEN_NO_ZERO(L"-h")) == 0)
				{
					print_help();
					exit(0);
					continue;
				}
				if (wcsncmp(argv[n_options], L"-config_file", STRING_LEN_NO_ZERO(L"-config_file")) == 0)
				{
					m_wstr_config_filepath = argv[n_options] + STRING_LEN_NO_ZERO(L"-config_file");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-f", STRING_LEN_NO_ZERO(L"-f")) == 0)
				{
					m_wstr_config_filepath = argv[n_options] + STRING_LEN_NO_ZERO(L"-f");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-program", STRING_LEN_NO_ZERO(L"-program")) == 0)
				{
					m_wstr_program_path = argv[n_options] + STRING_LEN_NO_ZERO(L"-program");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-p", STRING_LEN_NO_ZERO(L"-p")) == 0)
				{
					m_wstr_program_path = argv[n_options] + STRING_LEN_NO_ZERO(L"-p");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-command", STRING_LEN_NO_ZERO(L"-command")) == 0)
				{
					m_wstr_program_command = argv[n_options] + STRING_LEN_NO_ZERO(L"-command");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-cm", STRING_LEN_NO_ZERO(L"-cm")) == 0)
				{
					m_wstr_program_command = argv[n_options] + STRING_LEN_NO_ZERO(L"-cm");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-current_dir", STRING_LEN_NO_ZERO(L"-current_dir")) == 0)
				{
					m_wstr_program_current_directory = argv[n_options] + STRING_LEN_NO_ZERO(L"-current_dir");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-cd", STRING_LEN_NO_ZERO(L"-cd")) == 0)
				{
					m_wstr_program_current_directory = argv[n_options] + STRING_LEN_NO_ZERO(L"-cd");
					continue;
				}
				if (wcsncmp(argv[n_options], L"-sh", STRING_LEN_NO_ZERO(L"-sh")) == 0)
				{
					m_wstr_shell_execute = argv[n_options] + STRING_LEN_NO_ZERO(L"-sh");
					m_wstr_program_path = m_wstr_shell_execute;
					continue;
				}
				
			}
		}
		else
		{
			print_help();

			exit(0);
		}
		
		// load config
		
		//如果没有指定配置文件路径，并且设定了要启动的程序则尝试寻找启动程序对应的配置文件
		if (m_wstr_config_filepath.empty() == true && m_wstr_program_path.empty() != true)
		{
			GetProgramPath::GetCurrentProcessPathW(NULL, &m_wstr_config_filepath, NULL);
			m_wstr_config_filepath += L"\\";
			wstring::size_type n_name_start = m_wstr_program_path.rfind('\\');
			if (n_name_start != wstring::npos)
			{
				wstring wstr_process_name = m_wstr_program_path.substr(n_name_start+1);
				if (wstr_process_name.empty() == false)
				{
					m_wstr_config_filepath += wstr_process_name;
					m_wstr_config_filepath += L".ini";
				}
				else
				{
					m_wstr_config_filepath += L"default.ini";
				}
			}
			else
			{
				m_wstr_config_filepath += L"default.ini";
			}
		}
		//根据配置文件路径加载文件，如果加载失败尝试加载default.ini
		if (m_config.SetConfigPath(m_wstr_config_filepath, L"default.ini") == false)
		{
			printf("ERROR : Couldn't find config file.\n");
			print_help();
			exit(1);
		}
		//设定为shell启动时，强制使用配置文件中的参数
		if (m_wstr_shell_execute.empty() != true)
		{
			m_wstr_program_path.clear();
			m_wstr_program_path = m_config.ReadString(L"START_MODE", L"Program");
			wstring wstr_tmp = m_config.ReadString(L"START_MODE", L"Command");
			m_wstr_program_command = m_config.replace_string(wstr_tmp, L"%command%", m_wstr_shell_execute);
			m_wstr_program_current_directory = m_config.ReadString(L"START_MODE", L"Current_Directory");
		}

		//校验启动路径
		if (m_wstr_program_path.length() == 0 && m_wstr_program_command.length() == 0)
		{
			m_wstr_program_path = m_config.ReadString(L"START_MODE", L"Program");
			m_wstr_program_command = m_config.ReadString(L"START_MODE", L"Command");
			m_wstr_program_current_directory = m_config.ReadString(L"START_MODE", L"Current_Directory");
			if (m_wstr_program_path.length() == 0 && m_wstr_program_command.length() == 0)
			{
				printf("ERROR : Couldn't find start program.\n");
				print_help();
				exit(1);
			}
		}

		if (PathIsExistFile(m_wstr_program_path) == false)
		{
			printf("ERROR : Couldn't find start program.\n");
			exit(1);
		}
		//获取启动模式
		m_wstr_config_start_mode = m_config.ReadString(L"START_MODE", L"MODE");
		if (m_wstr_config_start_mode == L"LOW_PERMISSIONS")
		{
			//获取完整性等级
			m_wstr_config_low_integrity_sid = m_config.ReadString(L"LOW_PERMISSIONS", L"integrity_level_sid");
			//获取要禁用的组
			if (m_config.ReadString(L"LOW_PERMISSIONS", L"Administrators_SID") == L"DISABLE")
			{
				m_vec_disable_group.push_back(WinBuiltinAdministratorsSid);
			}
			if (m_config.ReadString(L"LOW_PERMISSIONS", L"AuthenticatedUser_SID") == L"DISABLE")
			{
				m_vec_disable_group.push_back(WinAuthenticatedUserSid);
			}
		}
		if (m_wstr_config_start_mode == L"LOGIN_WITH_USER")
		{
			m_wstr_config_user_name = m_config.ReadString(L"LOGON_INFO", L"user_name");
			m_wstr_config_user_domain = m_config.ReadString(L"LOGON_INFO", L"user_domain");
			m_wstr_config_user_password = m_config.ReadString(L"LOGON_INFO", L"user_password");
		}
	}

	void print_help()
	{
		printf("program help:\n");
		printf("-h                 display help message.\n");
		printf("-config_file,-f    used config file(default:default.ini).\n");
		printf("-program,-p        execute the program.\n");
		printf("-sh                used shell execute.\n");
		printf("-command,-cm       set execute the command\n");
		printf("-current_dir,-cd   set execute the current dir\n");
		printf("\n");
		printf("for example:\n");
		printf("wesuex.exe -pC:\\Windows\\notepad.exe\n");
		//exit(0);
	}
};

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
#ifdef _DEBUG
	MessageBoxW(NULL, L"wesuex.exe", L"wesuex", MB_OK);
#endif
	//AllocConsole();
	//AttachConsole(ATTACH_PARENT_PROCESS);
	//freopen("CONIN$", "r+t", stdin);
	//freopen("CONOUT$", "w+t", stdout);
	//freopen("CONERR$", "r+t", stderr);
	//HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetStdHandle(STD_OUTPUT_HANDLE, h_out);
	is_supported_os_version();
	//system("PAUSE");
	COMMAND_OPTIONS opts(argc, argv, envp);


	
	//GetSystemToken();
	//StartInteractiveClientProcess(L"hideuser", L"", L"1", wszProcessName);
	
	if (opts.m_wstr_config_start_mode == L"LOW_PERMISSIONS")
	{
		SetCurrentProcessPrivilege();
		wstring wstr_command, wstr_cd;
		CreateRestrictedProcess(opts.m_vec_disable_group, opts.m_wstr_config_low_integrity_sid
			, opts.m_wstr_program_path, opts.m_wstr_program_command, opts.m_wstr_program_current_directory);
		return 0;
	}
	if (opts.m_wstr_config_start_mode == L"SYSTEM")
	{
		SetCurrentProcessPrivilege();
		CreateSystemProcess(opts.m_wstr_program_path,
			opts.m_wstr_program_command,
			opts.m_wstr_program_current_directory);
		return 0;
	}
	if (opts.m_wstr_config_start_mode == L"LOGIN_WITH_USER")
	{
		CreateOtherUserProcess(opts.m_wstr_config_user_domain,
			opts.m_wstr_config_user_name,
			opts.m_wstr_config_user_password,
			opts.m_wstr_program_path,
			opts.m_wstr_program_command,
			opts.m_wstr_program_current_directory);
		return 0;
	}
	//CreateLowProcess();

    return 0;
}

