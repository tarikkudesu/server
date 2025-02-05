#ifndef __CGI_HPP__
#define __CGI_HPP__

# include "Get.hpp"

class Cgi
{
	private:
		Request&				__request;
		FileExplorer&		    __explorer;
		Location&				__location;
        BasicString             __reqBody;
		std::time_t				__start;
		String					__body;
		char					**env;

		void					execute(const char* path, int fd);
		void					readFromFile(String& file);
		void					setCgiEnvironement();
		void					cgiProcess(void);
		void					clear( void );
		String					getQueryString();


	public:
		String& 				 getBody();

		Cgi(FileExplorer &explorer, Request &request, Location &location, BasicString &body);
		~Cgi();
		// Cgi(const Cgi &copy);
		// Cgi &operator=(const Cgi &assign);
};

#endif