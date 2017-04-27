#pragma once
#include <string>
#include <vector>
#include <io.h>
#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
using namespace std;

class CONFIG_VALUE
{
	public:
	string config_key;
	string config_value;
};
class CONFIG_VALUEW
{
public:
	wstring config_key;
	wstring config_value;
};
class GetProgramPath
{
public:
	static void GetCurrentProcessPathA(string* pstr_fullpath, string* pstr_path, string* pstr_name)
	{
		if(pstr_fullpath !=NULL)
		{
			char strtemp[MAX_PATH] = { 0 };
			GetModuleFileNameA(NULL, strtemp,MAX_PATH);
			*pstr_fullpath = strtemp;
		}
		if(pstr_name !=NULL)
		{
			char strtemp[MAX_PATH]={0};
			GetModuleFileNameA(NULL,strtemp,MAX_PATH);
			char *p1=PathFindFileNameA(strtemp);
			*pstr_name = p1;
		}
		if(pstr_path !=NULL)
		{
			char strtemp[MAX_PATH]={0};
			GetModuleFileNameA(NULL,strtemp,MAX_PATH);
			char *p1=NULL;
			p1=strrchr(strtemp,'\\');
			if (p1 != NULL)
			{
				*p1 = 0;
				*pstr_path = strtemp;
			}
		}
	}
	static void GetCurrentProcessPathW(wstring* pstr_fullpath, wstring* pstr_path, wstring* pstr_name)
	{
		if (pstr_fullpath != NULL)
		{
			wchar_t strtemp[MAX_PATH] = { 0 };
			GetModuleFileNameW(NULL, strtemp,MAX_PATH);
			*pstr_fullpath = strtemp;
		}
		if (pstr_name != NULL)
		{
			wchar_t strtemp[MAX_PATH]={0};
			GetModuleFileNameW(NULL,strtemp,MAX_PATH);
			wchar_t *p1=PathFindFileNameW(strtemp);
			*pstr_name = p1;
		}
		if (pstr_path != NULL)
		{
			wchar_t strtemp[MAX_PATH]={0};
			GetModuleFileNameW(NULL,strtemp,MAX_PATH);
			wchar_t *p1=NULL;
			p1=wcsrchr(strtemp,'\\');
			if (p1 != NULL)
			{
				*p1 = 0;
				*pstr_path = strtemp;
			}
		}
	}
};

