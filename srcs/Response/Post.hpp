#pragma once

#include "RessourceHandler.hpp"

class Post
{
	private:
        String                      __boundary;
		t_response_phase			&__responsePhase;
		RessourceHandler			*explorer;
		Request 					*request;
		Location					*location;
		std::ifstream				__fs;


		void						writeDataIntoFile(BasicString &data); // omar
		void						createFile(std::vector<BasicString> &h); // omar
		void						mpHeaders(BasicString &data, t_multipartsection &part);
		void						mpBody(BasicString &data, t_multipartsection &part);
		void						processMultiPartBody(BasicString &data);
		void						processDefinedBody(BasicString &data);
		void						processCunkedBody(BasicString &data);
		void						processData(BasicString &data);

	public:
		void	setWorkers(RessourceHandler &explorer, Location &location, Request &request);
		void	executePost(BasicString &data);

		Post();
		Post(const Post &copy);
		Post &operator=(const Post &assign);
		~Post();
};
