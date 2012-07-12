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
			"id INT AUTO_INCREMENT,"
			"username varchar(16),"
			"password char(32),"
			"PRIMARY KEY (id))");
		query->Execute();
	/*	query->Reset("INSERT INTO admins (id, username, password) VALUES(:id, :username, :password)");
		query->BindValue(":id", 5);
		query->BindValue(":username", "my_username");
		query->BindValue(":password", "12345_password");
		query->Execute();*/

		SQLiteResult* result = 0;
		query->Reset("SELECT * FROM admins");
		query->Execute(&result);

		for (size_t x = 0; x < result->size(); x++) {
			printf("row\n");
			for (size_t c = 0; c < (result->get(x))->size(); c++)
				printf("sweet\n");
		}

		sql->free_object(query);
		delete sql;
	}
	catch (SQLiteError & e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}