class INICONFIG
{
public:
	INICONFIG()
	{

	}
	INICONFIG(const wstring& config_full_path, const wstring& config_name = L"default.ini")
	{
		SetConfigPath(config_full_path, config_name);
	}
	INICONFIG(const string& config_full_path, const string& config_name = "default.ini")
	{
		SetConfigPath(config_full_path, config_name);
	}
	bool PathIsExistFile(const wstring &wstr_path)
	{
		if (wstr_path.empty() == true)
		{
			return false;
		}
		bool bret = true;
		FILE* fp=NULL;
		errno_t err = _wfopen_s(&fp,wstr_path.c_str(), L"r");
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
	bool SetConfigPath(const wstring& config_full_path, const wstring& config_name=L"default.ini")
	{
		wstring wstr_full_path = config_full_path;
		if (PathIsExistFile(wstr_full_path) == false)
		{
			wstr_full_path = GetConfigPath_form_name(config_name);
			if (PathIsExistFile(wstr_full_path) == false)
			{
				//找不到配置文件
				return false;
			}
		}
		ConfigFilePathW = wstr_full_path;
		return true;
	}
	bool SetConfigPath(const string& config_full_path, const string& config_name = "default.ini")
	{
		string str_full_path = config_full_path;
		if (PathIsExistFile(str_full_path) == false)
		{
			str_full_path = GetConfigPath_form_name(config_name);
			if (PathIsExistFile(str_full_path) == false)
			{
				//找不到配置文件
				return false;
			}
		}
		ConfigFilePath = str_full_path;
		return true;
	}
	string GetConfigPath_form_name(const string& configfilename)
	{
		string ret_str;
		GetProgramPath::GetCurrentProcessPathA(NULL, &ret_str,NULL);
		if (configfilename[0]=='\\')
		{
			ret_str +=configfilename;
		}
		else
		{
			ret_str +='\\';
			ret_str +=configfilename;
		}
		return ret_str;
	}
	wstring GetConfigPath_form_name(const wstring& configfilename)
	{
		wstring ret_str;
		GetProgramPath::GetCurrentProcessPathW(NULL, &ret_str, NULL);
		if (configfilename[0] == L'\\')
		{
			ret_str += configfilename;
		}
		else
		{
			ret_str += L'\\';
			ret_str += configfilename;
		}
		return ret_str;
	}
	string ReadString(const char *appvalue, const char *configkey)
	{
		char buffer[MAX_PATH] = { 0 };
		GetPrivateProfileStringA(appvalue, configkey, NULL, buffer, MAX_PATH, ConfigFilePath.c_str());
		string retstring = buffer;
		return retstring;
	}
	wstring ReadString(const wchar_t *appvalue, const wchar_t *configkey)
	{
		wchar_t buffer[MAX_PATH] = { 0 };
		GetPrivateProfileStringW(appvalue, configkey, NULL, buffer, MAX_PATH, ConfigFilePathW.c_str());
		wstring retstring = buffer;
		return retstring;
	}
	int ReadInt(const char *appvalue, const char *configkey)
	{
		return GetPrivateProfileIntA(appvalue, configkey, 0, ConfigFilePath.c_str());;
	}
	int ReadInt(const wchar_t *appvalue, const wchar_t *configkey)
	{
		return GetPrivateProfileIntW(appvalue, configkey, 0, ConfigFilePathW.c_str());;
	}
	vector<CONFIG_VALUE> ReadAppSection(const char* SectionName)
	{
		vector<CONFIG_VALUE> AppSection;
		char buffer[4096 * 4] = { 0 };
		GetPrivateProfileStringA(SectionName, NULL, NULL, buffer, sizeof(buffer), ConfigFilePath.c_str());
		char *pbuffer = buffer;
		while (strlen(pbuffer) != 0)
		{
			CONFIG_VALUE tempkey;
			char bufferkey[MAX_PATH] = { 0 };
			GetPrivateProfileStringA(SectionName, pbuffer, NULL, bufferkey, sizeof(bufferkey), ConfigFilePath.c_str());
			tempkey.config_key = pbuffer;
			tempkey.config_value = bufferkey;
			AppSection.push_back(tempkey);
			pbuffer += strlen(pbuffer) + 1;
		}
		return AppSection;
	}
	vector<CONFIG_VALUEW> ReadAppSection(const wchar_t *SectionName)
	{
		vector<CONFIG_VALUEW> AppSection;
		wchar_t buffer[4096 * 4] = { 0 };
		GetPrivateProfileStringW(SectionName, NULL, NULL, buffer, sizeof(buffer), ConfigFilePathW.c_str());
		wchar_t *pbuffer = buffer;
		while (wcslen(pbuffer) != 0)
		{
			CONFIG_VALUEW tempkey;
			wchar_t bufferkey[MAX_PATH] = { 0 };
			GetPrivateProfileStringW(SectionName, pbuffer, NULL, bufferkey, sizeof(bufferkey), ConfigFilePathW.c_str());
			tempkey.config_key = pbuffer;
			tempkey.config_value = bufferkey;
			AppSection.push_back(tempkey);
			pbuffer += wcslen(pbuffer) + 1;
		}
		return AppSection;
	}
	wstring replace_string(const wstring& src,const wstring& replace,const wstring& replace_to)
	{
		wstring wstr_ret = src;
		wstring::size_type n_start = src.find(replace);
		if (n_start != wstring::npos)
		{
			wstr_ret.replace(n_start, replace.length(), replace_to);
		}
		return wstr_ret;
	}
	string replace_string(const string& src, const string& replace, const string& replace_to)
	{
		string str_ret = src;
		string::size_type n_start = src.find(replace);
		if (n_start != wstring::npos)
		{
			str_ret.replace(n_start,replace.length(), replace_to);
		}
		return str_ret;
	}
private:
	string ConfigFilePath;
	wstring ConfigFilePathW;
};
