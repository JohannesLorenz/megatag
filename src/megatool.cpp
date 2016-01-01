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

#include "megatool.h"

std::string megatool::get_known_tag_id(const char *id_name)
{
	int tag_id = get_tag_id(id_name);
	if(tag_id == -1)
	 throw std::runtime_error("Unknwon tag specified");
	return std::to_string(tag_id);
}

megatool::megatool()
{

}

void megatool::add_keyword(const char *keyword, const char *tag)
{
	if(!is_valid_keyword(keyword))
	 throw std::runtime_error("Keywords must currently be in '[A-Za-z]*'.");
	db.exec("INSERT OR IGNORE into keywords (keyword, id) VALUES('"
		+ std::string(keyword) + "', '"
		+ get_known_tag_id(tag) + "');");
}

void megatool::add_impl(const char *src_tag, const char *dest_tag)
{
	db.exec("INSERT OR IGNORE into implicits (src, dest) VALUES('"
		+ get_known_tag_id(src_tag) + "', '"
		+ get_known_tag_id(dest_tag) + "');");
}

void megatool::query(std::string sql_expr)
{
	db.func0("SELECT DISTINCT files.path from"
		" files"
		" JOIN tags ON files.id = tags.file_id"
		" JOIN ids ON tags.tag_id = ids.id"
		" WHERE " + sql_expr +
		" ORDER BY files.path",
		[&](char** arg) { std::cout << arg[0] << std::endl; });
}

