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

template<class T>
class functor_ftor_simple_ret : public functor_base
{
	const T& ftor;
	virtual bool operator()(int, char **argv, char **) {
		return ftor(argv);
	}
public:
	functor_ftor_simple_ret(const T& ftor) : ftor(ftor) {}
};

template<class T>
class functor_ftor_simple : public functor_base
{
	const T& ftor;
	virtual bool operator()(int, char **argv, char **) {
		return ftor(argv), true;
	}
public:
	functor_ftor_simple(const T& ftor) : ftor(ftor) {} // TODO: base class?
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

	template<class Ftor>
	void func0(const char* cmd, const Ftor& _ftor)
	{
		functor_ftor_simple<Ftor> ftor(_ftor);
		exec(cmd, ftor);
	}
	template<class Ftor>
	void func0(const std::string& str, const Ftor& _ftor)
	{
		func0(str.c_str(), _ftor);
	}

	bool contains(const std::string& str)
	{
		bool contained = false;
		func0(str, [&](char**) { contained = true; });
		return contained;
	}

	template<class Ftor>
	void func0_ret(const char* cmd, const Ftor& _ftor)
	{
		functor_ftor_simple_ret<Ftor> ftor(_ftor);
		exec(cmd, ftor);
	}
	template<class Ftor>
	void func0_ret(const std::string& str, const Ftor& _ftor)
	{
		func0_ret(str.c_str(), _ftor);
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
