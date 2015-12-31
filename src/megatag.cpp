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

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "megatag.h"

void get_input(const char* shell_command, pid_t* _childs_pid)
{
	int pipefd[2];
	pid_t childs_pid;

	if (pipe(pipefd) == -1) {
		throw std::runtime_error("pipe() failed");
	}

	//      fcntl(pipefd[0], F_SETFL, O_NONBLOCK); // ?????

	// fork sh
	childs_pid=fork();
	if(childs_pid < 0) {
		throw std::runtime_error("fork() failed");
	}
	else if(childs_pid == 0) {

		close(pipefd[0]); /* Close unused read end */

		dup2(pipefd[1], STDOUT_FILENO);
		setenv("DISPLAY", ":0", 1);
		execlp("/bin/sh", "sh"  , "-c", shell_command, NULL);

		close(pipefd[1]); /* Reader will see EOF */
		exit(0);
	}

	close(pipefd[1]); /* Close unused write end */
	dup2(pipefd[0], STDIN_FILENO);

	if(_childs_pid)
	 *_childs_pid = childs_pid;
}

pid_t megatag::get_xprop_pid()
{
	get_input("sleep 0.1 && xprop _NET_WM_PID");

	const std::size_t xprop_out_size = 64;
	char last_xprop_output[xprop_out_size];
	while( std::cin.getline(last_xprop_output, xprop_out_size) )
	{
		std::cout << "LAST: " << last_xprop_output << std::endl;

		const char* ptr = last_xprop_output;
		for(; *ptr != '=' && *ptr != 0; ++ptr) ;
		if(!*ptr)
		 continue;

		for( ++ptr; *ptr == ' ' || *ptr == '\t'; ++ptr) ;
		pid_t pid = atoi(ptr);

		if(!pid)
		 continue;

		return pid;
	}

	return 0;
}

std::string megatag::get_megatag_dir()
{
	struct passwd *pw = getpwuid(getuid());
	std::string homedir = pw->pw_dir;

	std::string _megatag_dir = homedir + "/.config/megatag/";

	if(access(_megatag_dir.c_str(), F_OK))
	 mkdir(_megatag_dir.c_str(), 0755);

	return _megatag_dir;
}

void megatag::get_file_id()
{
	db.func0("SELECT id FROM files WHERE path = '" + std::string(path) + "';",
			[&](char** arg) { file_id = atoi(arg[0]); } );
}


megatag::megatag() :
	megatag_dir(get_megatag_dir()),
	db((megatag_dir + "db").c_str())
{
}



time_t megatag::get_time()
{
	struct stat attrib;
	stat(path, &attrib);
	return attrib.st_mtime;
}
