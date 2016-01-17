/*************************************************************************/
/* megatag - A simple library to tag files graphically                   */
/* Copyright (C) 2015-2016                                               */
/* Johannes Lorenz (jlsf2013 @ sourceforge)                              */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 3 of the License, or (at */
/* your option) any later version.                                       */
/* This program is distributed in the hope that it will be useful, but   */
/* WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      */
/* General Public License for more details.                              */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program; if not, write to the Free Software           */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA  */
/*************************************************************************/

#include <cstring>
#include <cstdio> // TODO
#include <stdexcept>
#include <string>
#include <iostream> // TODO
#include "db.h"

using namespace std::literals::string_literals;

static int callback(void *voidptr, int argc, char **argv, char **col_names)
{
	functor_base* ftor_base = reinterpret_cast<functor_base*>(voidptr);
	return ((*ftor_base)(argc, argv, col_names)) ? 0 : 1;
}

void db_t::exec(const char *cmd, functor_base& ftor)
{
	if(debug)
	 std::cerr << "Executing: " << cmd <<std::endl;
	char* err_msg;
	int rc = sqlite3_exec(db, cmd, callback, &ftor, &err_msg);
	if( rc != SQLITE_OK ) {
		throw std::runtime_error("SQL error: "s + err_msg);
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

void db_t::load(const char* filename)
{
	sqlite3_enable_load_extension(db, 1);
	char* err_msg;
	if(sqlite3_load_extension(db, filename, NULL, &err_msg) == SQLITE_ERROR)
	 throw std::runtime_error(err_msg);
	sqlite3_enable_load_extension(db, 0);
}

db_t::db_t(const char *filename)
{
	int rc = sqlite3_open(filename, &db);

	if( rc ){
		throw std::runtime_error("Can't open database: "s + sqlite3_errmsg(db));
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
		"'keyword' varchar(255), "
		"'id' smallint(16), "
		"UNIQUE(keyword, id)"
	");";

	const char* create_implicits = "CREATE TABLE implicits ("
		"'src' smallint(16), "
		"'dest' smallint(16), "
		"UNIQUE(src, dest)"
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
