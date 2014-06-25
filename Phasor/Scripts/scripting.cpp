#include "scripting.hpp"
#include "../Phasor/Version.h"
#include "script-events.h"
#include "PhasorAPI/phasorapi.h"
#include "../Common/FileIO.h"
#include "../Phasor/Globals.h"
#include "../Common/Timers.h"
#include <array>

namespace scripting {

	std::array<size_t, 3> compatibleVersions = {200, 201, 202};
	static const std::string log_prefix = "   ";

	// only care about its memory address
	const char PhasorScript::thisKey = 0;

	//
	// --------------------------------------------------------------
	//
    
	PhasorScript& PhasorScript::get(lua_State* L) {
		lua_pushlightuserdata(L, (void *)&thisKey); 
		lua_gettable(L, LUA_REGISTRYINDEX);
		PhasorScript* pThis = static_cast<PhasorScript*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
		if (!pThis) luaL_error(L, "cannot find reference to this! (phasor bug)");
		return *pThis;
	}

	void PhasorScript::block(std::string f) {
		blockedFunctions.insert(std::move(f));
	}

	bool PhasorScript::isBlocked(const std::string& f) const {
		return blockedFunctions.find(f) != blockedFunctions.end();
	}

	//
	// --------------------------------------------------------------
	//
	size_t getRequiredVersion(lua::State& state) {
		boost::optional<size_t> required = state.getGlobal<phlua::PhasorPop, size_t>("required_version");
		if (required) return *required;
		if (!state.hasFunction("GetRequiredVersion"))
			throw std::exception("both required_version and GetRequiredVersion undefined");

		size_t version;
		phlua::Caller<size_t> c(state);
		std::tie(version) = c.call("GetRequiredVersion", std::make_tuple());
		if (c.hasError())
			throw std::exception("cannot determine required version");

		return version;
	}

	void checkCompatibility(lua::State& state) {
		size_t required = getRequiredVersion(state);
		for (size_t v : compatibleVersions) {
			if (v == required) return;
		}

		throw std::exception("script is not compatible with this version of Phasor");
	}

	//
	// --------------------------------------------------------------
	//

	ScriptHandler::ScriptHandler(std::string scriptDir, COutStream& errStream)
		: scriptDir(std::move(scriptDir)), errStream(errStream)
	{}

	bool ScriptHandler::isLoaded(const std::string& script) {
		for (auto itr = scripts.begin(); itr != scripts.end(); ++itr) {
			if ((*itr)->getName() == script) return true;
		}
		return false;
	}

	bool ScriptHandler::loadScript(const std::string& name, bool persistent, std::shared_ptr<PhasorScript>& out)
	{
		std::string file = scriptDir + name + ".lua";

		try {

			std::shared_ptr<PhasorScript> script = PhasorScript::create(persistent,
                                                                        file,
                                                                        name,
                                                                        phasorapi::funcTable.begin(),
                                                                        phasorapi::funcTable.end());

			lua::State& state = script->getState();
			checkCompatibility(state);

			if (state.hasFunction("OnScriptLoad")) {
				phlua::Caller<> c(state);
				c.call("OnScriptLoad", std::make_tuple(GetCurrentProcessId(), PHASOR_HALO_BUILDA, persistent));
			}

			// Blacklist any functions that aren't defined
			for (auto itr = events::eventList.begin();
				 itr != events::eventList.end(); ++itr)
			{
				if (!state.hasFunction(itr->c_str()))
					script->block(*itr);
			}

			out = script;

		} catch (std::exception& e) {
			NoFlush _(errStream);
			errStream << L"script '" << name << "' cannot be loaded." <<
				endl << log_prefix << e.what() << endl;
			return false;
		}

		return true;
	}

	bool ScriptHandler::loadScript(const std::string& name, bool persistent)
	{
        if (isLoaded(name)) {
            errStream << name << " is already loaded." << endl;
            return false;
        }
		std::shared_ptr<PhasorScript> script;
		if (!loadScript(name, persistent, script)) return false;
		scripts.push_back(script);
		return true;
	}

	void ScriptHandler::loadPersistentScripts() {
		std::wstring persistent = WidenString(scriptDir) + L"\\persistent\\*.lua";

		std::list<std::wstring> files;
		NDirectory::FindFiles(persistent, files);

		for (auto itr = files.cbegin(); itr != files.cend(); ++itr) {
			std::wstring script = L"persistent\\" + *itr;
			// remove .lua
			script = script.substr(0, script.size() - 4);
			loadScript(NarrowString(script).c_str(), true);
		}
	}

	void ScriptHandler::unloadScript(PhasorScript& script) {
		lua::State& state = script.getState();
		if (state.hasFunction("OnScriptUnload")) {
			phlua::Caller<> c(state);
			c.call("OnScriptUnload", std::make_tuple());
		}
	}

	bool ScriptHandler::unloadScript(const std::string& script) {
		for (auto itr = scripts.begin(); itr != scripts.end(); ) {
			if ((*itr)->getName() == script) {
				unloadScript(**itr);	
				itr = scripts.erase(itr);
				return true;
			}
			++itr;
		}
		return false;
	}

	void ScriptHandler::unloadAllScripts(bool includePersistent) {
		for (auto itr = scripts.begin(); itr != scripts.end(); ) {
			if (includePersistent || !(*itr)->isPersistent()) {
				unloadScript(**itr);
				itr = scripts.erase(itr);
			} else ++itr;
		}
	}

	bool ScriptHandler::reloadScript(const std::string& script)	{
		for (auto itr = scripts.begin(); itr != scripts.end(); ++itr) {
			if ((*itr)->getName() == script) {
				bool persistent = (*itr)->isPersistent();
				unloadScript(**itr);
				return loadScript(script, persistent, *itr);
			}
		}
		return false;
	}

	void ScriptHandler::reloadAllScripts(bool includePersistent)
	{
		for (auto itr = scripts.begin(); itr != scripts.end(); ++itr) {
			bool persistent = (*itr)->isPersistent();
			if (includePersistent || !persistent) {
				std::string name = (*itr)->getName();
				unloadScript(**itr);
				loadScript(name, persistent, *itr);
			}
		}
	}

    std::vector<std::pair<std::string, bool>> ScriptHandler::getLoadedScripts() const {
        std::vector<std::pair<std::string, bool>> loaded;
		loaded.reserve(scripts.size());
        for (auto itr = scripts.begin(); itr != scripts.end(); ++itr)
            loaded.emplace_back((*itr)->getName(), (*itr)->isPersistent());
		return loaded;
	}

	void ScriptHandler::handleError(PhasorScript& script, const std::string& func,
									const char* what)
	{
		NoFlush _(errStream); // flush once at the end of the func

		errStream << L"Error in '" << script.getName() << L'\'' <<  endl;
		errStream << log_prefix << L"Error: " << what << endl;

		script.block(func);

		errStream << log_prefix << L"Action: Further calls to '" << func <<
				L"' will be ignored." << endl;

		errStream << endl;
	}

}