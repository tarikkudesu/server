#ifndef __ERRORRESPONSE_HPP__
# define __ERRORRESPONSE_HPP__

# include "../ServerManager/Server.hpp"
class ErrorResponse
{
	private :

		static String						__errPage;
        String								__page;
		int16_t								__code;
		String								__Body;
		String								__headers;
		Location							*__location;
		String								__StatusLine;
		String 								__indication;
		String 								__redirection;
		String								__reasonPhrase;

		void								buildStatusLine();
		void								buildHeaderFeilds();
		void								buildResponseBody();
		void								constructErrorPage();
		String								readFielContent(String fileName);
		ErrorResponse();

	public:
		BasicString							getResponse() const;
		void								print() const;

		ErrorResponse( const ErrorResponse &copy );
		ErrorResponse( int code, String indication );
		ErrorResponse(String redirection, String setCookie);
		ErrorResponse( int code, Location &location, String indication );
		ErrorResponse( int code, String redirection , Location &location);
		ErrorResponse	&operator=( const ErrorResponse &assign );
		~ErrorResponse();
};

std::ostream &operator<<(std::ostream &o, const ErrorResponse &r);

#endif
