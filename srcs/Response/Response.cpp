#include "Response.hpp"

Response::Response(t_connection_phase &phase, Request &request) : __connectionPhase(phase),
                                                                  __responsePhase(PREPARING_RESPONSE),
                                                                  __get(request, __responsePhase),
                                                                  __post(request, __responsePhase),
                                                                  __request(request)
{
}
// a implementer
Response::Response(const Response &copy) : __connectionPhase(copy.__connectionPhase),
                                           __responsePhase(PREPARING_RESPONSE),
                                           __get(copy.__request, __responsePhase),
                                           __post(copy.__request, __responsePhase),
                                           __request(copy.__request)
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
void Response::reset()
{
    __post.reset();
    __responsePhase = PREPARING_RESPONSE;
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
void Response::__check_methods()
{
    if (__request.__method == GET)
        __responsePhase = GET_PROCESS;
    else if (__request.__method == POST)
        __responsePhase = POST_PROCESS;
    else if (__request.__method == DELETE)
        __responsePhase = DELETE_PROCESS;
    else
        throw ErrorResponse(405, *__location, "Server does not implement this method");
}
void Response::setupWorkers(Server &server, Location &location)
{
    this->__server = &server;
    this->__location = &location;
}
void Response::deletePhase()
{
    deleteFile();
    buildResponse();
    __responsePhase = PREPARING_RESPONSE;
    throw __body;
}
void Response::cgiPhase()
{
    Cgi cgi(explorer, __request, *__location, __body);
    __body = cgi.getBody();
    buildResponse();
    __responsePhase = PREPARING_RESPONSE;
}
void Response::getPhase()
{
    if (checkCgi())
        __responsePhase = CGI_PROCESS;
    else
    {
        if (__get.__phase == GET_IN)
        {
            __body.clear();
            buildResponse();
            __get.setWorkers(explorer, *__location, *__server);
        }
        __get.executeGet(__body);
    }
}
void Response::postPhase(BasicString &data)
{
    __post.setWorkers(explorer, *__location, *__server);
    __post.executePost(data);
    if (__responsePhase == PREPARING_RESPONSE)
    {
        buildResponse();
        throw __body;
    }
}
void Response::preparePhase()
{
    __body.clear();
    this->explorer.prepareRessource(*__location, __request.__URI);
    std::vector<t_method>::iterator it = __location->__allowMethods.begin();
    for (; it != __location->__allowMethods.end() && *it != __request.__method; it++)
        ;
    if (it == __location->__allowMethods.end())
        throw ErrorResponse(405, *__location, wsu::methodToString(__request.__method) + " : method not allowed in this location");
    __check_methods();
}
void Response::processData(BasicString &data)
{
    try
    {
        if (__responsePhase == PREPARING_RESPONSE)
            preparePhase();
        if (__responsePhase == POST_PROCESS)
            postPhase(data);
        if (__responsePhase == GET_PROCESS)
            getPhase();
        if (__responsePhase == CGI_PROCESS)
            cgiPhase();
        if (__responsePhase == DELETE_PROCESS)
            deletePhase();
    }
    catch (ErrorResponse &e)
    {
        reset();
        throw e;
    }
}
