#include <string>
#include <stdio.h>
#include "sqlitepp.h"

int main()
{
	using namespace sqlite;
	try
	{
		SQLite* sql = new SQLite("test.sqlite");
		SQLiteQuery* query = sql->NewQuery("CREATE TABLE IF NOT EXISTS admins("
			//"id INTEGER PRIMARY KEY,"
			"id int,"
			"username varchar(16),"
			"password char(32))");
		query->Execute();
		query->Reset("INSERT INTO admins (id, username, password) VALUES(:id, :username, :password)");
		query->BindValue(":id", 154);
		query->BindValue(":username", "uSer");
		query->BindValue(":password", "pass2");
		
		query->Execute();

		SQLiteResult* result = 0;
		query->Reset("SELECT * FROM admins");
		query->Execute(&result);

		for (size_t x = 0; x < result->size(); x++) {
			SQLiteRow* row = result->get(x);
			for (size_t c = 0; c < row->size(); c++)
				printf("%s\n", row->get(c)->ToString().c_str());
		}

		sql->free_object(result);
		sql->free_object(query);
		delete sql;
	}
	catch (SQLiteError & e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}/*

class A
{
private:
	//A& operator= (const A &v);
	A(const A &v);
public:
	A(int i)
	{
		printf("Construct\n");
	}
	A(const char * c)
	{

	}
	~A()
	{
		printf("Destructor\n");
	}

};

void test1(const A& a)
{
	printf("%08X\n", a);

}
void test(const A& a)
{
	test1(a);
}

int main()
{

	test(1);
	test("A");
//	A b = a;
	return 0;
}*/