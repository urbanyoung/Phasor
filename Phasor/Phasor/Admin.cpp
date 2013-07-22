#include "Admin.h"
#include "Directory.h"
#include "../Common/MyString.h"
#include "../Common/FileIO.h"
#include "Commands.h"
#include "Halo/Game/Game.h"
#include <map>
#include <set>
#include <vector>

namespace Admin
{
	std::wstring adminPath;
	std::wstring accessPath;
	bool challenge_admins = true;

	bool isChallengeEnabled() { return challenge_admins; }

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

			const std::set<const std::string>& 
				getCommands() { return commands; }

			bool allAccess() const { return all_access; }

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
				itr++;
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
	result_t add(const std::string& hash, const std::string& authname, int level)
	{
		access::s_access_level* accessLevel = 0;

		if (admin::find_admin_by_hash(hash, NULL)) return E_HASH_INUSE;
		if (admin::find_admin_by_name(authname, NULL)) return E_NAME_INUSE;		
		if (!access::find(level, &accessLevel)) return E_LEVEL_NOT_EXIST;

		admin::s_admin admin(authname, hash, *accessLevel);
		admin::add(admin);
		return E_OK;
	}

	void remove(const std::string& hash)
	{
		admin::remove(hash);
	}

	bool isAdmin(const std::string& hash)
	{
		return admin::find_admin_by_hash(hash, NULL);
	}

	bool getLevel(const std::string& hash, int* level)
	{
		Admin::admin::s_admin* admin = 0;
		if (!admin::find_admin_by_hash(hash, &admin)) return false;
		*level = admin->accessLevel.get_level();
		return true;
	}

	result_t canUseCommand(const std::string& hash, const std::string& command,
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
				result_t result = add(tokens[1], tokens[0], level);
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

	bool SaveAdminList(COutStream& stream)
	{
		std::string data;
		data.reserve(1 << 12); // 4kb

		for (auto itr = admin::adminList.cbegin(); itr != admin::adminList.cend();
			++itr)
		{
			data += itr->second.name + ",";
			data += itr->second.hash + ",";
			data += m_sprintf("%i\r\n", itr->second.accessLevel.get_level());
		}

		std::wstring adminPathTmp = adminPath + L".tmp";

		CTempFile<COutFile> file;
		if (!file.Open(adminPathTmp, GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ, OPEN_ALWAYS)) {
			stream << L"can't open admin file : " << adminPath << endl;
			return false;
		}

		DWORD written;
		if (!file.Write((BYTE*)data.c_str(), data.size(), &written)) {
			stream << "can't write to new admin file" << endl;
			return false;
		}

		file.Close();

		if (!CFile::Move(adminPathTmp, adminPath, true)) {
			stream << "can't replace old admin file " << adminPath << endl;
			return false;
		}
		file.success();
		return true;
	}

	void initialize(COutStream* out)
	{
		// read from files
		admin::clear();
		access::clear();

		adminPath = g_ProfileDirectory + L"admin.txt";
		accessPath = g_ProfileDirectory + L"access.ini";

		LoadAccessList(accessPath);
		LoadAdminList(adminPath, out);
	}

	void reload()
	{
		initialize(NULL);
	}

	// --------------------------------------------------------------------
	//
	void MarkPlayerAdmin(const std::string& hash, bool is_admin)
	{
		halo::s_player* player = halo::game::getPlayerFromHash(hash);
		if (!player) return;
		player->is_admin = is_admin;
		*player->console_stream << "You're " << (is_admin ? "an admin now." : "no longer an admin.")
			<< endl;
	}

	e_command_result sv_admin_add(void*, commands::CArgParser& args, COutStream& out)
	{
		std::string hash = args.ReadPlayerOrHash();
		std::string name = args.ReadString();
		int level = args.ReadInt();

		result_t result = add(hash, name, level);
		switch (result)
		{
		case E_HASH_INUSE:
			{
				out << L"That hash is already in use." << endl;
			} break;
		case E_NAME_INUSE:
			{
				out << L"That name is already in use." << endl;
			} break;
		case E_LEVEL_NOT_EXIST:
			{
				out << level << L" is not a valid access level." << endl;
			} break;
		default:
			{
				if (!SaveAdminList(out)) {
					out << name << " will be an admin until the server restarts, because the file wasn't saved." << endl;
				} else {
					out << name << " can now use admin at level " << level << endl;
					MarkPlayerAdmin(hash, true);
				}
			} break;
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_admin_del(void*, commands::CArgParser& args, COutStream& out)
	{
		std::string name = args.ReadString();
		admin::s_admin* admin;
		
		if (admin::find_admin_by_name(name, &admin)) {
			std::string hash = admin->hash;
			admin::remove(hash);
			if (!SaveAdminList(out)) {
				out << name << " will still be an until once the server restarts because the file wasn't saved." << endl;
			} else {
				out << name << " is no longer an admin. " << endl;
				MarkPlayerAdmin(hash, false);
			}
		} else out << name << " isn't an admin." << endl;

		return e_command_result::kProcessed;
	}

	e_command_result sv_admin_list(void*, commands::CArgParser& args, COutStream& out)
	{
		out << "List of current admins: " << endl;
		for (auto itr = admin::adminList.cbegin(); itr != admin::adminList.cend();
			++itr)
		{
			out.print("%-19s Level: %i", itr->second.name.c_str(), itr->second.accessLevel.get_level());
		}
		return e_command_result::kProcessed;
	}

	e_command_result sv_admin_cur(void*, commands::CArgParser& args, COutStream& out)
	{
		out << "Admins currently in the server: " << endl;
		for (int i = 0; i < 16; i++) {
			halo::s_player* player = halo::game::getPlayer(i);
			if (player && player->is_admin) {
				admin::s_admin* admin;
				if (admin::find_admin_by_hash(player->hash, &admin)) {
					out << player->mem->playerName << " authed as '" << 
						admin->name << "'" << endl;
				} else {
					out << player->mem->playerName << " (temporary admin)." << endl;
				}
			}
		}
		out << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_admin_reload(void*, commands::CArgParser& args, COutStream& out)
	{
		initialize(&out);
		return e_command_result::kProcessed;
	}

	e_command_result sv_commands(void* player1, commands::CArgParser& args, COutStream& out)
	{
		if (!player1) {
			out << "You can use all commands." << endl;
			return e_command_result::kProcessed;
		}

		halo::s_player* player = (halo::s_player*)player1;

		int level = 0;
		if (!getLevel(player->hash, &level)) {
			out << "The admin system isn't enabled, you can use any command" << endl;
			return e_command_result::kProcessed;
		}

		access::s_access_level* lvl = 0;
		if (find(level, &lvl)) {
			auto commands = lvl->getCommands();

			if (!lvl->allAccess())	{
				out << "You can use the following commands:" << endl;
				int i = 0;
				for (auto itr = commands.begin(); itr != commands.end(); ++itr, i++) {
					if (i % 4 == 3) out << endl;
					out << *itr << "   ";
				}
			} else out << "You can use all commands." << endl;
		}
		out << endl;

		return e_command_result::kProcessed;
	}

	e_command_result sv_admin_check(void*, commands::CArgParser& args, COutStream& out)
	{
		challenge_admins = args.ReadBool();
		out << "admin challenging is " << (challenge_admins ? "enabled" : "disabled") << endl;
		return e_command_result::kProcessed;
	}

	e_command_result sv_public(void*, commands::CArgParser& args, COutStream& out)
	{
		// sv_public only actually works before the server starts, but meh
		bool enable = args.ReadBool();
		if (!enable) challenge_admins = false;
		return e_command_result::kGiveToHalo;
	}

}