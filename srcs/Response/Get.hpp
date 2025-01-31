#pragma once

#include "Post.hpp"

class Get
{
	private:
		Request&						__request;
		t_response_phase&				__responsePhase;
		t_get_phase 					__phase;
		std::ifstream					__file;
		Token							__token;

		void							readFile(void);
		void							autoIndexing(void);
		bool							authenticated();

	public:
		void							executeGet(RessourceHandler &explorer, Location &location, BasicString &body);

		Get(t_response_phase &phase, Request& request);
		Get(const Get &copy);
		Get &operator=(const Get &assign);
		~Get();
};
