#include "Request.hpp"

Request::Request(t_connection_phase &phase) : __phase(phase)
{
}
Request::Request(const Request &copy) : __phase(copy.__phase)
{
    *this = copy;
}
Request &Request::operator=(const Request &assign)
{
    if (this != &assign)
    {
        this->__URI = assign.__URI;
        this->__method = assign.__method;
        this->__headers = assign.__headers;
        this->__protocole = assign.__protocole;
        this->__queryString = assign.__queryString;
        this->__headerFeilds = assign.__headerFeilds;
    }
    return *this;
}
Request::~Request()
{
}

/****************************************************************************
 *                               MINI METHODS                               *
 ****************************************************************************/

void Request::clear()
{
    this->__URI.clear();
    this->__headers.clear();
    this->__fragement.clear();
    this->__protocole.clear();
    this->__headerFeilds.clear();
    this->__queryString.clear();
    this->__headerFeilds.clear();
}

/*****************************************************************************
 *                                  METHODS                                  *
 *****************************************************************************/

void Request::validateHeaders()
{
    size_t pos = 0;
    do
    {
        pos = __requestHeaders.find("\r\n");
        if (pos == String::npos)
            break;
        {
            String hf(__requestHeaders.begin(), __requestHeaders.begin() + pos);
            size_t p = hf.find(": ");
            if (p == String::npos)
                throw ErrorResponse(400, "invalid Header feild");
            String key(hf.begin(), hf.begin() + p);
            wsu::toUpperString(key);
            String value(hf.begin() + p + 2, hf.end());
            if (key.empty() || String::npos != key.find_first_not_of(H_KEY_CHAR_SET))
                throw ErrorResponse(400, "invalid Header feild");
            this->__headerFeilds[key] = value;
            __requestHeaders.erase(0, pos + 2);
        }
    } while (__requestHeaders.empty() == false);
}
void Request::validateURI()
{
    size_t start = 0;
    size_t end = 0;
    start = this->__URI.find("?");
    end = this->__URI.find("#");
    if (end != String::npos)
    {
        this->__fragement = String(this->__URI.begin() + end + 1, this->__URI.end());
        this->__URI.erase(end);
    }
    if (start != String::npos)
    {
        this->__queryString = String(this->__URI.begin() + start + 1, this->__URI.end());
        this->__URI.erase(start);
    }
}
void Request::validateRequestLine()
{
    if (2 < std::count(__requestLine.begin(), __requestLine.end(), ' '))
        throw ErrorResponse(400, "invalid Request Line (extra space)");
    std::istringstream iss(__requestLine);
    String method, URI, protocole;
    iss >> method;
    iss >> URI;
    iss >> protocole;
    if (method.empty() || protocole.empty() || URI.empty())
        throw ErrorResponse(400, "invalid Request Line (METHOD URI PROTOCOLE)");
    if (method == "OPTIONS")
        this->__method = OPTIONS;
    else if (method == "GET")
        this->__method = GET;
    else if (method == "HEAD")
        this->__method = HEAD;
    else if (method == "POST")
        this->__method = POST;
    else if (method == "PUT")
        this->__method = PUT;
    else if (method == "DELETE")
        this->__method = DELETE;
    else if (method == "TRACE")
        this->__method = TRACE;
    else if (method == "CONNECT")
        this->__method = CONNECT;
    else
        throw ErrorResponse(501, "invalid method");
    this->__URI = URI;
    if (String::npos != URI.find_first_not_of(URI_CHAR_SET))
        throw ErrorResponse(400, "invalid URI (out of URI CHARSET)");
    this->__protocole = protocole;
    if (this->__protocole != PROTOCOLE_V)
        throw ErrorResponse(505, "Unsupported protocole");
    validateURI();
}
void Request::parseRequest()
{
    validateRequestLine();
    validateHeaders();
    __headers.parseHeaders(__headerFeilds);
}
void Request::processData(BasicString &data)
{
    wsu::info("process request");
    size_t s = data.find(LINE_BREAK);
    size_t h = data.find(D_LINE_BREAK);
    if (s == String::npos || h == String::npos)
    {
        if (data.length() > REQUEST_MAX_SIZE)
        {
            data.clear();
            throw ErrorResponse(400, "Oversized request");
        }
        throw wsu::persist();
    }
    h -= (s + 2);
    __requestLine = data.substr(0, s).to_string();
    data.erase(0, s + 2);
    __requestHeaders = data.substr(0, h + 2).to_string();
    data.erase(0, h + 4);
    parseRequest();
    this->__phase = IDENTIFY_WORKERS;
}
std::ostream &operator<<(std::ostream &o, const Request &req)
{
    std::cout << "Request: \n";
    std::cout << "\tprotocole: " << req.__protocole << "\n";
    std::cout << "\tmethod: " << wsu::methodToString(req.__method) << "\n";
    std::cout << "\tURI: " << req.__URI << "\n";
    std::cout << "\tquery: " << req.__queryString << "\n";
    std::cout << "\n";
    std::cout << "\tFragement: " << req.__fragement << "\n";
    std::cout << "\theaders: \n";
    std::cout << req.__headers << "\n";
    return o;
}
