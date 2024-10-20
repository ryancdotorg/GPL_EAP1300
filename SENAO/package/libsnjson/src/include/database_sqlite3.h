#ifndef SQLLITE_H
#define SQLLITE_H


int get_database(char *file, sqlite3 **sql_db);
void free_database(sqlite3 *db);
int insert_to_table(sqlite3 *db, char *table, char *col, char *val);
int open_database(char *table, char *columns, char *db_file, sqlite3 **sql_db);
int read_database(char *sql, sqlite3 *sql_db, void *read_callback, void *user);


















#endif
