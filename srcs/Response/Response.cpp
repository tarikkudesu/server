#include "Response.hpp"

Response::Response(t_connection_phase &phase, Request &request) : __connectionPhase(phase),
																  __responsePhase(PREPARING_RESPONSE),
																  __get(__responsePhase, request)
{
	// __check_methods();
	// buildResponse();
}
// a implementer
Response::Response(const Response &copy)
{
	*this = copy;
}
Response::~Response()
{
}
// a implementer
Response &Response::operator=(const Response &assign)
{
	if (this != &assign)
	{
	}
	return *this;
}
/***********************************************************************
 *								 METHODS							   *
 ***********************************************************************/

void Response::deleteFile(void)
{
	if (unlink(explorer.__fullPath.c_str()) != 0)
		throw ErrorResponse(500, *__location, "Internal Server Error");
}

bool Response::shouldAuthenticate()
{
	return !__location->__authenticate.empty();
}

// void Response::executePost()
// {
// 	//verify if the post content shouldnt be reconstructed;
// 	if (__request.__headers.__transferType != MULTIPART && !__location.__authenticate.empty())
// 	{
// 		String cookie;
// 		String data = readFielContent(__request.__body[0]._fileName);
// 		if (!token.userInDb(data))
// 		{
// 			cookie = token.addUserInDb(data, __server.serverIdentity());
// 			if (!cookie.empty())
// 				__server.__tokenDB.insert(std::make_pair(cookie, data));
// 		}
// 		throw ErrorResponse(explorer.__fullPath, cookie);
// 	}
// 	Post post(explorer, __request);
// 	code = 200;
// 	reasonPhrase = "Ok";
// }

void Response::buildResponse()
{
	setHeader();
	BasicString resMsg = PROTOCOLE_V " " + wsu::intToString(code) + " " + reasonPhrase + LINE_BREAK;
	for (std::map<String, String>::iterator it = headers.begin(); it != headers.end(); ++it)
		resMsg.join(it->first + ": " + it->second + "\r\n");
	resMsg.join(String("\r\n"));
}
void Response::setHeader()
{
	headers["Accept-Ranges"] = "none";
	headers["Connection"] = "keep-alive";
	headers["content-length"] = wsu::intToString(wsu::getFileSize(explorer.__fullPath));
	headers["Content-Type"] = wsu::getContentType(explorer.__fullPath) + "; charset=UTF-8";
	headers["server"] = "webserv/1.0";
	headers["date"] = wsu::buildIMFDate();
	// if (!request->__headers.__cookie.empty())
	// 	headers["cookie"] = "token=" + __request.__headers.__cookie + "; expires=Thu, 31 Dec 2025 12:00:00 UTC;";
}
BasicString &Response::getResponse() const
{
	return __body;
}
bool Response::checkCgi()
{
	if (__location->__cgiPass.empty())
		return 0;
	if (explorer.__type == FOLDER)
		return 0;
	if (!wsu::endWith(explorer.__fullPath, ".java") && !wsu::endWith(explorer.__fullPath, ".php"))
		return 0;
	return 1;
}
void Response::executeCgi()
{
	Cgi cgi(explorer, *__request, *__location);
	__body = cgi.getBody();
}
void Response::__check_methods()
{
	if (checkCgi())
		executeCgi();
	if (__request->__method == GET)
		__responsePhase = GET_PROCESS;
	else if (__request->__method == POST)
		__responsePhase = POST_PROCESS;
	else if (__request->__method == DELETE)
		__responsePhase = DELETE_PROCESS;
	else
		throw ErrorResponse(405, *__location, "Server does not implement this method");
}
void Response::setupWorkers(Request &request, Server &server, Location &location)
{
	this->__server = &server;
	this->__request = &request;
	this->__location = &location;
}
void Response::processData(BasicString &data)
{
	if (__responsePhase == PREPARING_RESPONSE)
	{
		this->explorer.prepareRessource(*__location, __request->__URI);
		std::vector<t_method>::iterator it = __location->__allowMethods.begin();
		for (; it != __location->__allowMethods.end() && *it != __request->__method; it++)
			;
		if (it == __location->__allowMethods.end())
			throw ErrorResponse(405, *__location, wsu::methodToString(__request->__method) + " : method not allowed in this location");
		__check_methods();
	}
	if (__responsePhase == POST_PROCESS)
	{
		__post.setWorkers(explorer, *__location, *__request);
		__post.executePost(data);
		if (__responsePhase == PREPARING_RESPONSE)
		{
			buildResponse();
			throw __body;
		}
	}
	if (__responsePhase == CGI_PROCESS)
	{
		executeCgi();
		buildResponse();
		__responsePhase = PREPARING_RESPONSE;
	}
	if (__responsePhase == GET_PROCESS)
	{
		buildResponse();
		throw __body;
		__get.executeGet(explorer, *__location, __body);
	}
	if (__responsePhase == DELETE_PROCESS)
	{
		deleteFile();
		buildResponse();
		__responsePhase = PREPARING_RESPONSE;
		throw __body;
	}
}

void Response::print() const
{
	std::cout << explorer;
	for (std::vector<BasicString>::const_iterator it = body.begin(); it != body.end(); it++)
		std::cout << GREEN << *it << "\n";
}

std::ostream &operator<<(std::ostream &o, const Response &r)
{
	r.print();
	return o;
}
