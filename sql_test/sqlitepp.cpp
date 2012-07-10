#pragma once

#include <string>
#include <vector>

/* Simple SQLite wrapper supporting prepared parameter binding for select
 * and insert operations. 
 */
namespace sqlite 
{
	class SQLiteRow;
	class SQLiteTypeError;
	class SQLiteBinding;

	class SQLite
	{
	public:
		SQLite(std::string path_to_db);
		~SQLite();

		/* Execute a prepared select query using bound parameters
		 * no good way to do this without C++0x :/ */
		typedef SQLiteBinding sqlb;
		std::vector<SQLiteRow*> Select(const wchar_t* statement);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2, sqlb b3);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2, sqlb b3, sqlb b4);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2, sqlb b3, sqlb b4, sqlb b5);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2, sqlb b3, sqlb b4, sqlb b5, sqlb b6);
		std::vector<SQLiteRow*> Select(const wchar_t* statement,
			sqlb b1, sqlb b2, sqlb b3, sqlb b4, sqlb b5, sqlb b6, sqlb b7);



		void sql_free(SQLiteRow* obj);		
	};

	class SQLiteBinding 
	{
	public:
		SQLiteBinding(int val);
		SQLiteBinding(std::string val);
		SQLiteBinding(std::wstring val);
		SQLiteBinding(const char* val);
		SQLiteBinding(const wchar_t* val);
		SQLiteBinding(double val);
		SQLiteBinding(float val);
		SQLiteBinding(bool val);
		~SQLiteBinding();
	};

	class SQLiteRow 
	{
	private: 
		SQLiteRow();
		~SQLiteRow();

	public:
		
		/*	Get the row data in various data types, if the requested type
		 *	is not stored in this row a SQLiteTypeError exception is thrown
		 */
		const std::string GetStr();
		const std::wstring WGetStr();
		const int GetInt();
		const double GetDouble();
		const float GetFloat();
		const bool GetBool();

		friend class SQLite;
	};

	class SQLiteTypeError : public std::exception
	{
	public:
		SQLiteTypeError(const char* excp);
		virtual const char* what() const throw();
	};
}