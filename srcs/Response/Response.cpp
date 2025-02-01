#include "Response.hpp"

Response::Response(t_connection_phase &phase, Request &request) : __connectionPhase(phase),
                                                                  __responsePhase(PREPARING_RESPONSE),
                                                                  __get(request, __responsePhase),
                                                                  __post(request, __responsePhase),
                                                                  __request(request),
                                                                  __tmp(true)
{
}
// a implementer
Response::Response(const Response &copy) : __connectionPhase(copy.__connectionPhase),
                                           __responsePhase(PREPARING_RESPONSE),
                                           __get(copy.__request, __responsePhase),
                                           __post(copy.__request, __responsePhase),
                                           __request(copy.__request),
                                           __tmp(copy.__tmp)
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
        this->__connectionPhase = assign.__connectionPhase;
        this->__responsePhase = assign.__responsePhase;
        this->__body = assign.__body;
        this->__get = assign.__get;
        this->__post = assign.__post;
        this->explorer = assign.explorer;
        this->__server = assign.__server;
        this->__request = assign.__request;
        this->__location = assign.__location;
        this->reasonPhrase = assign.reasonPhrase;
        this->headers = assign.headers;
        this->code = assign.code;
        this->__tmp = assign.__tmp;
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

BasicString Response::getResponse()
{
    return __body;
}
void Response::deleteFile(void)
{
    if (unlink(explorer.__fullPath.c_str()) != 0)
        throw ErrorResponse(500, *__location, "unlink");
}
bool Response::shouldAuthenticate()
{
    return !__location->__authenticate.empty();
}
void Response::buildResponse()
{
    code = 200;
    reasonPhrase = "OK";
    __body.clear();
    setHeader();
    __body.join(PROTOCOLE_V " " + wsu::intToString(code) + " " + reasonPhrase + LINE_BREAK);
    for (std::map<String, String>::iterator it = headers.begin(); it != headers.end(); ++it)
        __body.join(it->first + ": " + it->second + "\r\n");
    __body.join(String("\r\n"));
}
void Response::setHeader()
{
    headers["Accept-Ranges"] = "none";
    headers["Connection"] = "keep-alive";
    headers["Content-Length"] = wsu::intToString(wsu::getFileSize(explorer.__fullPath));
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
    wsu::info("Delete phase");
    deleteFile();
    buildResponse();
    __responsePhase = PREPARING_RESPONSE;
    __connectionPhase = PROCESSING_REQUEST;
}
void Response::cgiPhase()
{
    wsu::info("CGI phase");
    Cgi cgi(explorer, __request, *__location, __body);
    __body = cgi.getBody();
    buildResponse();
    __responsePhase = PREPARING_RESPONSE;
    __connectionPhase = PROCESSING_REQUEST;
}
void Response::getPhase()
{
    wsu::info("Get phase");
    if (checkCgi())
        __responsePhase = CGI_PROCESS;
    else
    {
        if (__tmp == true)
        {
            wsu::info("preparing Get");
            buildResponse();
            std::cout << __body << "\n";
            __get.setWorkers(explorer, *__location, *__server);
            __tmp = false;
        }
        else
        {
            wsu::info("executing Get");
            __get.executeGet(__body);
            if (__responsePhase == PREPARING_RESPONSE)
            {
                __tmp = true;
                __connectionPhase = PROCESSING_REQUEST;
            }
        }
    }
}
void Response::postPhase(BasicString &data)
{
    wsu::info("post phase");
    __post.setWorkers(explorer, *__location, *__server);
    __post.executePost(data);
    if (__responsePhase == PREPARING_RESPONSE)
    {
        __connectionPhase = PROCESSING_REQUEST;
        buildResponse();
    }
}
void Response::preparePhase()
{
    wsu::info("preparing response");
    __body.clear();
    this->explorer.prepareRessource(*__location, __request.__URI);
    std::vector<t_method>::iterator it = __location->__allowMethods.begin();
    for (; it != __location->__allowMethods.end() && *it != __request.__method; it++)
        ;
    if (it == __location->__allowMethods.end())
        throw ErrorResponse(405, *__location, wsu::methodToString(__request.__method) + " : method not allowed in this location");
    __check_methods();
    std::cout << explorer;
}
void Response::processData(BasicString &data)
{
    wsu::info("processing response");
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
    catch (int &i)
    {
        exit(1);
    }
}
