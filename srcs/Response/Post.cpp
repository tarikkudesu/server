#include "Post.hpp"

Post::Post(Request &request, t_response_phase &phase) : request(request),
                                                        __responsePhase(phase),
                                                        explorer(NULL),
                                                        location(NULL),
                                                        server(NULL),
                                                        __phase(MP_INIT)
{
}

Post::Post(const Post &copy) : request(copy.request),
                               __responsePhase(copy.__responsePhase),
                               explorer(copy.explorer),
                               location(copy.location),
                               server(copy.server),
                               __phase(copy.__phase),
                               __data(copy.__data)
{
    *this = copy;
}

Post &Post::operator=(const Post &assign)
{
    if (this != &assign)
    {
        this->__data = assign.__data;
        this->__form = assign.__form;
        this->server = assign.server;
        this->__phase = assign.__phase;
        this->request = assign.request;
        this->explorer = assign.explorer;
        this->location = assign.location;
        this->__responsePhase = assign.__responsePhase;
    }
    return *this;
}

Post::~Post()
{
    reset();
}

/****************************************************************************
 *                               MINI METHODS                               *
 ****************************************************************************/

void Post::reset()
{
    if (__fs.is_open())
        __fs.close();
    __phase = MP_INIT;
    __responsePhase = RESPONSE_DONE;
    request.__headers.__boundary.clear();
    __form.clear();
}
BasicString &Post::getForm()
{
    return this->__form;
}
void Post::writeDataIntoFile(BasicString &data)
{
    __fs.write(data.getBuff(), data.length());
}
void Post::createFile(std::vector<String> &headers)
{
    if (__fs.is_open())
        return;
    for (std::vector<String>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        if (it->find("Content-Disposition:") != String::npos)
        {

            size_t pos = it->find("filename=\"");
            if (pos != String::npos)
            {
                size_t startPos = pos + 10;
                size_t endPos = it->find("\"", startPos);
                String file = it->substr(startPos, endPos - startPos);
                if (file.empty())
                    return;
                __fs.open(wsu::joinPaths(explorer->__fullPath, file).c_str());
                if (!__fs.is_open())
                    throw ErrorResponse(403, *location, "Could not create file on the server1");
                std::cout << "Succesfully extracted\n";
                return;
            }
            else
                throw ErrorResponse(400, *location, "Malformed Content-Disposition header.2");
        }
        else
            throw ErrorResponse(400, *location, "Malformed Content-Disposition header.2");
    }
    throw ErrorResponse(400, *location, "Malformed Content-Disposition header.3");
}

void Post::mpBody()
{
    wsu::info("Post multipart data body");
    size_t end = __data.find(LINE_BREAK "--" + this->request.__headers.__boundary + "--" LINE_BREAK);
    size_t pos = __data.find(LINE_BREAK "--" + this->request.__headers.__boundary + LINE_BREAK);
    if (pos == String::npos && end == String::npos)
    {
        size_t len = this->request.__headers.__boundary.length() + 6;
        if (__data.length() <= len)
            throw wsu::persist();
        BasicString tmp = __data.substr(0, __data.length() - len);
        __data.erase(0, tmp.length());
        writeDataIntoFile(tmp);
    }
    else if (pos != String::npos)
    {
        size_t len = this->request.__headers.__boundary.length() + 4;
        BasicString tmp = __data.substr(0, pos);
        writeDataIntoFile(tmp);
        __data.erase(0, pos + len);
        __phase = MP_HEADERS;
        __fs.close();
    }
    else if (end != String::npos)
    {
        size_t len = this->request.__headers.__boundary.length() + 6;
        BasicString tmp = __data.substr(0, end);
        writeDataIntoFile(tmp);
        __data.erase(0, end + len);
        __responsePhase = RESPONSE_DONE;
        reset();
    }
}
void Post::mpHeaders()
{
    wsu::info("Post form data headers");
    size_t pos = __data.find(D_LINE_BREAK);
    if (pos == String::npos && __data.length() > REQUEST_MAX_SIZE)
        throw ErrorResponse(400, *location, "Oversized headers");
    else if (pos == String::npos)
        throw wsu::persist();
    std::vector<String> headers;
    __phase = MP_BODY;
    do
    {
        size_t p = __data.find(LINE_BREAK);
        if (p == 0 || p == String::npos)
            break;
        headers.push_back(__data.substr(0, p).to_string());
        std::cout << RED << headers.back() << "\n"
                  << RESET;
        __data.erase(0, p + 2);
    } while (true);
    __data.erase(0, 2);
    createFile(headers);
}
void Post::mpInit()
{
    wsu::info("Post multipart data init");
    size_t pos1 = __data.find("--" + request.__headers.__boundary + LINE_BREAK);
    size_t pos2 = __data.find("--" + request.__headers.__boundary + "--" LINE_BREAK);
    if (pos1 == String::npos && pos2 == String::npos && __data.length() < request.__headers.__boundary.length() + 6)
        throw wsu::persist();
    if (pos2 == 0)
    {
        __data.erase(0, request.__headers.__boundary.length() + 6);
        throw ErrorResponse(400, *location, "No files were uploaded");
    }
    if (pos1 != 0)
        throw ErrorResponse(400, *location, "Multipart/data-from: boundry mismatch");
    __data.erase(0, request.__headers.__boundary.length() + 4);
    __phase = MP_HEADERS;
}
void Post::processMultiPartBody()
{
    wsu::info("Post multipart data");
    if (__phase == MP_INIT)
        mpInit();
    if (__phase == MP_HEADERS)
        mpHeaders();
    if (__phase == MP_BODY)
        mpBody();
}
void Post::processFormData()
{
    wsu::info("Post form data");
    __form.join(__data);
    if (__form.length() > FORM_MAX_SIZE)
        throw ErrorResponse(415, *location, "Oversized Form");
    if (location->__authenticate.size())
    {
        if (!server->authentified(__form.to_string()))
            request.__headers.__cookie = server->addUserInDb(__form.to_string());
        else
            request.__headers.__cookie = server->getCookie(__form.to_string());
        explorer->changeRequestedFile(location->__authenticate[0]);
        __responsePhase = GET_PROCESS;
    }
    else
        __responsePhase = CGI_PROCESS;
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::setWorkers(FileExplorer &explorer, Location &location, Server &server)
{
    this->location = &location;
    this->explorer = &explorer;
    this->server = &server;
}
void Post::processData(BasicString &data)
{
    __data.join(data);
    try
    {
        if (request.__bodySize > location->__clientBodyBufferSize)
            throw ErrorResponse(413, *location, "Request body too large.");
        if (request.__headers.__contentType == FORM)
            processFormData();
        else if (request.__headers.__contentType == MULTIPART)
            processMultiPartBody();
        else
            throw ErrorResponse(415, *location, "Content-Type not supported");
    }
    catch (ErrorResponse &e)
    {
        this->request.__headers.__connectionType = CLOSE;
        __data.clear();
        reset();
        throw e;
    }
}
