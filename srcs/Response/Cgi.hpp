#ifndef __CGI_HPP__
#define __CGI_HPP__

# include "Get.hpp"

class Cgi
{
	private:
		Request 					&__request;

		FileExplorer				*__explorer;
		Location					*__location;

		int 						__fd;
		String						__file;
		char						**env;

		void						execute(const char* path);
		String						getQueryString( BasicString &reqBody );
		void						setCgiEnvironement( BasicString &reqBody );


	public:
		void						reset();
		void						setWorkers(FileExplorer &explorer, Location &location);
		void						processData( BasicString &reqBody );
		String						&getFileName();

		Cgi(Request &request);
		Cgi(const Cgi& cgi);
		Cgi	&operator=(const Cgi& cgi);
		~Cgi();
};

#endif