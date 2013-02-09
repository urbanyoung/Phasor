#include "Admin.h"
#include "Directory.h"
#include "../Common/MyString.h"
#include "../Common/FileIO.h"
#include <map>
#include <set>
#include <vector>

/*! \todo
 * when adding sv_admin_add etc force a recheck of players in the server
 */
namespace Admin
{
	namespace access
	{
		static const std::string kAllAccess = "-1";
		class s_access_level
		{
			std::set<const std::string> commands;
			int level;
			bool all_access;
		public:
			s_access_level(int level, const std::vector<std::string>& commands)
				: level(level), all_access(false)
			{
				for (size_t x = 0; x < commands.size(); x++) {
					const std::string& command = commands[x];
					if (command == kAllAccess) all_access = true;
					else this->commands.insert(command);
				}
			}

			bool is_allowed(const std::string& command) const {
				return all_access || commands.find(command) != commands.end();
			}

			int get_level() const { return level; }
		};

		typedef std::map<int, s_access_level> access_t;
		typedef std::pair<int, s_access_level> pair_t;
		access_t accessList;

		void add(const s_access_level& access) {
			accessList.insert(pair_t(access.get_level(), access));
		}

		void clear() { accessList.clear(); }

		bool find(int level, s_access_level** out) {
			auto access = accessList.find(level);
			if (access == accessList.end()) return false;
			if (out) *out = &access->second;
			return true;	
		}
	}

	namespace admin
	{
		struct s_admin
		{
			const access::s_access_level& accessLevel;
			std::string name, hash;

			s_admin(const std::string& name, const std::string& hash,
				const access::s_access_level& accessLevel)
				: name(name), hash(hash), accessLevel(accessLevel)
			{
			}

			bool is_allowed(const std::string& command) const {
				return accessLevel.is_allowed(command);
			}

		};

		typedef std::map<std::string, s_admin> adminList_t;
		typedef std::pair<std::string, s_admin> pair_t;
		adminList_t adminList;

		size_t size() { return adminList.size(); }
		
		void add(const s_admin& admin) { 
			adminList.insert(pair_t(admin.hash, admin));
		}

		void remove(const std::string& hash) {
			auto itr = adminList.find(hash);
			if (itr != adminList.end())	adminList.erase(itr);
		}

		void clear() { adminList.clear(); }

		bool find_admin_by_name(const std::string& name, s_admin** out)
		{
			auto itr = adminList.begin();
			while (itr != adminList.end()) {
				if (itr->second.name == name) {
					if (out) *out = &itr->second;
					return true;
				}
			}
			return false;
		}

		bool find_admin_by_hash(const std::string& hash, s_admin** out) {
			auto admin = adminList.find(hash);
			if (admin == adminList.end()) return false;
			if (out) *out = &admin->second;
			return true;
		}
	}

	// --------------------------------------------------------------------
	//
	result_t Add(const std::string& hash, const std::string& authname, int level)
	{
		access::s_access_level* accessLevel = 0;

		if (admin::find_admin_by_hash(hash, NULL)) return E_HASH_INUSE;
		if (admin::find_admin_by_name(authname, NULL)) return E_NAME_INUSE;		
		if (!access::find(level, &accessLevel)) return E_LEVEL_NOT_EXIST;

		admin::s_admin admin(authname, hash, *accessLevel);
		admin::add(admin);
		return E_OK;
	}

	void Remove(const std::string& hash)
	{
		admin::remove(hash);
	}

	bool IsAdmin(const std::string& hash)
	{
		return admin::find_admin_by_hash(hash, NULL);
	}

	result_t CanUseCommand(const std::string& hash, const std::string& command,
		std::string* authName)
	{
		if (admin::size() == 0) return E_OK; // hash system inactive
		admin::s_admin* admin = 0;
		if (!admin::find_admin_by_hash(hash, &admin)) return E_NOT_ADMIN;
		if (authName) *authName = admin->name;
		return admin->is_allowed(command) ? E_OK : E_NOT_ALLOWED;
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

				access::s_access_level accessLevel(level, tokens);
				access::add(accessLevel);

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
		admin::clear();
		access::clear();

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