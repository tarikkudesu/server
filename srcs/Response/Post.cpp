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
                               __phase(copy.__phase)
{
    *this = copy;
}

Post &Post::operator=(const Post &assign)
{
    if (this != &assign)
    {
        this->request = assign.request;
        this->__responsePhase = assign.__responsePhase;
        this->explorer = assign.explorer;
        this->location = assign.location;
        this->server = assign.server;
        this->__phase = assign.__phase;
        this->__boundary = assign.__boundary;
        this->__form = assign.__form;
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
    __responsePhase = PREPARING_RESPONSE;
    __boundary.clear();
    __form.clear();
}
void Post::writeDataIntoFile(BasicString &data)
{
    if (!(__fs << data))
    {
        reset();
        throw wsu::Close();
    }
}
void Post::createFile(std::vector<String> &headers)
{
    for (std::vector<String>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        size_t pos = it->find("Content-Disposition: form-data");
        if (pos != String::npos)
        {
            size_t filenamePos = it->find("filename\"", pos);
            if (filenamePos != String::npos)
            {
                size_t startPos = filenamePos + 9;
                size_t endPos = it->find("\"", startPos);
                if (endPos != String::npos)
                {
                    std::string fileName = it->substr(startPos, endPos - startPos);
                    __fs.open(fileName);
                    if (!__fs.is_open())
                        throw ErrorResponse(500, *location, "Could not create file on the server");
                    return;
                }
                else
                    throw ErrorResponse(400, *location, "Malformed Content-Disposition header.");
            }
            else
                throw ErrorResponse(400, *location, "Malformed Content-Disposition header.");
        }
    }
    throw ErrorResponse(400, *location, "Malformed Content-Disposition header.");
}
void Post::mpBody(BasicString &data)
{
    wsu::info("Post form data body");
    size_t end = data.find(LINE_BREAK "--" + this->__boundary + "--" LINE_BREAK);
    size_t pos = data.find(LINE_BREAK "--" + this->__boundary + LINE_BREAK);
    if (pos == String::npos && end == String::npos)
    {
        size_t len = this->__boundary.length() + 8;
        if (data.length() <= len)
            throw wsu::persist();
        BasicString tmp = data.substr(0, data.length() - len);
        writeDataIntoFile(tmp);
        data.erase(0, data.length());
        request.__bodySize += data.length();
    }
    else if (pos != String::npos)
    {
        size_t len = this->__boundary.length() + 6;
        BasicString tmp = data.substr(0, pos);
        writeDataIntoFile(tmp);
        data.erase(0, pos + len);
        request.__bodySize += pos + len;
        __phase = MP_HEADERS;
    }
    else if (end != String::npos)
    {
        size_t len = this->__boundary.length() + 8;
        BasicString tmp = data.substr(0, end);
        writeDataIntoFile(tmp);
        data.erase(0, end + len);
        request.__bodySize += end + len;
        __responsePhase = PREPARING_RESPONSE;
        reset();
    }
}
void Post::mpHeaders(BasicString &data)
{
    wsu::info("Post form data headers");
    size_t pos = data.find(D_LINE_BREAK);
    if (pos == String::npos)
    {
        if (data.length() > REQUEST_MAX_SIZE)
            throw ErrorResponse(400, *location, "Oversized headers");
        throw wsu::persist();
    }
    std::vector<String> headers;
    __phase = MP_BODY;
    do
    {
        size_t p = data.find(LINE_BREAK);
        if (p == 0 || p == String::npos)
            break;
        headers.push_back(data.substr(0, p).to_string());
        request.__bodySize += p + 2;
        data.erase(0, p + 2);
    } while (true);
    data.erase(0, 2);
    createFile(headers);
    request.__bodySize += 2;
}
void Post::setBoundry()
{
    t_svec elements = wsu::splitByChar(request.__headers.__contentType, ';');
    for (t_svec::iterator it = elements.begin(); it != elements.end(); it++)
    {
        wsu::trimSpaces(*it);
        t_svec kv = wsu::splitByChar(*it, '=');
        if (kv.size() != 2)
            continue;
        if (kv.at(0) == "boundary")
            __boundary = kv.at(1);
    }
    if (__boundary.empty())
        throw ErrorResponse(400, *location, "No boundry");
}
void Post::mpInit(BasicString &data)
{
    wsu::info("Post form data init");
    setBoundry();
    size_t pos1 = data.find("--" + __boundary + LINE_BREAK);
    size_t pos2 = data.find("--" + __boundary + "--" LINE_BREAK);
    if (pos1 == String::npos && pos2 == String::npos)
        throw wsu::persist();
    if (pos2 == 0)
    {
        data.erase(0, __boundary.length() + 6);
        throw ErrorResponse(400, *location, "No files were uploaded");
    }
    if (pos1 == String::npos)
        throw wsu::persist();
    else if (pos1 != 0)
    {
        data.erase(0, __boundary.length() + 6);
        throw ErrorResponse(400, *location, "Multipart/data-from: boundry mismatch");
    }
    data.erase(0, __boundary.length() + 4);
    __phase = MP_HEADERS;
}
void Post::processMultiPartBody(BasicString &data)
{
    wsu::info("Post form data");
    do
    {
        if (__phase == MP_INIT)
            mpInit(data);
        if (__phase == MP_HEADERS)
            mpHeaders(data);
        if (__phase == MP_BODY)
            mpBody(data);
    } while (__responsePhase != PREPARING_RESPONSE);
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::processData(BasicString &data)
{
    if (request.__bodySize > location->__clientBodyBufferSize)
        throw ErrorResponse(413, *location, "Request body too large.");
    if (request.__headers.__contentType == FORM_DATA)
    {
        wsu::info("Post form data");
        __form.join(data);
        if (__form.length() > FORM_MAX_SIZE)
            throw ErrorResponse(415, *location, "Content-Type not supported");
        if (location->__authenticate.size())
        {
            String cook;
            if (!server->authentified(__form.to_string()))
            {
                cook = server->addUserInDb(__form.to_string());
            }
            else
                cook = server->getCookie(__form.to_string());
            throw ErrorResponse(location->__authenticate[0], cook);
        }
        else
            __responsePhase = CGI_PROCESS;
        //verify the logic later
    }
    else if (request.__headers.__contentType == MULTIPART_DATA_FORM) // no exactly, you need to get the boundry
        processMultiPartBody(data);
    else
        throw ErrorResponse(415, *location, "Content-Type not supported");
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::processCunkedBody(BasicString &data)
{
    wsu::info("Post chunked body");
    static size_t chunkSize;
    do
    {
        if (chunkSize == 0)
        {
            size_t pos = data.find(LINE_BREAK);
            if (pos == String::npos)
            {
                if (data.length() > REQUEST_MAX_SIZE)
                    throw ErrorResponse(400, *location, "Oversized chunk value");
                throw wsu::persist();
            }
            BasicString hex = data.substr(0, pos);
            size_t extPos = hex.find(";");
            if (extPos != String::npos)
                hex = hex.substr(0, extPos);
            chunkSize = wsu::hexToInt(hex.to_string());
            data.erase(0, pos + 2);
            request.__bodySize += pos + 2;
            if (chunkSize == 0)
            {
                __responsePhase = PREPARING_RESPONSE;
                return;
            }
        }
        if (chunkSize < data.length())
        {
            BasicString tmp = data.substr(0, chunkSize);
            processData(tmp);
            data.erase(0, chunkSize);
            request.__bodySize += chunkSize;
            chunkSize -= data.length();
        }
        else
        {
            BasicString tmp = data;
            processData(tmp);
            size_t len = data.length();
            chunkSize -= len;
            request.__bodySize += len;
            data.clear();
            break;
        }
    } while (true);
}
void Post::processDefinedBody(BasicString &data)
{
    wsu::info("Post defined body");
    if (request.__headers.__contentLength < data.length())
    {
        BasicString tmp = data.substr(0, request.__headers.__contentLength);
        request.__headers.__contentLength -= tmp.length();
        data.erase(0, tmp.length());
        processData(tmp);
        request.__bodySize += tmp.length();
    }
    else
    {
        BasicString tmp = data;
        processData(tmp);
        request.__bodySize += data.length();
        request.__headers.__contentLength -= data.length();
        data.clear();
    }
    if (request.__headers.__contentLength == 0)
        __responsePhase = PREPARING_RESPONSE;
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::setWorkers(RessourceHandler &explorer, Location &location, Server &server)
{
    this->location = &location;
    this->explorer = &explorer;
    this->server = &server;
}
void Post::executePost(BasicString &data)
{
    try
    {
        if (request.__headers.__transferType == DEFINED)
            processDefinedBody(data);
        else if (request.__headers.__transferType == CHUNKED)
            processCunkedBody(data);
        else
            throw ErrorResponse(400, *location, "Missing Content-Length or Transfer-Encoding");
    }
    catch (ErrorResponse &e)
    {
        reset();
        throw e;
    }
}
