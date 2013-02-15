#include "api_readers.h"
#include "../Phasor/Halo/Alias.h"
#include "../Manager.h"
#include "../Scripting.h"
#include "../Common/MyString.h"
#include <list>

using namespace Common;
using namespace Manager;
using namespace halo;


template <class AliasQuery>
class QueryAliasResults : public AliasQuery, public scripting::CheckedScriptReference
{
private:
	std::string callback;

public:
	QueryAliasResults(const std::string& query, const std::string& callback,
		Manager::ScriptState* state)
		: AliasQuery(std::unique_ptr<COutStream>(new SinkStream()), query),
		CheckedScriptReference(state),
		callback(callback)
	{
	}

	void ProcessResult() override
	{
		if (!still_valid()) return;

		std::unique_ptr<Common::Object> result_obj(new Common::ObjTable());
		Common::ObjTable* result_table = static_cast<Common::ObjTable*>(result_obj.get());

		// results are always ordered by hash. 
		size_t x = 0;
		while (x < result->size()) {
			std::unique_ptr<Common::ObjTable> name_table(new ObjTable());
			std::wstring old_hash;
			size_t i = x;
			for (; i < result->size(); i++) {
				ObjWString& nameobj = static_cast<ObjWString&>((*result)[i]["name"]);
				ObjWString& hashobj = static_cast<ObjWString&>((*result)[i]["hash"]);
				std::string name = NarrowString(nameobj.GetValue());
			
				// not same hash anymore
				if (i != x && old_hash != hashobj.GetValue()) break;
				old_hash = hashobj.GetValue();

				size_t tbl_index = i - x + 1;
				name_table->insert(Common::ObjTable::pair_t(
					std::unique_ptr<Common::Object>(new ObjNumber((DWORD)tbl_index)), 
					std::unique_ptr<Common::Object>(new ObjString(name))));
			}
			result_table->insert(Common::ObjTable::pair_t(
				std::unique_ptr<Common::Object>(new ObjString(NarrowString(old_hash))),
				std::move(name_table)));
			x += i - x;
		}
		
		Manager::Caller caller;
		caller.AddArg(std::move(result_obj));
		caller.Call(*state, callback, 5000);
	}
};

template <class AliasQuery>
void perform_alias_query(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	std::string hash_or_name = ReadRawString(*args[0]);
	std::string callback = ReadRawString(*args[1]);

	std::shared_ptr<alias::CAliasEvent> e(
		new QueryAliasResults<AliasQuery>(hash_or_name, callback, &handler.state)
		);
	AddResultBool(alias::ExecuteEvent(e), results);	
}

void l_alias_search(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	return perform_alias_query<alias::AliasEventQueryPlayer>(handler, args, results);
}

void l_alias_hash(CallHandler& handler, Object::unique_deque& args, Object::unique_list& results)
{
	return perform_alias_query<alias::AliasEventQueryHash>(handler, args, results);
}