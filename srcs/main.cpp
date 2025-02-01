
#include "ServerManager/ServerManager.hpp"

void f()
{
	int fd = open(".log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (-1 == fd)
	{
		wsu::terr(String(" .logs/sds.log cannot be opened"));
		exit(EXIT_FAILURE);
	}
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
	system("lsof -c webserv");
	std::cout << "\n********************************************************************\n";
	std::cout << "********************************************************************\n\n";
	system("leaks -list webserv");
}

#include <execinfo.h>

void signalHandler(int signal)
{
	if (signal == SIGINT || signal == SIGPIPE)
	{
		// void *array[10];
		// size_t size = backtrace(array, 10);
		// fprintf(stderr, "Error: signal %d:\n", signal);
		// backtrace_symbols_fd(array, size, STDERR_FILENO);
		exit(1);
		std::cout << "exiting\n";
		Core::up = false;
	}
}

int main(int ac, char **av)
{
	atexit(f);
	signal(SIGINT, signalHandler);
	signal(SIGPIPE, signalHandler);
	std::vector<String> args;
	for (int i = 1; i < ac; ++i)
		args.push_back(String(av[i]));
	{
		wsu::logs(args);
		ServerManager webserv(*(args.end() - 1));
		webserv.setUpWebserv();
	}
}