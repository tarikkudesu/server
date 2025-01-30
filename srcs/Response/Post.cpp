#include "Post.hpp"

Post::Post(t_response_phase phase) : __responsePhase(phase), explorer(NULL), request(NULL), location(NULL)
{
}

Post::Post(const Post &copy) : __responsePhase(copy.__responsePhase), explorer(copy.explorer), request(copy.request)
{
    *this = copy;
}

Post &Post::operator=(const Post &assign)
{
    if (this != &assign)
    {
    }
    return *this;
}

Post::~Post()
{
}

/****************************************************************************
 *                               MINI METHODS                               *
 ****************************************************************************/

void Post::setWorkers(RessourceHandler &explorer, Location &location, Request &request)
{
    this->location = &location;
    this->explorer = &explorer;
    this->request = &request;
}
void writeDataIntoFile(BasicString &data)
{

}
void createFile(std::vector<BasicString> &h)
{

}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::processData(BasicString &data)
{
    if (request->__headers.__contentType == FORM_DATA)
        __responsePhase = CGI_PROCESS; // authenticate, otherwise do something
    else if (request->__headers.__contentType == MULTIPART_DATA_FORM) // no exactly, you need to get the boundry
        processMultiPartBody(data);
    else
        throw ErrorResponse(415, "Content-Type not supported");
}
void Post::executePost(BasicString &data)
{
    if (request->__headers.__transferType == DEFINED)
        processDefinedBody(data);
    else if (request->__headers.__transferType == CHUNKED)
        processCunkedBody(data);
    else
        throw ErrorResponse(400, "Missing Content-Length or Transfer-Encoding");
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::processCunkedBody(BasicString &data)
{
    static size_t chunkSize;
    do
    {
        if (chunkSize == 0)
        {
            size_t pos = data.find("\r\n");
            if (pos == String::npos)
            {
                if (data.length() > REQUEST_MAX_SIZE)
                    throw ErrorResponse(400, "Oversized chunk value");
                throw wsu::persist();
            }
            BasicString hex = data.substr(0, pos);
            size_t extPos = hex.find(";");
            if (extPos != String::npos)
                hex = hex.substr(0, extPos);
            chunkSize = wsu::hexToInt(hex.to_string());
            data.erase(0, pos + 2);
            request->__bodySize += pos + 2;
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
            request->__bodySize += chunkSize;
            chunkSize -= data.length();
        }
        else
        {
            BasicString tmp = data;
            processData(tmp);
            size_t len = data.length();
            chunkSize -= len;
            request->__bodySize += len;
            data.clear();
            break;
        }
    } while (true);
}
void Post::processDefinedBody(BasicString &data)
{
    if (request->__headers.__contentLength < data.length())
    {
        BasicString tmp = data.substr(0, request->__headers.__contentLength);
        request->__headers.__contentLength -= tmp.length();
        data.erase(0, tmp.length());
        processData(tmp);
        request->__bodySize += tmp.length();
    }
    else
    {
        BasicString tmp = data;
        processData(tmp);
        request->__bodySize += data.length();
        request->__headers.__contentLength -= data.length();
        data.clear();
    }
    if (request->__headers.__contentLength == 0)
    {
        __responsePhase = PREPARING_RESPONSE;
    }
}
/***********************************************************************************************
 *                                           METHODS                                           *
 ***********************************************************************************************/
void Post::mpBody(BasicString &data, t_multipartsection &part)
{
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
        request->__bodySize += data.length();
    }
    else if (pos != String::npos)
    {
        size_t len = this->__boundary.length() + 6;
        BasicString tmp = data.substr(0, pos);
        writeDataIntoFile(tmp);
        data.erase(0, pos + len);
        request->__bodySize += pos + len;
        part = MP_HEADERS;
    }
    else if (end != String::npos)
    {
        size_t len = this->__boundary.length() + 8;
        BasicString tmp = data.substr(0, end);
        writeDataIntoFile(tmp);
        data.erase(0, end + len);
        request->__bodySize += end + len;
        __responsePhase = PREPARING_RESPONSE;
        __boundary.clear();
        part = MP_HEADERS;
    }
}
void Post::mpHeaders(BasicString &data, t_multipartsection &part)
{
    size_t pos = data.find(D_LINE_BREAK);
    if (pos == String::npos)
    {
        if (data.length() > REQUEST_MAX_SIZE)
            throw ErrorResponse(400, "Oversized headers");
        throw wsu::persist();
    }
    std::vector<BasicString> headers;
    do
    {
        size_t p = data.find(LINE_BREAK);
        if (p == 0 || p == String::npos)
            break;
        headers.push_back(data.substr(0, p));
        request->__bodySize += p + 2;
        data.erase(0, p + 2);
    } while (true);
    data.erase(0, 2);
    createFile(headers);
    request->__bodySize += 2;
}
void Post::processMultiPartBody(BasicString &data)
{
    static t_multipartsection part = MP_HEADERS;
    do
    {
        if (part == MP_HEADERS)
            mpHeaders(data, part);
        if (part == MP_BODY)
            mpBody(data, part);
    } while (__responsePhase != PREPARING_RESPONSE);
}
