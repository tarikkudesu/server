/***************************************************************************
 *                             BODY PROCESSING                             *
 ***************************************************************************/

void Connection::processCunkedBody()
{
    static size_t chunkSize;
    do
    {
        if (chunkSize == 0)
        {
            size_t pos = __data.find("\r\n");
            if (pos == String::npos)
                throw wsu::persist();
            BasicString hex = __data.substr(0, pos);
            size_t extPos = hex.find(";");
            if (extPos != String::npos)
                hex = hex.substr(0, extPos);
            chunkSize = wsu::hexToInt(hex.to_string());
            __data.erase(0, pos + 2);
            this->__request.__bodySize += pos + 2;
            if (chunkSize == 0)
            {
                Connection::__fs.close();
                __request.__phase = COMPLETE;
                return;
            }
        }
        if (chunkSize < __data.length())
        {
            BasicString data = __data.substr(0, chunkSize);
            Connection::__fs << data;
            __data.erase(0, chunkSize);
            this->__request.__bodySize += chunkSize;
            chunkSize -= data.length();
        }
        else
        {
            Connection::__fs << __data;
            size_t  len = __data.length();
            chunkSize -= len;
            this->__request.__bodySize += len;
            __data.clear();
            break;
        }
    } while (true);
}
void Connection::processDefinedBody()
{
    if (__request.__headers.__contentLength < __data.length())
    {
        BasicString data = __data.substr(0, __request.__headers.__contentLength);
        __request.__headers.__contentLength -= data.length();
        __data.erase(0, data.length());
        Connection::__fs << data;
        this->__request.__bodySize += data.length();
    }
    else
    {
        Connection::__fs << __data;
        this->__request.__bodySize += __data.length();
        __request.__headers.__contentLength -= __data.length();
        __data.clear();
    }
    if (__request.__headers.__contentLength == 0)
    {
        __request.__phase = COMPLETE;
        Connection::__fs.close();
    }
}
void Connection::mpHeaders(t_multipartsection &part)
{
    size_t pos = __data.find("\r\n\r\n");
    if (pos == String::npos)
        throw wsu::persist();
    do
    {
        s_body body;
        body._fileName = wsu::generateTimeBasedFileName();
        Connection::__fs.open(body._fileName.c_str());
        if (!Connection::__fs.good())
            continue ;
        __request.__body.push_back(body);
        part = MP_BODY;
        break;
    } while (true);
    do
    {
        size_t p = __data.find("\r\n");
        if (p == 0 || p == String::npos)
            break;
        BasicString header = __data.substr(0, p);
        __request.__body.back()._headers.push_back(header.to_string());
        __data.erase(0, p + 2);
        this->__request.__bodySize += p + 2;
    } while (true);
    __data.erase(0, 2);
    this->__request.__bodySize += 2;
}
void Connection::mpBody(t_multipartsection &part)
{
    size_t end = __data.find("\r\n--" + __request.__headers.__boundary + "--\r\n");
    size_t pos = __data.find("\r\n--" + __request.__headers.__boundary + "\r\n");
    if (pos == String::npos && end == String::npos)
    {
        size_t len = __request.__headers.__boundary.length() + 8;
        if (__data.length() <= len)
            throw wsu::persist();
        BasicString data = __data.substr(0, __data.length() - len);
        Connection::__fs << data;
        __data.erase(0, data.length());
        this->__request.__bodySize += data.length();
    }
    else if (pos != String::npos)
    {
        size_t len = __request.__headers.__boundary.length() + 6;
        BasicString data = __data.substr(0, pos);
        Connection::__fs << data;
        Connection::__fs.close();
        __data.erase(0, pos + len);
        this->__request.__bodySize += pos + len;
        part = MP_HEADERS;
    }
    else if (end != String::npos)
    {
        size_t len = __request.__headers.__boundary.length() + 8;
        BasicString data = __data.substr(0, end);
        Connection::__fs << data;
        Connection::__fs.close();
        __data.erase(0, end + len);
        this->__request.__bodySize += end + len;
        __request.__phase = COMPLETE;
        part = MP_HEADERS;
    }
}
void Connection::processMultiPartBody()
{
    static t_multipartsection part = MP_HEADERS;
    do
    {
        if (part == MP_HEADERS)
            mpHeaders(part);
        if (part == MP_BODY)
            mpBody(part);
    } while(__request.__phase != COMPLETE);
}
void Connection::indentifyRequestBody()
{
    wsu::info("PROCESSING");
    if (__request.__headers.__transferType == DEFINED)
        processDefinedBody();
    else if (__request.__headers.__transferType == CHUNKED)
        processCunkedBody();
    else if (__request.__headers.__transferType == MULTIPART)
        processMultiPartBody();
}

// void Connection::initializeTmpFiles()
// {
//     wsu::info("INITIALIZING");
//     if (__request.__headers.__transferType == DEFINED || __request.__headers.__transferType == CHUNKED)
//     {
//         wsu::warn("defined || chunked");
//         do
//         {
//             s_body body;
//             body._fileName = wsu::generateTimeBasedFileName();
//             Connection::__fs.open(body._fileName.c_str());
//             if (!Connection::__fs.good())
//                 continue;
//             __request.__body.push_back(body);
//             break;
//         } while (true);
//     }
//     else if (__request.__headers.__transferType == MULTIPART)
//     {
//         size_t pos1 = __data.find("--" + __request.__headers.__boundary + "\r\n");
//         size_t pos2 = __data.find("--" + __request.__headers.__boundary + "--\r\n");
//         if (pos1 == String::npos && pos2 == String::npos)
//             throw wsu::persist();
//         else
//         {
//             if (pos2 == 0)
//             {
//                 __data.erase(0, __request.__headers.__boundary.length() + 6);
//                 throw ErrorResponse(400, "No files were uploaded");
//             }
//             if (pos1 == String::npos)
//                 throw wsu::persist();
//             else if (pos1 != 0)
//             {
//                 __data.erase(0, __request.__headers.__boundary.length() + 6);
//                 throw ErrorResponse(400, "Multipart/data-from: boundry mismatch");
//             }
//         }
//         __data.erase(0, __request.__headers.__boundary.length() + 4);
//     }
//     __request.__phase = PROCESSING;
// }
