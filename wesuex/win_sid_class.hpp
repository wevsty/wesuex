#pragma once
#include <Windows.h>
#include <Sddl.h>
#include <string>
using std::wstring;
#define MAX_WINSID_LENGTH 1000
class WIN_SID
{
public:
	BYTE SID_buffer[SECURITY_MAX_SID_SIZE];
	//SID WIN_SID_STRUCT;
	const PSID pSid_buffer = SID_buffer;
	size_t sid_length;
	WIN_SID():pSid_buffer(SID_buffer), sid_length(0)
	{
		return;
	}
	WIN_SID(PSID pSid) :pSid_buffer(SID_buffer), sid_length(0)
	{
		sid_length=GetLengthSid(pSid);
		memcpy(pSid_buffer, pSid, sid_length);
	}

	void load_sid(PSID pSid)
	{
		sid_length = GetLengthSid(pSid);
		memcpy(pSid_buffer, pSid, sid_length);
	}

	bool wstring_to_sid(const wstring& wstring_sid)
	{
		PSID pSid = NULL;
		if (ConvertStringSidToSidW(wstring_sid.c_str(), &pSid) == 0)
		{
			return false;
		}
		memcpy(pSid_buffer, pSid, sizeof(SID));
		LocalFree(pSid);
		return true;
	}

	wstring sid_to_wstring()
	{
		PSID pSid = pSid_buffer;
		wchar_t wsz_buffer[MAX_WINSID_LENGTH] = { 0 };
		wchar_t *p_wsz_buffer = NULL;
		if (ConvertSidToStringSidW(pSid, &p_wsz_buffer) == 0)
		{
			//return false;
			return L"";
		}
		wstring ws_ret = p_wsz_buffer;
		LocalFree(p_wsz_buffer);
		//return true;
		return ws_ret;
	}
	bool GetSID_from_Name(const wstring& system_name, const wstring& account_name,wstring &domain_name)
	{
		//SID_NAME_USE sid_type;
		SID_NAME_USE sid_type;
		DWORD dw_sid_size = SECURITY_MAX_SID_SIZE;
		
		DWORD dw_domain_name_size = 0;
		LookupAccountNameW(
			system_name.c_str(), account_name.c_str(),
			NULL, &dw_sid_size, NULL, &dw_domain_name_size, &sid_type);
		WCHAR* pwsz_domain_name= new WCHAR[dw_domain_name_size];

		if (LookupAccountNameW(
			system_name.c_str(), account_name.c_str(),
			pSid_buffer, &dw_sid_size, pwsz_domain_name, &dw_domain_name_size, &sid_type) == FALSE)
		{
			delete[] pwsz_domain_name;
			return false;
		}
		domain_name = pwsz_domain_name;
		delete[] pwsz_domain_name;
		return true;
	}
	DWORD length()
	{
		PSID pSid = pSid_buffer;
		return GetLengthSid(pSid);
	}
	bool Create_SID(WELL_KNOWN_SID_TYPE WellKnownSidType)
	{
		sid_length = SECURITY_MAX_SID_SIZE;
		if (!CreateWellKnownSid(WellKnownSidType, NULL, pSid_buffer, (DWORD*)&sid_length))
		{
			return false;
		}
		return true;
	}
};
