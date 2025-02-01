#include "Response.hpp"

Response::Response(t_connection_phase &phase, Request &request) : __connectionPhase(phase),
                                                                  __responsePhase(PREPARING_RESPONSE),
                                                                  __getPhase(GET_INIT),
                                                                  __get(request, __responsePhase),
                                                                  __post(request, __responsePhase),
                                                                  __request(request)
{
}
// a implementer
Response::Response(const Response &copy) : __connectionPhase(copy.__connectionPhase),
                                           __responsePhase(copy.__responsePhase),
                                           __getPhase(copy.__getPhase),
                                           __get(copy.__get),
                                           __post(copy.__post),
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
        this->__connectionPhase = assign.__connectionPhase;
        this->__responsePhase = assign.__responsePhase;
        this->__getPhase = assign.__getPhase;
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
    }
    return *this;
}
void Response::reset()
{
    __get.reset();
    __post.reset();
    __responsePhase = PREPARING_RESPONSE;
    __connectionPhase = PROCESSING_REQUEST;
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
    __responsePhase = RESPONSE_DONE;
}
void Response::cgiPhase()
{
    wsu::info("CGI phase");
    Cgi cgi(explorer, __request, *__location, __body);
    __body = cgi.getBody();
    buildResponse();
    __responsePhase = RESPONSE_DONE;
}
void Response::autoindex()
{
    t_svec directories;
    DIR *dir = opendir(explorer.__fullPath.c_str());
    if (!dir)
        throw ErrorResponse(500, *__location, "could not open directory");
    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        directories.push_back(entry->d_name);
        directories.push_back(" ");
    }
    closedir(dir);
    String body = wsu::buildListingBody(explorer.__fullPath, directories);
    buildResponse();
    __responsePhase = RESPONSE_DONE;
}
void Response::getProcess()
{
    if (__getPhase == GET_INIT)
    {
        wsu::info("preparing Get");
        buildResponse();
        __get.setWorkers(explorer, *__location, *__server);
        __getPhase = GET_EXECUTE;
    }
    else
    {
        wsu::info("executing Get");
        __get.executeGet(__body);
        if (__responsePhase == RESPONSE_DONE)
            __getPhase = GET_INIT;
    }
}
void Response::getPhase()
{
    wsu::info("Get phase");
    if (checkCgi())
        __responsePhase = CGI_PROCESS;
    else
    {
        wsu::info("GET in phase");
        if (explorer.__type == FILE_)
            getProcess();
        else if (__location->__autoindex)
            autoindex();
        else
            throw ErrorResponse(403, "Forbidden");
    }
}
void Response::postPhase(BasicString &data)
{
    wsu::info("post phase");
    __post.setWorkers(explorer, *__location, *__server);
    __post.executePost(data);
    if (__responsePhase == RESPONSE_DONE)
    {
        buildResponse();
        reset();
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
        if (__responsePhase == RESPONSE_DONE)
            reset();
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
