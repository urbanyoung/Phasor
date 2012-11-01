#include "Scripting.h"
#include <map>
#include <string>
#include <sstream>
#include <windows.h> // for GetCurrentProcessId()

// todo: overload Call for no args
// check what happens if attempt to call non existant lua function
// 
namespace Scripting
{
	#define exception_type std::exception // todo: change to custom type
	#define DEFAULT_TIMEOUT 2000

	const uint_32 versions[] = {1234};

	std::string scriptsDir;
	std::map<std::string, Lua::State*> scripts;

	struct PhasorAPI
	{
		std::vector<Lua::Object*> (*func)(Lua::State*, std::vector<Lua::Object*>&);
		const char* name;
	};

	std::vector<Lua::Object*> testf(Lua::State*, std::vector<Lua::Object*>&)
	{
		return std::vector<Lua::Object*>();
	}

	PhasorAPI PhasorExportTable[] =
	{
		{&testf, "updateammo"}
	};

	// Registers the Phasor API with scripts
	void RegisterFunctions(Lua::State* state);
	// Checks if the script is compatible with this version of Phasor.
	void CheckScriptCompatibility(Lua::State* state, const char* script);

	// --------------------------------------------------------------------
	//
	// 
	
	// Checks if the specified function exists before calling it
	// Return: TRUE (exists) FALSE (doesn't exist).
	// Throws exceptions in same way as Call
	bool CheckCall(Lua::State* state, const char* func,  
		const std::vector<Lua::Object*>& args,
		std::vector<Lua::Object*>& rets, int timeout=DEFAULT_TIMEOUT) 
	{
		if (!state->HasFunction(func)) return false;
		rets = state->Call(func, args, timeout);
		return true;
	}

	bool CheckCall(Lua::State* state, const char* func, int timeout=DEFAULT_TIMEOUT)
	{
		if (!state->HasFunction(func)) return false;
		state->Call(func, timeout);
		return true;
	}

	void OpenScript(const char* script)
	{
		// Make sure the script isn't already loaded
		if (scripts.find(script) != scripts.end()) {
			std::stringstream err;
			err << "script: " << script << " is already loaded.";
			throw std::exception(err.str().c_str());
		}
		std::stringstream abs_file;
		abs_file << scriptsDir << "\\" << script << ".lua";

		Lua::State* state = Lua::State::NewState();

		try 
		{
			state->DoFile(abs_file.str().c_str());
			
			RegisterFunctions(state);
			CheckScriptCompatibility(state, script);			

			// Notify the script that it's been loaded.
			std::vector<Lua::Object*> args;
			args.push_back(state->NewNumber(GetCurrentProcessId()));

			if (!CheckCall(state, "OnScriptLoad")) {
				std::stringstream err;
				err << "script: " << script << " cannot be loaded, OnScriptLoad undefined.";
				throw std::exception(err.str().c_str());
			}

			scripts[script] = state;
		}
		catch (exception_type) 
		{
			Lua::State::Close(state);
			throw;
		}	
	}

	void CloseScript(const char* script) 
	{
		// todo: Check how Call can fail and make sure all failures
		// are handled here.
		std::map<std::string, Lua::State*>::iterator itr = scripts.find(script);

		if (itr != scripts.end()) {
			Lua::State* state = itr->second;
			try 
			{
				CheckCall(state, "OnScriptUnload");
				Lua::State::Close(state);
			}
			catch (exception_type) 
			{
				Lua::State::Close(state);	
				throw;
			}					
		}
	}

	// Sets the path to be used by this namespace (where scripts are).
	void SetPath(const char* scriptPath)
	{
		scriptsDir = scriptPath;
	}

	// Registers the Phasor API with scripts
	void RegisterFunctions(Lua::State* state)
	{
		const int n = sizeof(PhasorExportTable)/sizeof(PhasorAPI);
		for (int i = 0; i < n; i++)
			state->RegisterFunction(PhasorExportTable[i].name, PhasorExportTable[i].func);
	}

	// Checks if the specified version is compatible
	bool CompatibleVersion(uint_32 required) 
	{
		const int n = sizeof(versions)/sizeof(versions[0]);
		bool compatible = false;
		for (int i = 0; i < n; i++)
		{
			if (versions[i] == required) {
				compatible = true;
				break;
			}
		}
		return compatible;		
	}

	// Checks if the script is compatible with this version of Phasor.
	void CheckScriptCompatibility(Lua::State* state, const char* script) 
	{
		std::vector<Lua::Object*> args, vals;
		if (!CheckCall(state, "GetRequiredVersion", args, vals)) {
			std::stringstream err;
			err << "script: " << script << " cannot be loaded, GetRequiredVersion undefined.";
			throw std::exception(err.str().c_str());
		}

		// Make sure the requested version is compatible
		bool is_compatible = false;
		if (vals.size() == 1) {
			Lua::Number* ver = (Lua::Number* )vals[0];
			if (ver->GetType() == Lua::Type_Number &&
				CompatibleVersion((uint_32)ver->GetValue())) {
					is_compatible = true;
			}
		}

		if (!is_compatible) {
			std::stringstream err;
			err << "script: " << script << " is not compatible with this version of Phasor.";
			throw std::exception(err.str().c_str());
		}
	}

