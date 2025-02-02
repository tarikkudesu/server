#ifndef __GET_HPP__
#define __GET_HPP__

#include "Post.hpp"

class Get
{
	private:
		Request							&request;
		t_response_phase				&__responsePhase;

		FileExplorer				    *explorer;
		Location						*location;
		Server							*server;

		std::ifstream					__file;

		void							readFile(void);
		void							autoIndexing(void);
		void							getInPhase();
		void							duringGetPhase(BasicString &body);

	public:
		t_get_file_operation			__phase;

		void							reset();
		void							setWorkers(FileExplorer &explorer, Location &location, Server &server);
		void							executeGet(BasicString &body);

		Get(Request &request, t_response_phase &phase);
		Get(const Get &copy);
		Get &operator=(const Get &assign);
		~Get();
};

#endif