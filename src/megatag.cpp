//#include "megatag.h"

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

megatag::megatag() :
	megatag_dir(get_megatag_dir()),
	db((megatag_dir + "db").c_str())
{
}

