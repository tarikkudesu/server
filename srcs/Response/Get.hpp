#pragma once

#include "Post.hpp"

class Get
{
	private:
		t_response_phase				__responsePhase;
		t_get_phase 					__phase;
		std::ifstream					__file;

		void							readFile(void);
		void							autoIndexing(void);

	public:
		void							executeGet(RessourceHandler &explorer, const Location &location, BasicString &body);

		Get(t_response_phase phase);
		Get(const Get &copy);
		Get &operator=(const Get &assign);
		~Get();
};
