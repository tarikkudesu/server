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
		BasicString 				__form;
		std::ofstream				__fs;
		std::ofstream				__tmp;

		void						processMultiPartBody(BasicString &data);
		void						processFormData(BasicString &data);
		void						processDefinedBody(BasicString &data);
		void						processCunkedBody(BasicString &data);
		void						writeDataIntoFile(BasicString &data);
		void						createFile(std::vector<String> &h);
		void						processData(BasicString &data);
		void						mpHeaders(BasicString &data);
		void						mpInit(BasicString &data);
		void						mpBody(BasicString &data);

	public:
		void						reset();
		void						setWorkers(FileExplorer &explorer, Location &location, Server &server);
		void						executePost(BasicString &data);

		Post(Request &request, t_response_phase &phase);
		Post(const Post &copy);
		Post &operator=(const Post &assign);
		~Post();
};

#endif