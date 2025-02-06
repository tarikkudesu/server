
#include "Cgi.hpp"

/*=---------------------constructors-----------------------*/

Cgi::Cgi(Request &request) : __request(request),
                             __explorer(NULL),
                             __location(NULL),
                             env(NULL)
{
    wsu::debug("Cgi param constructor");
}

Cgi &Cgi::operator=(const Cgi &cgi)
{
    if (this != &cgi)
    {
        this->__request = cgi.__request;
        this->__location = cgi.__location;
    }
    return *this;
}

Cgi::Cgi(const Cgi &cgi) : __request(cgi.__request)
{
    *this = cgi;
}

Cgi::~Cgi()
{
    wsu::debug("Cgi destructor");
    if (env)
    {
        for (int i = 0; env[i]; i++)
            delete[] env[i];
        delete[] env;
        env = NULL;
    }
    this->__explorer = NULL;
    this->__location = NULL;
    close(__fd);
}

/*-----------------------getters----------------------------*/

void Cgi::setWorkers(FileExplorer &explorer, Location &location)
{
    this->__location = &location;
    this->__explorer = &explorer;
}
/*----------------------business logic------------------------*/

void Cgi::execute(const char *path)
{
    if (dup2(__fd, STDOUT_FILENO) < 0)
        exit(1);
    int nullfd = open("/dev/null", O_WRONLY);
    dup2(nullfd, STDERR_FILENO);
    close(nullfd);
    const char *argv[] = {__location->__cgiPass.c_str(), path, NULL};
    execve(argv[0], (char *const *)argv, env);
    exit(1);
}

String Cgi::getQueryString(BasicString &reqBody)
{
    String query = __request.__method == POST ? reqBody.to_string() : __request.__queryString;
	size_t pos = query.find('/');
	if (pos != String::npos)
		query.erase(0, pos);
    return wsu::decode(query);
}

String Cgi::getPathInfo(BasicString &reqBody)
{
    String query = __request.__method == POST ? reqBody.to_string() : __request.__queryString;
	size_t pos = query.find('/');
	if (pos == String::npos)
		return "";
	query = query.substr(pos);
    return wsu::decode(query);
}

void Cgi::setCgiEnvironement(BasicString &reqBody)
{
	Map headers = __request.__headerFeilds;
    headers["GATEWAY_INTERFACE"] = "CGI/1.1";
    headers["SERVER_NAME"] = __request.__headers.__host ; //tmp header value
    headers["SERVER_SOFTWARE"] = "webserv/1.0";
    headers["SERVER_PROTOCOL"] = "HTTP/1.1";
    headers["SERVER_PORT"] = wsu::intToString(__request.__headers.__port);
    headers["QUERY_STRING"] = getQueryString(reqBody);
    headers["PATH_INFO"] = getPathInfo(reqBody);
    headers["REQUEST_METHOD"] = wsu::methodToString(__request.__method);
    headers["SCRIPT_FILENAME"] = __explorer->__fullPath;
    headers["REDIRECT_STATUS"] = "200";
    env = new char *[headers.size() + 1];
    int i = 0;
    for (Map::iterator it = headers.begin(); it != headers.end(); it++, i++)
    {
        String header = it->first + "=" + it->second;
        env[i] = new char[header.size() + 1];
        std::strcpy(env[i], header.c_str());
    }
    env[i] = NULL;
}

void Cgi::processData(BasicString &reqBody, String file)
{
    if (!__explorer || !__location)
        return wsu::error("no objects to be referenced");
    std::time_t start = std::time(NULL);
    setCgiEnvironement(reqBody);
    int child, status, pid;

    __fd = open(file.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (__fd < 0)
        throw ErrorResponse(500, *__location, "open fail");

    pid = fork();
    if (pid < 0)
        throw ErrorResponse(500, *__location, "fork error");

    if (!pid)
        execute(__explorer->__fullPath.c_str());

    while (!(child = waitpid(pid, &status, WNOHANG)) && (std::time(NULL) - start) < CGI_TIMEOUT)
        ;

    if (!child)
        kill(pid, SIGKILL), throw ErrorResponse(504, *__location, "CGI Time-out");

    if (WIFEXITED(pid) && WEXITSTATUS(status))
        throw ErrorResponse(500, *__location, "wait error");
}
