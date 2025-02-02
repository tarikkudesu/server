#include "Response.hpp"

Response::Response(t_connection_phase &phase, Request &request) : __connectionPhase(phase),
                                                                  __responsePhase(PREPARING_RESPONSE),
                                                                  __postPhase(POST_INIT),
                                                                  __getPhase(GET_INIT),
                                                                  __get(request, __responsePhase),
                                                                  __post(request, __responsePhase),
                                                                  __request(request)
{
}
// a implementer
Response::Response(const Response &copy) : __connectionPhase(copy.__connectionPhase),
                                           __responsePhase(copy.__responsePhase),
                                           __postPhase(copy.__postPhase),
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
        this->__get = assign.__get;
        this->__body = assign.__body;
        this->__post = assign.__post;
        this->explorer = assign.explorer;
        this->__server = assign.__server;
        this->__request = assign.__request;
        this->__location = assign.__location;
        this->__getPhase = assign.__getPhase;
        this->__postPhase = assign.__postPhase;
        this->__responsePhase = assign.__responsePhase;
        this->__connectionPhase = assign.__connectionPhase;
    }
    return *this;
}
void Response::reset()
{
    __get.reset();
    __post.reset();
    __getPhase = GET_INIT;
    __postPhase = POST_INIT;
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
bool Response::shouldAuthenticate()
{
    if (__location->__authenticate.size() == 2)
        return explorer.__fullPath.compare(__location->__authenticate[0]);
    return false;
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
    if (!__request.__headers.__cookie.empty())
        __body.join("Set-Cookie: token=" + __request.__headers.__cookie + "; expires=Thu, 31 Dec 2025 12:00:00 UTC;");
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

/*************************************************************************************
 *                                  BODY PROCESSING                                  *
 *************************************************************************************/

void Response::postDone()
{
    if (__location->__authenticate.size() == 2)
    {
        String form = __post.getForm().to_string();
        wsu::decode(form);
        if (!__server->authentified(form))
            __request.__headers.__cookie = "token=" + __server->addUserInDb(form);
        else
            __request.__headers.__cookie = "token=" +__server->getCookie(form);
        explorer.changeRequestedFile(__location->__authenticate[0]);
        std::cout << "new cookie " << __request.__headers.__cookie << "\n";
        __responsePhase = GET_PROCESS;
    }
    else
        __responsePhase = CGI_PROCESS;
    __post.reset();
}
void Response::processCunkedBody(BasicString &data)
{
    wsu::info("Post chunked body");
    static size_t chunkSize;
    do
    {
        if (chunkSize == 0)
        {
            size_t pos = data.find(LINE_BREAK);
            if (pos == String::npos && data.length() > REQUEST_MAX_SIZE)
            {
                data.clear();
                __request.__headers.__connectionType = CLOSE;
                throw ErrorResponse(400, *__location, "Oversized chunk value");
            }
            else if (pos == String::npos)
                throw wsu::persist();
            BasicString hex = data.substr(0, pos);
            size_t extPos = hex.find(";");
            if (extPos != String::npos)
                hex = hex.substr(0, extPos);
            chunkSize = wsu::hexToInt(hex.to_string());
            data.erase(0, pos + 2);
            __request.__bodySize += pos + 2;
            if (chunkSize == 0)
                return postDone();
        }
        if (chunkSize < data.length())
        {
            BasicString tmp = data.substr(0, chunkSize);
            __post.processData(tmp);
            data.erase(0, chunkSize);
            __request.__bodySize += chunkSize;
            chunkSize -= data.length();
        }
        else
        {
            BasicString tmp = data;
            __post.processData(tmp);
            size_t len = data.length();
            __request.__bodySize += len;
            chunkSize -= len;
            data.clear();
            break;
        }
    } while (true);
}
void Response::processDefinedBody(BasicString &data)
{
    wsu::info("Post defined body");
    if (__request.__headers.__contentLength < data.length())
    {
        BasicString tmp = data.substr(0, __request.__headers.__contentLength);
        __request.__headers.__contentLength -= tmp.length();
        __request.__bodySize += tmp.length();
        data.erase(0, tmp.length());
        __post.processData(tmp);
    }
    else
    {
        BasicString tmp = data;
        __request.__bodySize += data.length();
        __request.__headers.__contentLength -= data.length();
        __post.processData(tmp);
        data.clear();
    }
    if (__request.__headers.__contentLength == 0)
        postDone();
}
/*************************************************************************************
 *                                        GET                                        *
 *************************************************************************************/

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
bool Response::authenticated()
{
    if (__request.__headers.__cookie.empty())
        return false;
    t_svec cookies = wsu::splitByChar(__request.__headers.__cookie, ';');
    for (t_svec::iterator it = cookies.begin(); it != cookies.end(); it++)
    {
        t_svec cook = wsu::splitByChar(*it, '=');
        if (cook.size() == 2 && __server->authentified(cook[1]))
            return true;
    }
    return false;
}
void Response::getProcess()
{
    if (__getPhase == GET_INIT)
    {
        if (__location->__authenticate.size() == 2 &&
            wsu::samePath(explorer.__fullPath, wsu::joinPaths(__location->__root, __location->__authenticate[0])))
        {

            if (!authenticated())
                throw ErrorResponse(301, __location->__authenticate[1], *__location);
        }
        __get.setWorkers(explorer, *__location, *__server);
        buildResponse(200, wsu::getFileSize(explorer.__fullPath));
        __getPhase = GET_EXECUTE;
    }
    else
        __get.executeGet(__body);
}

/******************************************************************************
 *                                   PHASES                                   *
 ******************************************************************************/

void Response::deletePhase()
{
    wsu::info("Delete phase");
    if (unlink(explorer.__fullPath.c_str()) != 0)
        throw ErrorResponse(500, *__location, "unlink");
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
            throw ErrorResponse(403, "not a file not autoindex");
    }
}

void Response::postPhase(BasicString &data)
{
    wsu::info("post phase");
    if (__postPhase == POST_INIT)
    {
        __post.setWorkers(explorer, *__location, *__server);
        __postPhase = POST_EXECUTE;
    }
    else
    {
        if (__request.__headers.__transferType == DEFINED)
            processDefinedBody(data);
        else if (__request.__headers.__transferType == CHUNKED)
            processCunkedBody(data);
        else
            throw ErrorResponse(400, *__location, "Missing Content-Length or Transfer-Encoding");
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
