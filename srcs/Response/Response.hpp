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

		void							reset();
		void							cgiPhase();
		void							getPhase();
		bool							checkCgi();
		void							setHeader();
		void							deleteFile();
		void							deletePhase();
		void							preparePhase();
		void							buildResponse();
		void							__check_methods();
		bool							shouldAuthenticate();
		void							postPhase(BasicString &data);

	public:
		void							setupWorkers(Server &server, Location &location);
		void							processData(BasicString &data);

		Response(t_connection_phase &phase, Request &request);
		Response(const Response &copy);
		Response &operator=(const Response &assign);
		~Response();
};

#endif
