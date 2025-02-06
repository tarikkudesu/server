
#include "Cgi.hpp"

typedef std::map<String, String> Map;

/*=---------------------constructors-----------------------*/

Cgi::Cgi(Request &request) : __request(request),
                             env(NULL)
{
    wsu::debug("Cgi param constructor");
}

Cgi::~Cgi()
{
    wsu::debug("Cgi destructor");
    reset();
}

Cgi &Cgi::operator=(const Cgi &cgi)
{
    if (this != &cgi)
    {
        this->__file = cgi.__file;
        this->__request = cgi.__request;
        this->__location = cgi.__location;
    }
    return *this;
}

Cgi::Cgi(const Cgi &cgi) : __request(cgi.__request)
{
    *this = cgi;
}

void Cgi::reset(void)
{
    if (env)
    {
        for (int i = 0; env[i]; i++)
            delete[] env[i];
        delete[] env;
        env = NULL;
    }
    this->__explorer = NULL;
    this->__location = NULL;
    if (!__file.empty())
    {
        unlink(__file.c_str());
        wsu::fatal("unlinked " + __file);
        __file.clear();
    }
    close(__fd);
}

/*-----------------------getters----------------------------*/

void Cgi::setWorkers(FileExplorer &explorer, Location &location)
{
    this->__location = &location;
    this->__explorer = &explorer;
}

String &Cgi::getFileName()
{
    return __file;
}
/*----------------------business logic------------------------*/

void Cgi::execute(const char *path)
{
    if (dup2(__fd, STDOUT_FILENO) < 0)
    {
        reset();
        exit(1);
    }
    int nullfd = open("/dev/null", O_WRONLY);
    dup2(nullfd, STDERR_FILENO);
    close(nullfd);
    const char *argv[] = {__location->__cgiPass.c_str(), path, NULL};
    execve(argv[0], (char *const *)argv, env);
    reset();
    exit(1);
}

String Cgi::getQueryString(BasicString &reqBody)
{
    String query = __request.__method == POST ? reqBody.to_string() : __request.__queryString;
    return wsu::decode(query);
}

void Cgi::setCgiEnvironement(BasicString &reqBody)
{
    Map headers = __request.__headerFeilds;
    headers["GATEWAY_INTERFACE"] = "CGI/1.1";
    headers["SERVER_NAME"] = "SERVER_NAME"; //tmp header value
    headers["SERVER_SOFTWARE"] = "WebServ-1337/1.0.0";
    headers["SERVER_PROTOCOL"] = "HTTP/1.1";
    headers["SERVER_PORT"] = "9001"; //tmp header value
    headers["QUERY_STRING"] = getQueryString(reqBody);
    headers["REQUEST_METHOD"] = wsu::methodToString(__request.__method);
    headers["SCRIPT_NAME"] = "index.php";
    headers["SCRIPT_FILENAME"] = "cgi-bin/php/index.php";
    headers["REDIRECT_STATUS"] = "200";
    headers["REMOTE_ADDR"] = "127.0.0.1"; //tmp header value
    headers["REMOTE_HOST"] = "127.0.0.1"; //tmp header value
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

void Cgi::processData(BasicString &reqBody)
{
    if (!__explorer || !__location)
        return wsu::fatal("no objects to be referenced");
    std::time_t start = std::time(NULL);
    __file = wsu::joinPaths(CGI_PATH, wsu::generateTimeBasedFileName());
    setCgiEnvironement(reqBody);
    int child, status, pid;

    __fd = open(__file.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
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
        kill(pid, SIGKILL), throw ErrorResponse(408, *__location, "Request Time-out");

    if (WIFEXITED(pid) && WEXITSTATUS(status))
        throw ErrorResponse(500, *__location, "wait error");
}
