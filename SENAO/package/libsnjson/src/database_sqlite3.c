#include <api_common.h>
#include <sys_common.h>
// #include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <api_rainier.h>
// #include <wireless_tokens.h>
// #include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_rainier.h>
#include <json_common.h>
#include <unistd.h>
#include <api_sys.h>
#include <sqlite3.h>
#include <time.h>
#include "database_sqlite3.h"




void free_database(sqlite3 *db)
{
	if (db) {
		sqlite3_close(db);
		db = NULL;
	}
}

int Callback_ShowList( void *context, int count, char **values, char ** columnName )
{
	int i;
	context = NULL;

	printf("%s : %d \n", __FUNCTION__, __LINE__)    ;
	for( i=0 ; i<count ; ++i )
	{
		debug_print( "\t\t%s = %s\n" , columnName[i] , values[i] ? values[i] : "NULL" );
	        debug_print( "\n" );
	}
	return SQLITE_OK;
}


int insert_to_table(sqlite3 *db, char *table, char *col, char *val)
{
	char buf[512] = {0};
	char *error_report = NULL;

	sprintf(buf, "INSERT INTO %s (%s) VALUES(%s)",table, col, val);


	if( sqlite3_exec(db , buf, Callback_ShowList , NULL , &error_report ) != SQLITE_OK )
	{
		debug_print("Jason DEBUG %s[%d] cmd: %s error: %s \n", __FUNCTION__, __LINE__,  buf , error_report);
		sqlite3_free(error_report);
		return -1;
	}
	return 0;

}



int get_database(char *file, sqlite3 **sql_db)
{

	sqlite3 *db = NULL;

	int rc = sqlite3_open(file, &db);
	
	if (rc != SQLITE_OK) 
		return -1;
	
	*sql_db = db;

	return 0;

}

int read_database(char *sql, sqlite3 *sql_db, void *read_callback, void *user)
{
	int rc = 0;
	char *err_msg = 0;

	rc = sqlite3_exec(sql_db, sql, read_callback, user, &err_msg);

	if (rc != SQLITE_OK) 
	{
		debug_print("Jason DEBUG %s[%d] read database  error: %s !!\n", __FUNCTION__, __LINE__, err_msg);
		sqlite3_free(err_msg);
		return -1;
	}
	return 0;

}

int open_database(char *table, char *columns, char *db_file, sqlite3 **sql_db)
{
	char *error;
	char filename[80];
	int res;
	char *sql;

	sqlite3 *db = NULL;

	/* is the database there? */
	snprintf(filename, sizeof(filename), "%s", db_file);
	res = sqlite3_open(filename, &db);
	if (res != SQLITE_OK) {
		free_database(db);
		return -1;
	}
	sqlite3_busy_timeout(db, 1000);
	/* is the table there? */
	sql = sqlite3_mprintf("SELECT COUNT(AcctId) FROM %q;", table);
	res = sqlite3_exec(db, sql, NULL, NULL, NULL);
	sqlite3_free(sql);
	if (res != SQLITE_OK) {
		/* We don't use %q for the column list here since we already escaped when building it */
		sql = sqlite3_mprintf("CREATE TABLE %q (AcctId INTEGER PRIMARY KEY, %s)", table, columns);
		res = sqlite3_exec(db, sql, NULL, NULL, &error);
		sqlite3_free(sql);
		if (res != SQLITE_OK) {
			sqlite3_free(error);
			free_database(db);
			return -1;
		}
	}
	else
	{
		debug_print("Jason DEBUG %s[%d] table %s selected !!\n", __FUNCTION__, __LINE__, table);
	}
	*sql_db = db;

	return 0;
}
