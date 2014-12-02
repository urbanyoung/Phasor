#include "sqlitepp.h"

using namespace Common;

namespace sqlite
{
	//-----------------------------------------------------------------------------------------
	// Class: SQLiteError
	//
	SQLiteError::SQLiteError(int error) : exception(err_descs[error])
	{
	}

	SQLiteError::SQLiteError(const char* e) : exception(e)
	{
	}

	SQLiteError::~SQLiteError()
	{
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLite
	//
	SQLite::SQLite(const std::string& path_to_db)
	{
		int result = sqlite3_open(path_to_db.c_str(), &sqlhandle);

		if (result != SQLITE_OK) { // some error	
			std::string desc = LastError();
			sqlite3_close(sqlhandle);
			throw SQLiteError(desc.c_str());
		}
	}

	SQLite::~SQLite()
	{
		Close();
	}

	void SQLite::Close()
	{
		if (sqlhandle) {
			sqlite3_close(sqlhandle);
			sqlhandle = NULL;
		}
	}

	sqlite3* SQLite::GetSQLite()
	{
		if (!sqlhandle) throw SQLiteError(SQL_ERRCODE_CLOSED);
		return sqlhandle;
	}

	std::unique_ptr<SQLiteQuery> SQLite::NewQuery(const std::wstring& query)
	{
		return std::unique_ptr<SQLiteQuery>(new SQLiteQuery(*this, query));
	}

	const char* SQLite::LastError()
	{
		return sqlite3_errmsg(sqlhandle);
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteObject
	// Purpose: Base class for objects returned to the user
	// 
	SQLiteObject::SQLiteObject(SQLite& parent) : parent(parent)
	{
	}

	SQLiteObject::~SQLiteObject()
	{
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteQuery
	// Purpose: Provide an interface for executing prepared queries
	// 
	SQLiteQuery::SQLiteQuery(SQLite& parent, const std::wstring& query)
		: SQLiteObject(parent), query(query), stmt(NULL)
	{
		prepare_statement();
	}

	SQLiteQuery::~SQLiteQuery()
	{
		if (stmt) sqlite3_finalize(stmt);	
	}

	void SQLiteQuery::prepare_statement()
	{
		// Get a statement handle, on failure abort.
		if (sqlite3_prepare16_v2(parent.GetSQLite(), query.c_str(), -1,
			&stmt, NULL) != SQLITE_OK) 
		{
			throw SQLiteError(parent.LastError());
		}
	}

	std::unique_ptr<SQLiteResult> SQLiteQuery::Execute() 
	{		
		std::unique_ptr<SQLiteResult> out_result(new SQLiteResult());

		while (1)
		{
			int result = sqlite3_step(stmt);
			if (result == SQLITE_DONE) break;

			if (result == SQLITE_ROW) { // data!
				/*if (!out_result) { // no data was expected
					error = SQL_ERRCODE_NDE;
					break;
				}*/
				std::unique_ptr<SQLiteRow> row(new SQLiteRow());

				int columns = sqlite3_column_count(stmt);
				for (int x = 0; x < columns; x++) {
					int type = sqlite3_column_type(stmt, x);
					const char* name = sqlite3_column_name(stmt, x);
					switch (type)
					{
					case SQLITE_INTEGER:
						row->AddColumn(Object::unique_ptr(
							new ObjNumber(sqlite3_column_int(stmt, x))), name);
						break;
					case SQLITE_FLOAT:
						row->AddColumn(Object::unique_ptr(
							new ObjNumber(sqlite3_column_double(stmt, x))), name);
						break;
					case SQLITE_TEXT:
						row->AddColumn(Object::unique_ptr(
							new ObjWString((const wchar_t*)sqlite3_column_text16(stmt, x))),
							name);
						break;
					case SQLITE_BLOB:
						row->AddColumn(Object::unique_ptr(
							new ObjBlob((BYTE*)sqlite3_column_blob(stmt, x),
										sqlite3_column_bytes(stmt, x))),
							name);
						break;
					}
				}
				out_result->AddRow(std::move(row));
			}
			else { // error
				throw SQLiteError(parent.LastError());
			}
		}

		sqlite3_finalize(stmt);
		stmt = NULL;

		return out_result;
	}

	void SQLiteQuery::Reset(const std::wstring& query)
	{
		this->query = query;
		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
		prepare_statement();
	}

	void SQLiteQuery::BindValue(const char* name, const SQLiteValue& value) 
	{
		BindValue(sqlite3_bind_parameter_index(stmt, name), value);	
	}

	void SQLiteQuery::BindValue(int index, const SQLiteValue& value)
	{
		if (!stmt) throw SQLiteError(SQL_ERRCODE_NO_INIT);
		if (!index) throw SQLiteError(SQL_ERRCODE_NO_PARAM);

		int result = 0;
		switch (value.type)
		{
		case TYPE_STRING:
			{
				ObjString& str = static_cast<ObjString&>(*value.object);
				result = sqlite3_bind_text(stmt, index, str.GetValue(),
					-1, SQLITE_TRANSIENT);
			} break;
			
		case TYPE_WSTRING:
			{
				ObjWString& str = static_cast<ObjWString&>(*value.object);
				result = sqlite3_bind_text16(stmt, index, str.GetValue(),
					-1, SQLITE_TRANSIENT);
			} break;
		case TYPE_INT:
			{
				ObjNumber& num = static_cast<ObjNumber&>(*value.object);
				result = sqlite3_bind_int(stmt, index, (int)num.GetValue());
			}break;
		case TYPE_DOUBLE:
			{
				ObjNumber& num = static_cast<ObjNumber&>(*value.object);
				result = sqlite3_bind_double(stmt, index, num.GetValue());

			}break;
		case TYPE_BLOB:
			{
				ObjBlob& blob = static_cast<ObjBlob&>(*value.object);
				size_t length;
				BYTE* data = blob.GetData(length);
				result = sqlite3_bind_blob(stmt, index, data, length,
					SQLITE_TRANSIENT);

			}break;
		}

		// Handle any errors
		switch (result)
		{
		case SQLITE_OK:
			break;
		default:
			throw SQLiteError(parent.LastError());
			break;
		}	
	}

	// --------------------------------------------------------------------
	SQLiteValue::SQLiteValue(const std::string& val)
		: object(new ObjString(val)), type(TYPE_STRING)
	{
	}

	SQLiteValue::SQLiteValue(const std::wstring& val)
		: object(new ObjWString(val)), type(TYPE_WSTRING)
	{
	}

	SQLiteValue::SQLiteValue(const char* val)
		: object(new ObjString(val)), type(TYPE_STRING)
	{
	}

	SQLiteValue::SQLiteValue(const wchar_t* val)
		: object(new ObjWString(val)), type(TYPE_WSTRING)
	{
	}


	SQLiteValue::SQLiteValue(int val)
		: object(new ObjNumber(val)), type(TYPE_INT)
	{
	}
	
	SQLiteValue::SQLiteValue(double val)
		: object(new ObjNumber(val)), type(TYPE_DOUBLE)
	{
	}

	SQLiteValue::SQLiteValue(BYTE* val, size_t length)
		: object(new ObjBlob(val, length)), type(TYPE_BLOB)
	{
	}


	//-----------------------------------------------------------------------------------------
	// Class: SQLiteRow
	//

	void SQLiteRow::AddColumn(Common::Object::unique_ptr value, 
		const std::string& name)
	{
		columns.insert(pair_t(name, std::move(value)));
	}

	Object& SQLiteRow::operator[] (const std::string& key)
	{
		return get(key);
	}

	Object& SQLiteRow::get(const std::string& key)
	{
		auto itr = columns.find(key);
		return *(itr->second);
	}

	size_t SQLiteRow::size() 
	{
		return columns.size();
	}

	//-----------------------------------------------------------------------------------------
	// Class: SQLiteResult
	//
	void SQLiteResult::AddRow(std::unique_ptr<SQLiteRow> row)
	{
		rows.push_back(std::move(row));
	}

	SQLiteRow& SQLiteResult::operator[] (size_t i)
	{
		return get(i);
	}

	SQLiteRow& SQLiteResult::get(size_t i)
	{
		return *rows[i];
	}

	size_t SQLiteResult::size()
	{
		return rows.size();
	}
}