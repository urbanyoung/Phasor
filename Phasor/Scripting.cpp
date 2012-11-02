#include "Scripting.h"
#include "Lua.h"
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

	const DWORD versions[] = {1234};

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
			bool fexists = false;
			Caller caller;
			caller.AddArg(GetCurrentProcessId());
			caller.Call(state, "OnScriptLoad", &fexists);

			if (!fexists) {
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
				Caller caller;
				caller.Call(state, "OnScriptUnload");

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
	bool CompatibleVersion(DWORD required) 
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
		bool funcExists = false;
		Caller caller;
		Result result = caller.Call(state, "GetRequiredVersion", &funcExists);

		if (!funcExists) {
			std::stringstream err;
			err << "script: " << script << " cannot be loaded, GetRequiredVersion undefined.";
			throw std::exception(err.str().c_str());
		}

		// Make sure the requested version is compatible
		bool is_compatible = false;
		if (result.size() == 1 && result[0]->GetType() == TYPE_NUMBER) {
			ObjNumber* ver = (ObjNumber*)result[0];
			if (CompatibleVersion((DWORD)ver->GetValue())) {
					is_compatible = true;
			}
		}

		if (!is_compatible) {
			std::stringstream err;
			err << "script: " << script << " is not compatible with this version of Phasor.";
			throw std::exception(err.str().c_str());
		}
	}

	// --------------------------------------------------------------------
	// Class: Caller
	// Provides an interface for passing parameters to scripts.
	// 
	Caller::Caller()
	{
	}

	void Caller::SetData(const Caller& other)
	{
		std::list<Object*>::const_iterator args_itr = other.args.begin();
		while (args_itr != other.args.end())
		{
			args.push_back((*args_itr)->NewCopy());
			args_itr++;
		}
	}

	Caller::Caller(const Caller& other)
	{
		SetData(other);
	}

	void Caller::Free()
	{
		std::list<Object*>::iterator args_itr = args.begin();
		while (args_itr != args.end())
		{
			delete *args_itr;
			args_itr = args.erase(args_itr);
		}

	}

	Caller::~Caller()
	{
		Free();
	}

	Caller& Caller::operator=(const Caller& rhs)
	{
		if (this == &rhs) return *this;
		Free();
		SetData(rhs);
		return *this;
	}
		
	void Caller::AddArg(bool b)
	{
		args.push_back(new ObjBool(b));
	}

	void Caller::AddArg(const char* str)
	{
		args.push_back(new ObjString(str));
	}

	void Caller::AddArg(int value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(DWORD value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(float value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(double value)
	{
		args.push_back(new ObjNumber(value));
	}

	void Caller::AddArg(const std::map<std::string, std::string>& table)
	{
		args.push_back(new ObjTable(table));
	}

	// Calls the function in the specific script.
	// Doesn't automatically append the 'using' parameter
	Result Caller::Call(Lua::State* state, const char* function, bool* found, bool erase)
	{
		Result result = Result();
		if (state->HasFunction(function))
		{
			result = state->Call(function, args, DEFAULT_TIMEOUT);
			if (found) *found = true;
		}
		else if (found) *found = false;

		if (erase) this->Free();
		return result;
	}

	Result Caller::Call(const char* function)
	{
		// This is the argument which indicates if a scripts' return 
		// value is used.
		AddArg(true);
		ObjBool* using_param = (ObjBool*)*args.rbegin();

		using namespace std;
		map<string, Lua::State*>::iterator itr = Scripting::scripts.begin();
		Result result;
		bool result_set = false;

		while (itr != Scripting::scripts.end())
		{
			Lua::State* state = itr->second;
			Result r = Call(state, function, 0, false);

			// The first result with any non-nil values is used
			if (!result_set) {
				for (size_t i = 0; i < r.size(); i++) {
					if (r[i]->GetType() != TYPE_NIL) {
						result = r;
						result_set = true;

						// Change the 'using' argument so other scripts know
						// they won't be considered
						*using_param = false;
						break;
					}
				}
			}
			// todo: change how i handle return values for multiple scripts (see notes)
			
			itr++;
		}
		this->Free();

		return result;
	}

	// --------------------------------------------------------------------
	// Class: Result
	// Provides an interface for retrieving values from scripts
	//
	Result::Result()
	{

	}

	Result::Result(const std::vector<Object*>& result) : result(result)
	{
	}

	void Result::SetData(const Result& other)
	{
		std::vector<Object*>::const_iterator itr = other.result.begin();
		result.reserve(other.result.size());
		while (itr != other.result.end())
		{
			result.push_back((*itr)->NewCopy());
			itr++;
		}
	}

	Result::Result(const Result& other)
	{
		SetData(other);
	}

	void Result::Free()
	{
		std::vector<Object*>::iterator itr = result.begin();
		while (itr != result.end())
		{
			delete *itr;
			itr++;
		}
		result.clear();
	}

	Result::~Result()
	{
		Free();
	}

	Result& Result::operator=(const Result& rhs)
	{
		if (this == &rhs) return *this;
		Free();
		SetData(rhs);
		return *this;
	}

	const Object* Result::operator[](size_t index)
	{
		if (index < 0 || index >= result.size()) {
			std::stringstream err;
			err << "script: attempt to access out of bounds result.";
			throw std::exception(err.str().c_str());
		}
		return result[index];
	}

	size_t Result::size() const
	{
		return result.size();
	}

	// --------------------------------------------------------------------
	// Class: Object
	// Provides an interface between Lua and Phasor objects. The derived
	// classes provide specific types.
	// 
	Object::Object(obj_type _type) : type(_type) 
	{
	}

	Object::Object() : type(TYPE_NIL)
	{
	}

	Object::~Object()
	{
	}

	obj_type Object::GetType() const
	{
		return type;
	}

	Object* Object::NewCopy() const
	{
		return new Object(TYPE_NIL);
	}

	// --------------------------------------------------------------------
	//
	
	ObjBool::ObjBool(bool b) : Object(TYPE_BOOL)
	{
		this->b = b;
	}

	ObjBool::ObjBool(const ObjBool& other) : Object(TYPE_BOOL)
	{
		this->b = other.b;
	}

	ObjBool::~ObjBool()
	{
	}

	ObjBool & ObjBool::operator=(const ObjBool &rhs) 
	{
		if (this == &rhs) return *this;

		this->b = rhs.b;
		return *this;
	}

	ObjBool* ObjBool::NewCopy() const
	{
		return new ObjBool(*this);
	}

	bool ObjBool::GetValue() const
	{
		return this->b;
	}

	// --------------------------------------------------------------------
	//
	
	ObjNumber::ObjNumber(int value) : Object(TYPE_NUMBER)
	{
		this->value = value;
	}

	ObjNumber::ObjNumber(DWORD value) : Object(TYPE_NUMBER)
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

	ObjNumber::~ObjNumber()
	{

	}

	ObjNumber::ObjNumber(const ObjNumber& other) : Object(TYPE_NUMBER)
	{
		this->value = other.value;
	}

	ObjNumber& ObjNumber::operator=(const ObjNumber& rhs)
	{
		if (this == &rhs) return *this;

		this->value = rhs.value;
		return *this;
	}

	ObjNumber* ObjNumber::NewCopy() const
	{
		return new ObjNumber(*this);
	}

	double ObjNumber::GetValue() const
	{
		return this->value;
	}
	// --------------------------------------------------------------------
	//
	
	ObjString::ObjString(const char* str) : Object(TYPE_STRING)
	{
		CopyString(str);
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
		if (this == &rhs) return *this;
		
		delete[] str;
		CopyString(rhs.str);

		return *this;
	}

	ObjString* ObjString::NewCopy() const
	{
		return new ObjString(*this);
	}

	void ObjString::CopyString(const char* str)
	{
		this->str = new char [strlen(str) + 1];
		strcpy(this->str, str);
	}

	const char* ObjString::GetValue() const
	{
		return this->str;
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

	ObjTable::ObjTable(const ObjTable& other) : Object(TYPE_TABLE)
	{
		CopyTable(other);
	}

	ObjTable::~ObjTable() 
	{
		FreeTable();
	}

	void ObjTable::FreeTable()
	{
		std::map<Object*, Object*>::iterator itr = table.begin();
		while (itr != table.end())
		{
			delete itr->first;
			delete itr->second;
			itr = table.erase(itr);
		}
	}

	ObjTable& ObjTable::operator=(const ObjTable &rhs)
	{
		if (this == &rhs) return *this;
		FreeTable();
		CopyTable(rhs);
		return *this;
	}

	ObjTable* ObjTable::NewCopy() const
	{
		return new ObjTable(*this);
	}

	void ObjTable::CopyTable(const ObjTable& other)
	{
		std::map<Object*, Object*>::const_iterator itr = other.table.begin();

		while (itr != other.table.end())
		{
			Object* key = itr->first->NewCopy();
			Object* value = itr->second->NewCopy();
			table.insert(std::pair<Object*, Object*>(key, value));
		}
	}

	const Object* ObjTable::operator [] (const Object& key)
	{
		std::map<Object*, Object*>::iterator itr = table.find((Object*)&key);
		if (itr == table.end()) {
			std::stringstream err;
			err << __FUNCTION__ << ": specified key doesn't exist ";
			throw std::exception(err.str().c_str());
		}
		return itr->second;
	}
}