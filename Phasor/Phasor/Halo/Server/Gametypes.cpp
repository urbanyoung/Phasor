#include "Gametypes.h"
#include "../../Directory.h"
#include "../../../Common/FileIO.h"
#include "../../../Common/MyString.h"
#include "../Addresses.h"
#include <map>

namespace halo { namespace server { namespace gametypes {
	std::map<std::wstring, std::wstring> gametypes;

	std::wstring normalizeGametype(const std::wstring& gametype)
	{
		std::wstring lower = gametype;
		ToLowercase(lower);
		return lower;
	}

	bool BuildGametypeList()
	{
		std::wstring hdmuPath = g_ProfileDirectory + L"saved\\hdmu.map";
		CInFile file;
		if (!file.Open(hdmuPath)) return false;

		DWORD size = file.GetFileSize(), read;
		std::vector<BYTE> buffer(size);
		if (!file.Read(buffer.data(), size, &read)) return false;

		gametypes.clear(); 

		// build list of gametypes
		LPBYTE listing = buffer.data();
		for (DWORD n = 0; n < size; n += 0x206)
		{
			bool isGametype = listing[n + 0x200] == 1;
			if (!isGametype) continue;

			// make sure the strings are null terminated
			listing[n + 0xFF] = '\0';
			*(wchar_t*)(listing + n + 0x1FF) = L'\0';

			std::string filePath = (char*)(listing + n);
			std::wstring gametype = (wchar_t*)(listing + n + 0x100);

			ToLowercase(gametype);
			std::wstring wFilePath = WidenString(filePath);
			//	wprintf(L"Found gametype %s\n", gametype.c_str());
			gametypes.insert(
				std::pair<std::wstring, std::wstring>(gametype, wFilePath)
				);
		}
		return true;
	}

	bool ReadGametypeData(const std::wstring& gametype, BYTE* out,
		DWORD outSize)
	{
		auto itr = gametypes.find(normalizeGametype(gametype));
		if (itr == gametypes.end()) return false;
		std::wstring& gametypePath = itr->second;
		CInFile file;
		if (!file.Open(gametypePath)) return false;
		DWORD read;
		return file.Read(out, outSize-4, &read);		
	}

	bool IsValidGametype(const std::wstring& gametype)
	{
		return gametypes.find(normalizeGametype(gametype)) != gametypes.end();
	}

}}}