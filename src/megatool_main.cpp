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

#include <cstdlib>
#include <string>
#include "megatool.h"

class argument_error : public std::runtime_error
{
public:
	argument_error(const char* msg) : std::runtime_error(msg) { }

	const char* what() const noexcept {
		using namespace std::literals::string_literals;
		return ("invalid arguments specified: "s
			+ runtime_error::what()).c_str();
	}
};

enum class impl_direction_t
{
	no_impl,
	rightwards,
	leftwards
};

enum class impl_type_t
{
	no_impl,
	impl,
	keywords
};

void run(int argc, char** argv)
{
	if(!argc)
	 throw argument_error("No arguments specified");

	if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "help"))
	{
		std::cerr << "usage" << std::endl;
		std::cerr << argv[0] << " <keywords> '->' <tags>" << std::endl;
		std::cerr << argv[0] << " <tags> '<-' <keywords>" << std::endl;
		std::cerr << argv[0] << " <tags> '->' <tags>" << std::endl;
		std::cerr << argv[0] << " <tags> '<-' <tags>" << std::endl;
		std::cerr << argv[0] << " <sql-where-expr without 'where'>" << std::endl;
		std::cerr << argv[0] << "   (in the sql-expr, refer to the tag name as 'name')" << std::endl;
		std::cerr << "Note that < and > must be shell-escaped." << std::endl;
	}

	megatool m;

	impl_direction_t impl_direction = impl_direction_t::no_impl;
	impl_type_t impl_type = impl_type_t::no_impl;
	int found_arrow = -1;
	for(std::size_t i = 1; i < (std::size_t)argc; ++i)
	{
		if(!strcmp(argv[i], "<=") || ! strcmp(argv[i], "<-")
			|| !strcmp(argv[i], "=>") || ! strcmp(argv[i], "->"))
		{
			if(found_arrow != -1)
			 throw argument_error("Two arrows found");
			found_arrow = i;

			impl_direction = !strcmp(argv[i], "<=") || ! strcmp(argv[i], "<-") ?
				impl_direction_t::leftwards : impl_direction_t::rightwards;

			impl_type = !strcmp(argv[i], "<=") || ! strcmp(argv[i], "=>") ?
				impl_type_t::impl : impl_type_t::keywords;
		}

	}

	if(impl_type == impl_type_t::no_impl)
	{
		if(argc != 2)
		 throw argument_error("Currently, queries must only consist of one arg, the sql-where-expr");
		// just print the query
		m.query(argv[1]);
	}
	else
	{
		// draw implications or keywords
		for(std::size_t left_id = 1; left_id < (std::size_t)found_arrow; ++left_id)
		for(std::size_t right_id = found_arrow + 1; right_id < (std::size_t)argc; ++right_id)
		{
			const char* src_ptr =
				impl_direction == impl_direction_t::leftwards
				? argv[right_id] : argv[left_id];
			const char* dest_ptr =
				impl_direction == impl_direction_t::leftwards
				? argv[left_id] : argv[right_id];

			if(impl_type == impl_type_t::keywords)
			 m.add_keyword(src_ptr, dest_ptr);
			else
			 m.add_impl(src_ptr, dest_ptr);

		}
	}

}

int main(int argc, char** argv)
{
	int rval = EXIT_SUCCESS;
	try {
		run(argc, argv);
	} catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
		rval = EXIT_FAILURE;
	}

	return rval;
}

