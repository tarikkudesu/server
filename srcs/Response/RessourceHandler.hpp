#pragma once

# include "../Request/Request.hpp"

class RessourceHandler
{
	private:
		void			loadType(const char* path);

	public:
		t_type			__type;
		String			__fullPath;
		Location		*__location;

		void			changeRequestedFile(String file);
		void			loadPathExploring(const String& uri);
		void			prepareRessource(Location& location, const String& uri);

		RessourceHandler();
		RessourceHandler(const RessourceHandler &copy);
		RessourceHandler &operator=(const RessourceHandler &assign);
		~RessourceHandler();
};

std::ostream &operator<<(std::ostream &o, RessourceHandler const &r);
