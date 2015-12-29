#ifndef DB_T_H
#define DB_T_H

#include <sqlite3.h>
#include <string>

struct functor_base
{
	virtual bool operator()(int argc, char **argv, char **col_names) = 0;
};

struct functor_silent : public functor_base
{
	virtual bool operator()(int, char **, char **) { return true; }
};

template<class T>
class functor_ftor : public functor_base
{
	const T& ftor;
	virtual bool operator()(int argc, char **argv, char **col_names) {
		return ftor(argc, argv, col_names);
	}
public:
	functor_ftor(const T& ftor) : ftor(ftor) {}
};

struct functor_print : public functor_base
{
	bool operator()(int argc, char **argv, char **col_names);
};

class db_t
{
	sqlite3 *db;
public:
	bool table_exists(const char* table_name);
	void exec(const char* cmd, functor_base& ftor);
	void exec(const std::string& cmd, functor_base& ftor);
	template<class Ftor>
	void func(const char* cmd, const Ftor& _ftor)
	{
		functor_ftor<Ftor> ftor(_ftor);
		exec(cmd, ftor);
	}
	template<class Ftor>
	void func(const std::string& str, const Ftor& _ftor)
	{
		func(str.c_str(), _ftor);
	}
	void exec(const char* cmd) {
		functor_silent silence;
		exec(cmd, silence);
	}
	void exec(const std::string& cmd) {
		functor_silent silence;
		exec(cmd, silence);
	}


	db_t(const char* filename);
	~db_t();
};

#endif // DB_T_H
