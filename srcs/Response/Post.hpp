#ifndef __POST_HPP__
#define __POST_HPP__


#include "FileExplorer.hpp"

class Post
{
	private:
		Request 					&request;
		t_response_phase			&__responsePhase;

		FileExplorer				*explorer;
		Location					*location;
		Server						*server;

		t_multipartsection			__phase;
		BasicString					__data;
		BasicString					__form;
		std::ofstream				__fs;

		void						processMultiPartBody();
		void						processFormData();
		void						writeDataIntoFile(BasicString &data);
		void						createFile(std::vector<String> &h);
		void						mpHeaders();
		void						mpInit();
		void						mpBody();

	public:
		void						reset();
		void						setWorkers(FileExplorer &explorer, Location &location, Server &server);
		void						processData(BasicString &data);
		BasicString 				&getForm();

		Post(Request &request, t_response_phase &phase);
		Post(const Post &copy);
		Post &operator=(const Post &assign);
		~Post();
};

#endif