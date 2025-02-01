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
    }
    return *this;
}
void Response::reset()
{
    __get.reset();
    __post.reset();
    __getPhase = GET_INIT;
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
void Response::buildResponse(int code, size_t length)
{
    __body.clear();
    String reasonPhrase;
    std::map<int16_t, String>::iterator it = wsu::__errCode.find(code);
    if (it != wsu::__errCode.end())
        reasonPhrase = it->second;
    else
        reasonPhrase = "OK";
    __body.join(PROTOCOLE_V " " + wsu::intToString(code) + " " + reasonPhrase + LINE_BREAK);
    __body.join("Content-Type: " + wsu::getContentType(explorer.__fullPath) + "; charset=UTF-8" + LINE_BREAK);
    __body.join("date: " + wsu::buildIMFDate() + LINE_BREAK);
    __body.join(String("Accept-Ranges: none") + LINE_BREAK);
    __body.join(String("server: webserv/1.0") + LINE_BREAK);
    __body.join(String("Connection: keep-alive") + LINE_BREAK);
    if (length)
        __body.join("Content-Length: " + wsu::intToString(length) + LINE_BREAK);
    // if (!request->__headers.__cookie.empty())
    //     headers["cookie"] = "token=" + __request.__headers.__cookie + "; expires=Thu, 31 Dec 2025 12:00:00 UTC;";
    __body.join(String(LINE_BREAK));
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
void Response::autoindex()
{
    wsu::info("autoindex");
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
    String body = wsu::buildListingBody(__request.__URI, directories);
    buildResponse(200, body.length());
    __body.join(body);
    __responsePhase = RESPONSE_DONE;
}
void Response::getProcess()
{
    if (__getPhase == GET_INIT)
    {
        buildResponse(200, wsu::getFileSize(explorer.__fullPath));
        __get.setWorkers(explorer, *__location, *__server);
        __getPhase = GET_EXECUTE;
    }
    else
    {
        __get.executeGet(__body);
        if (__responsePhase == RESPONSE_DONE)
            __getPhase = GET_INIT;
    }
}
/******************************************************************************
 *                                   PHASES                                   *
 ******************************************************************************/
void Response::getPhase()
{
    wsu::info("Get phase");
    if (checkCgi())
        __responsePhase = CGI_PROCESS;
    else
    {
        if (explorer.__type == FILE_)
            getProcess();
        else if (__location->__autoindex)
            autoindex();
        else
            throw ErrorResponse(403, "Forbidden");
    }
}
void Response::deletePhase()
{
    wsu::info("Delete phase");
    deleteFile();
    buildResponse(204, 0);
    __responsePhase = RESPONSE_DONE;
}
void Response::cgiPhase()
{
    wsu::info("CGI phase");
    Cgi cgi(explorer, __request, *__location, __body);
    buildResponse(200, cgi.getBody().length());
    __body.join(cgi.getBody());
    __responsePhase = RESPONSE_DONE;
}
void Response::postPhase(BasicString &data)
{
    wsu::info("post phase");
    __post.setWorkers(explorer, *__location, *__server);
    __post.executePost(data);
    if (__responsePhase == RESPONSE_DONE)
    {
        String r = "<h2>Success!</h2>";
        buildResponse(201, r.length());
        __body.join(r);
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
}
