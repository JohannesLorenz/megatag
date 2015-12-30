#include <cstring>
#include <cstdio> // TODO
#include <stdexcept>
#include <string>
#include <iostream> // TODO
#include "db.h"

static int callback(void *voidptr, int argc, char **argv, char **col_names)
{
	functor_base* ftor_base = reinterpret_cast<functor_base*>(voidptr);
	return ((*ftor_base)(argc, argv, col_names)) ? 0 : 1;
}

void db_t::exec(const char *cmd, functor_base& ftor)
{
	std::cerr << "Executing: " << cmd <<std::endl;
	char* err_msg;
	int rc = sqlite3_exec(db, cmd, callback, &ftor, &err_msg);
	if( rc != SQLITE_OK ) {
		throw std::runtime_error(std::string("SQL error: ") + err_msg);
		sqlite3_free(err_msg);
		}
}

void db_t::exec(const std::string &cmd, functor_base &ftor)
{
	exec(cmd.c_str(), ftor);
}

bool db_t::table_exists(const char* table_name)
{
	struct functor_table_found : functor_base
	{
		bool operator()(int, char **argv, char **)
		{
			if(search && argv[0] && !strcmp(argv[0], search))
			 search = nullptr;
			return true; // TODO: throw here?
		}
		const char* search;
	public:
		functor_table_found(const char* search) : search(search) {}
		bool found() const { return search == nullptr; }
	};

	functor_table_found ftor(table_name);
	exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;", ftor);
	return ftor.found();
}

db_t::db_t(const char *filename)
{
	int rc = sqlite3_open(filename, &db);

	if( rc ){
		throw std::runtime_error(std::string("Can't open database: ") + sqlite3_errmsg(db));
	}


	const char* create_str = "CREATE TABLE files ("
		"'id' INTEGER PRIMARY KEY, "
		"'path' varchar(255),"
		"'last_changed' int,"
		"'filetype' varchar(8),"
		"'quality' smallint(6),"
		"'md5sum' varchar(128)"
		");";

	const char* create_str_ids = "CREATE TABLE ids ("
		"'id' INTEGER PRIMARY KEY, "
		"'name' varchar(127),"
		"'timestamp' int,"
		"'used_count' int"
		");";

	const char* create_tags = "CREATE TABLE tags ("
		"'file_id' int,"
		"'tag_id' smallint(16),"
		// pairs of (file_id, tag_id) are unique
		// same effect if both columns would make 1 prim krey
		"UNIQUE(file_id, tag_id)"
		");";

	const char* create_keywords = "CREATE TABLE keywords ("
		"'keyword' varchar(255),"
		"'id' smallint(16)"
	");";

	const char* create_implicits = "CREATE TABLE implicits ("
		"'src' smallint(16),"
		"'dest' smallint(16)"
	");";

	if(!table_exists("files"))
	 exec(create_str);
	if(!table_exists("ids"))
	 exec(create_str_ids);
	if(!table_exists("tags"))
	 exec(create_tags);
	if(!table_exists("keywords"))
	 exec(create_keywords);
	if(!table_exists("implicits"))
	 exec(create_implicits);
}

db_t::~db_t()
{
	sqlite3_close(db);
}



bool functor_print::operator()(int argc, char **argv, char **col_names)
{
	int i;
	for(i=0; i<argc; i++){
		fprintf(stderr, "%s = %s\n", col_names[i], argv[i] ? argv[i] : "NULL");
	}
	fprintf(stderr, "\n");
	return true;
}