	/*std::vector<Lua::Object*> Call(const char* function, 
		const std::vector<Lua::Object*>& args)
	{
		return std
	}*/
	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	Object::Object(obj_type _type) : type(_type) 
	{
	}

	Object::~Object()
	{
	}

	obj_type Object::GetType()
	{
		return type;
	}

	// --------------------------------------------------------------------
	//
	
	ObjBool::ObjBool(bool b) : Object(TYPE_BOOL)
	{
		this->b = b;
	}

	ObjBool::ObjBool(Lua::Boolean* b) : Object(TYPE_BOOL)
	{
		if (b->GetType() != Lua::Type_Boolean) {
			std::stringstream err;
			err << __FUNCTION__ << " expects boolean types, not " << b->GetType();
			throw std::exception(err.str().c_str());
		}
		this->b = b->GetValue();
	}

	ObjBool::ObjBool(const ObjBool& other) : Object(TYPE_BOOL)
	{
		this->b = other.b;
	}

	ObjBool & ObjBool::operator=(const ObjBool &rhs) 
	{
		this->b = rhs.b;
		return *this;
	}

	// --------------------------------------------------------------------
	//
	
	ObjNumber::ObjNumber(int value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(uint_32 value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(double value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(float value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(Lua::Number* value) : Object(TYPE_NUMBER)
	{
		if (value->GetType() != Lua::Type_Boolean) {
			std::stringstream err;
			err << __FUNCTION__ << " expects number types, not " << value->GetType();
			throw std::exception(err.str().c_str());
		}
		this->value = value->GetValue();
	}

	ObjNumber::ObjNumber(const ObjNumber& other) : Object(TYPE_NUMBER)
	{
		this->value = other.value;
	}

	ObjNumber& ObjNumber::operator=(const ObjNumber& rhs)
	{
		this->value = rhs.value;
		return *this;
	}

	// --------------------------------------------------------------------
	//
	
	ObjString::ObjString(const char* str) : Object(TYPE_STRING)
	{
		CopyString(str);
	}

	ObjString::ObjString(Lua::String* str) : Object(TYPE_STRING)
	{
		if (str->GetType() != Lua::Type_String) {
			std::stringstream err;
			err << __FUNCTION__ << " expects string types, not " << str->GetType();
			throw std::exception(err.str().c_str());
		}
		CopyString(str->GetValue());
	}

	ObjString::ObjString(const ObjString& other) : Object(TYPE_STRING)
	{
		CopyString(other.str);
	}

	ObjString::~ObjString()
	{
		delete[] str;
	}

	ObjString& ObjString::operator=(const ObjString& rhs) 
	{
		CopyString(rhs.str);
		return *this;
	}

	void ObjString::CopyString(const char* str)
	{
		this->str = new char [strlen(str) + 1];
		strcpy(this->str, str);
	}

	// --------------------------------------------------------------------
	//
	
	ObjTable::ObjTable(const std::map<std::string, std::string>& table) : Object(TYPE_TABLE)
	{
		using namespace std;
		map<string, string>::const_iterator itr = table.begin();
		while (itr != table.end())
		{
			ObjString* key = new ObjString(itr->first.c_str());
			ObjString* value = new ObjString(itr->second.c_str());
			this->table.insert(pair<Object*, Object*>(key, value));
			itr++;
		}
	}

	ObjTable::ObjTable(Lua::Table* table) : Object(TYPE_TABLE)
	{
		if (table->GetType() != Lua::Type_Table) {
			std::stringstream err;
			err << __FUNCTION__ << " expects table types, not " << table->GetType();
			throw std::exception(err.str().c_str());
		}

		std::map<Lua::Object*, Lua::Object*> tbl_map = table->GetMap();

		std::map<Lua::Object*, Lua::Object*>::const_iterator itr = tbl_map.begin();
		while (itr != tbl_map.end())
		{
			/*switch (itr->)
			{

			}*/
			itr++;
		}
	}

	ObjTable::~ObjTable() 
	{
		std::map<Object*, Object*>::iterator itr = table.begin();
		while (itr != table.end())
		{
			delete itr->first;
			delete itr->second;
			itr = table.erase(itr);
		}
	}

	Object* ObjTable::ConvertObject(Lua::Object* obj)
	{
		Object* out = NULL;
		switch (obj->GetType())
		{
		case Lua::Type_Nil:
			out = new Object(TYPE_NIL);
			break;
		case Lua::Type_Boolean:
			out = new ObjBool((Lua::Boolean*)obj);
			break;

		}
	}

}