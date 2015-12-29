#ifndef MEGATAG_H
#define MEGATAG_H

#include <string>
#include "db.h"

void get_input(const char* shell_command, pid_t* _childs_pid = nullptr);

//! class to hold "global" variables, gui-independent
class megatag
{
	static std::string get_megatag_dir();

protected:
	const std::string megatag_dir;
	db_t db;

public:
	megatag();
	pid_t get_xprop_pid();

};

#endif // MEGATAG_H
