#ifndef __REQUEST_HPP__
# define __REQUEST_HPP__

# include "Headers.hpp"

class Request
{
	private:
		t_connection_phase				&__phase;
		String							__requestLine;
		String							__requestHeaders;

		void							clear();
		void							validateURI();
		void							parseRequest();
		void							validateHeaders();
		void							validateRequestLine();

	public:
		String							__URI;
		t_method						__method;
		Headers							__headers;
        size_t                          __bodySize;
		String							__fragement;
		String							__protocole;
		String							__queryString;
		std::map< String, String >		__headerFeilds;

		void							processData(BasicString &data);

		Request(t_connection_phase &phase);
		Request( const Request &copy );
		Request& operator=( const Request &assign );
		~Request();
};

std::ostream &operator<<(std::ostream &o, const Request &req );

#endif
