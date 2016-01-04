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

#ifndef MEGATAG_H
#define MEGATAG_H

#include <map>
#include <string>
#include "db.h"
#include "graph.h"

void get_input(const char* shell_command, pid_t* _childs_pid = nullptr);

//! class to hold "global" variables, gui-independent
class megatag
{
	struct edge_t {
		std::string label() const { return ""; }
	};
	struct vertex_t {
		std::size_t tag_id;
		vertex_t() = default;
		vertex_t(int t) : tag_id(t) {}
		std::string label() const { return std::to_string(tag_id); }
	};

	using graph_t = graph_base_t<vertex_t, edge_t>;

	static std::string get_megatag_dir();
	std::map<std::size_t, graph_t::vertex_t> vertex_of;

protected:
	const std::string megatag_dir;
	db_t db;
	std::map<std::string, std::size_t> id_of; // TODO: bimap?

	char path[PATH_MAX];
	const char* _basename;
	int file_id; //!< id in 'file' table

	time_t get_ftime();
	void get_file_id();
	static pid_t get_xprop_pid();

	graph_t graph; // TODO: private?
	graph_t tra_cl; // TODO: private?

	bool is_reachable_from_current(std::size_t id);
	bool is_reachable_from_current(const char* id) {
		return is_reachable_from_current(atoi(id));
	}

	int get_tag_id(const std::string& id_name);

	static bool is_valid_keyword(const char* keyword);

	std::set<std::size_t> are_reachable_from(std::size_t src);

	void reread_graph();

public:
	megatag();
};

#endif // MEGATAG_H
