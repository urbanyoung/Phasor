#include "Admin.h"
#include "Directory.h"
#include "../Common/MyString.h"
#include "../Common/FileIO.h"
#include <map>
#include <set>
#include <vector>

namespace Admin
{
	static const std::string kAllAccess = "-1";
	class CAccessLevel
	{
	private:
		typedef std::map<int, CAccessLevel> accessList_t;
		typedef std::pair<int, CAccessLevel> pair_t;
		static accessList_t accessList;
		
		std::set<const std::string> commands;
		int level;
		bool all_access;

		static accessList_t CreateAccessList()
		{
			return accessList_t();
		}

	public:

		CAccessLevel(int level, const std::vector<std::string>& commands)
			: level(level), all_access(false)
		{
			for (size_t x = 0; x < commands.size(); x++) {
				const std::string& command = commands[x];
				if (command == kAllAccess) all_access = true;
				else this->commands.insert(command);
			}
		}
		
		static void Add(CAccessLevel& access)
		{
			accessList.insert(pair_t(access.level, access));
		}

		static void Clear()
		{
			accessList.clear();
		}

		bool IsAllowed(const std::string& command)
		{
			return all_access || commands.find(command) != commands.end();
		}

		static bool Find(int level, CAccessLevel** out)
		{
			auto access = accessList.find(level);
			if (access == accessList.end()) return false;
			if (out) *out = &access->second;
			return true;			
		}
	};
	CAccessLevel::accessList_t CAccessLevel::accessList(CAccessLevel::CreateAccessList());

	class CAdmin
	{
	private:
		typedef std::map<std::string, CAdmin> adminList_t;
		typedef std::pair<std::string, CAdmin> pair_t;
		static adminList_t adminList;
		
		//int level; // used to lookup CAccessLevel
		CAccessLevel& accessLevel;
		std::string name, hash;

		static adminList_t CreateAdminList()
		{
			return adminList_t();
		}

	public:
		CAdmin(const std::string& name, const std::string& hash,
			CAccessLevel& accessLevel)
			: name(name), hash(hash), accessLevel(accessLevel)
		{
		}

		static size_t size()
		{
			return adminList.size();
		}

		bool IsAllowed(const std::string& command)
		{
			return accessLevel.IsAllowed(command);
		}

		std::string GetName() { return name; }

		static void Add(CAdmin admin)
		{
			adminList.insert(pair_t(admin.hash, admin));
		}

		static void Remove(const std::string& hash)
		{
			auto itr = adminList.find(hash);
			if (itr != adminList.end())
				adminList.erase(itr);
		}

		static void Clear()
		{
			adminList.clear();
		}

		static bool GetAdminFromName(const std::string& name, CAdmin** out)
		{
			auto itr = adminList.begin();
			while (itr != adminList.end())
			{
				if (itr->second.name == name) {
					if (out) *out = &itr->second;
					return true;
				}
			}
			return false;
		}

		static bool GetAdmin(const std::string& hash, CAdmin** out)
		{
			auto admin = adminList.find(hash);
			if (admin == adminList.end()) return false;
			if (out) *out = &admin->second;
			return true;
		}
	};
	CAdmin::adminList_t CAdmin::adminList(CAdmin::CreateAdminList());

	// --------------------------------------------------------------------
	result_t Add(const std::string& hash, const std::string& authname, int level)
	{
		CAccessLevel* accessLevel = 0;

		if (CAdmin::GetAdmin(hash, NULL)) return E_HASH_INUSE;
		if (CAdmin::GetAdminFromName(authname, NULL)) return E_NAME_INUSE;		
		if (!CAccessLevel::Find(level, &accessLevel)) return E_LEVEL_NOT_EXIST;

		CAdmin admin(authname, hash, *accessLevel);
		CAdmin::Add(admin);
		return E_OK;
	}

	void Remove(const std::string& hash)
	{
		CAdmin::Remove(hash);
	}

	bool IsAdmin(const std::string& hash)
	{
		return CAdmin::GetAdmin(hash, NULL);
	}

	result_t CanUseCommand(const std::string& hash, const std::string& command,
		std::string* authName)
	{
		if (CAdmin::size() == 0) return E_OK; // hash system inactive
		CAdmin* admin = 0;
		if (!CAdmin::GetAdmin(hash, &admin)) return E_NOT_ADMIN;
		if (authName) *authName = admin->GetName();
		return admin->IsAllowed(command) ? E_OK : E_NOT_ALLOWED;
	}

	// --------------------------------------------------------------------
	//
	void LoadAccessList(const std::wstring& waccessPath)
	{
		std::string accessPath = NarrowString(waccessPath);

		char outBuffer[4096] = {0};
		DWORD count = GetPrivateProfileSectionNames(outBuffer, NELEMS(outBuffer),
			accessPath.c_str());

		if (count > 0) {
			DWORD processedCount = 0;//characters	
			while (processedCount < count - 1)
			{
				char dataBuffer[8192] = {0};
				int level = atoi(outBuffer + processedCount);

				DWORD dataCount = GetPrivateProfileString(outBuffer + processedCount,
					"data", "", dataBuffer, NELEMS(dataBuffer), accessPath.c_str());

				std::vector<std::string> tokens = 
					Tokenize<std::string>(dataBuffer, ", ");

				CAccessLevel accessLevel(level, tokens);
				CAccessLevel::Add(accessLevel);

				processedCount += strlen(outBuffer + processedCount) + 1;			
			}
		}
	}

	void LoadAdminList(const std::wstring adminPath, COutStream* out)
	{
		CInFile file;
		if (!file.Open(adminPath))
			return; // may not exist

		char line[4096];
		int n = 1;
		while (file.ReadLine<char>(line, NELEMS(line), NULL)) {
			std::vector<std::string> tokens = Tokenize<std::string>(line,",");
			if (tokens.size() == 3) {
				int level = atoi(tokens[2].c_str());
				result_t result = Add(tokens[1], tokens[0], level);
				if (result == E_LEVEL_NOT_EXIST && out != NULL) {
					*out << adminPath << L" : invalid access level (line "
						<< n << L")" << endl; 
				}
			} else if (out) {
				*out << adminPath << L" : line " << n << 
					L" is incorrectly formatted." << endl;
			}
			n++;
		}
	}

	void Initialize(COutStream* out)
	{
		// read from files
		CAdmin::Clear();
		CAccessLevel::Clear();

		std::wstring adminPath = g_ProfileDirectory + L"admin.txt";
		std::wstring accessPath = g_ProfileDirectory + L"access.ini";

		LoadAccessList(accessPath);
		LoadAdminList(adminPath, out);
	}

	void Reload()
	{
		Initialize(NULL);
	}
}