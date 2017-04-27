// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdio.h>
#include <string>
#include <wchar.h>

using std::wstring;

#define STRING_LEN_NO_ZERO(_) (((sizeof (_) / sizeof (*_)) - 1))

static void GetCurrentProcessPathW(wstring* pstr_fullpath, wstring* pstr_path, wstring* pstr_name)
{
	if (pstr_fullpath != NULL)
	{
		wchar_t strtemp[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, strtemp, MAX_PATH);
		*pstr_fullpath = strtemp;
	}
	if (pstr_name != NULL)
	{
		wchar_t strtemp[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, strtemp, MAX_PATH);
		wchar_t *p1 = NULL;
		p1 = wcsrchr(strtemp, '\\');
		if (p1 != NULL)
		{
			*pstr_name = p1 + 1;
		}
	}
	if (pstr_path != NULL)
	{
		wchar_t strtemp[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, strtemp, MAX_PATH);
		wchar_t *p1 = NULL;
		p1 = wcsrchr(strtemp, '\\');
		if (p1 != NULL)
		{
			*p1 = 0;
			*pstr_path = strtemp;
		}
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
#ifdef _DEBUG
	MessageBoxW(NULL, L"debug", L"starts", MB_OK);
#endif
	LPWSTR *wszArgList;
	int n_argc=0;

	//wszArgList = CommandLineToArgvW(GetCommandLineW(), &n_argc);
	wszArgList = CommandLineToArgvW(lpCmdLine, &n_argc);
	if (wszArgList == NULL)
	{
		exit(1);
	}
	wstring wstr_filepath;
	wstring wstr_command;
	BOOL b_hide_gui = FALSE;
	for (int n = 0; n < n_argc; n++)
	{
		wstring wstr_options = wszArgList[n];
		wstring::size_type n_options = wstr_options.find(L"--file",0, STRING_LEN_NO_ZERO(L"--file"));
		if (n_options != wstring::npos)
		{
			wstr_filepath = wstr_options.substr(n_options + STRING_LEN_NO_ZERO(L"--file"));
			continue;
		}
		n_options = wstr_options.find(L"--cmd",0, STRING_LEN_NO_ZERO(L"--cmd"));
		if (n_options != wstring::npos)
		{
			wstr_command = wstr_options.substr(n_options + STRING_LEN_NO_ZERO(L"--cmd"));
			continue;
		}
		n_options = wstr_options.find(L"--wesuex", 0, STRING_LEN_NO_ZERO(L"--wesuex"));
		if (n_options != wstring::npos)
		{
			GetCurrentProcessPathW(NULL, &wstr_filepath, NULL);
			if (wstr_filepath.empty() == true)
			{
				exit(1);
			}
			wstr_filepath += L"\\wesuex.exe";
			wstr_command = L"\"";
			wstr_command += wstr_options.substr(n_options + STRING_LEN_NO_ZERO(L"--wesuex"));
			wstr_command += L"\"";
			#ifndef _DEBUG
			b_hide_gui = TRUE;
			#endif
			//b_hide_gui = TRUE;
			continue;
		}
		n_options = wstr_options.find(L"--hide",0, STRING_LEN_NO_ZERO(L"--hide"));
		if (n_options != wstring::npos)
		{
			b_hide_gui = TRUE;
			continue;
		}
	}
	const wchar_t* wsz_file = NULL;
	const wchar_t* wsz_cmd = NULL;
	int nShow = SW_NORMAL;
	if (wstr_filepath.empty() != true)
	{
		wsz_file = wstr_filepath.c_str();
	}
	if (wstr_command.empty() != true)
	{
		wsz_cmd = wstr_command.c_str();
	}
	if (b_hide_gui == TRUE)
	{
		nShow = SW_HIDE;
	}
	if (wsz_file == NULL&&wsz_cmd == NULL)
	{
		exit(1);
	}

	SHELLEXECUTEINFO shexec_info = { 0 };
	shexec_info.cbSize = sizeof(SHELLEXECUTEINFO);
	shexec_info.fMask = NULL;
	shexec_info.hwnd = NULL;
	shexec_info.lpVerb = NULL;
	shexec_info.lpFile = wsz_file;
	shexec_info.lpParameters = wsz_cmd;
	shexec_info.lpDirectory = NULL;
	//shexec_info.nShow = SW_NORMAL;
	shexec_info.nShow = nShow;
	shexec_info.hInstApp = NULL;

	ShellExecuteExW(&shexec_info);
	if (shexec_info.hProcess != NULL)
	{
		CloseHandle(shexec_info.hProcess);
	}
    return 0;
}
