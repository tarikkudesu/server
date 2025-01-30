#pragma once

# include "../Request/Request.hpp"

class RessourceHandler
{
	private:
		void			loadType(const char* path);
		void			loadPathExploring(void);
        void            clear();

	public:
		String 		    __URI;
		t_type			__type;
		String			__fullPath;
		Location		*__location;

		void			prepareRessource(const Location& location, const String& uri);

		RessourceHandler();
		RessourceHandler(const RessourceHandler &copy);
		RessourceHandler &operator=(const RessourceHandler &assign);
		~RessourceHandler();
};

std::ostream &operator<<(std::ostream &o, RessourceHandler const &r);
