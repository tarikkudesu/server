#ifndef __CGI_HPP__
#define __CGI_HPP__

# include "Get.hpp"

class Cgi
{
	private:
		FileExplorer&		    __explorer;
		Location&				__location;
		Request&				__request;
        BasicString             __reqBody;
		std::ofstream			__stream;
		std::time_t				__start;
		String					__body;
		String					__file;
		char					**env;

		void					execute(const char* path, int fd);
		void					setCgiEnvironement( void );
		String					getQueryString( void );
		void					readFromFile( void );
		void					cgiProcess( void );
		void					clear( void );


	public:
		String& 				getBody();

		Cgi(FileExplorer &explorer, Request &request, Location &location, BasicString &body);
		Cgi(const Cgi& cgi);
		Cgi	&operator=(const Cgi& cgi);
		~Cgi();
};

#endif