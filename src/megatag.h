/*************************************************************************/
/* megatag - A simple library to tag files graphically                   */
/* Copyright (C) 2015                                                    */
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

#ifndef MEGATAG_H
#define MEGATAG_H

#include <map>
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
	std::map<std::string, std::size_t> id_of;

	char path[PATH_MAX];
	const char* _basename;
	int file_id; //!< id in 'file' database

	time_t get_time();
	void get_file_id();

public:
	megatag();
	pid_t get_xprop_pid();
};

#endif // MEGATAG_H
