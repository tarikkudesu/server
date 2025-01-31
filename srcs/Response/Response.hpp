#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__

#include "Cgi.hpp"

class Response
{
	private:
		t_connection_phase				&__connectionPhase;
		t_response_phase				__responsePhase;
		BasicString 					__body;
		Get 							__get;
		Post							__post;
		RessourceHandler				explorer;
		Server							*__server;
		Request							&__request;
		Location						*__location;
		String							reasonPhrase;
		std::map<String, String>		headers;
		int 							code;
		Token							token;


		void							__check_methods();
		void							buildResponse();
		void							setHeader();
		void							executeGet(void);
		void							executePost(void);
		void							executeDelete(void);
		void							executeCgi(void);
		bool							checkCgi();
		void							deleteFile();
		void							executeAuth();
		bool							authenticated();
		bool							shouldAuthenticate();


	public:
		const std::vector<BasicString>&	getResponse(void) const;
		String							getMethod(t_method Method);
		void							print() const;

		void							setupWorkers(Request &request, Server &server, Location &location);
		void							processData(BasicString &data);

		Response(t_connection_phase &phase, Request &request);
		Response(const Response &copy);
		Response &operator=(const Response &assign);
		~Response();
};

std::ostream &operator<<(std::ostream &o, const Response &r);

#endif
