
#include "Cgi.hpp"

typedef std::map<String, String> Map;

/*=---------------------constructors-----------------------*/

Cgi::Cgi(FileExplorer &explorer, Request &request, Location &location, BasicString &body) : __request(request),
                                                                                            __explorer(explorer),
                                                                                            __location(location),
                                                                                            __reqBody(body),
                                                                                            __start(std::time(NULL)),
                                                                                            __body("")
{
	wsu::debug("Cgi default constructor");
    cgiProcess();
}

Cgi::~Cgi()
{
	wsu::debug("Cgi destructor");
    clear();
}

/*----------------------business logic------------------------*/

void Cgi::clear(void)
{
    for (int i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;
}

void Cgi::execute(const char *path, int fd)
{
    if (dup2(fd, STDOUT_FILENO) < 0)
    {
        perror("dup2");
        clear();
        close(fd);
        exit(1);
    }
    const char *argv[] = {__location.__cgiPass.c_str(), path, NULL};
    execve(argv[0], (char *const *)argv, env);
    clear();
    close(fd);
    exit(1);
}

void Cgi::readFromFile(String &file)
{
	String buffer;
	std::ifstream content(file.c_str());
	if (!content.is_open())
        throw ErrorResponse(500, __location, "open fail");
	while (std::getline(content, buffer))
		__body.append(buffer);
	content.close();
}

String Cgi::getQueryString()
{
    return __request.__method == POST ? __reqBody.to_string() : __request.__queryString;
}

void Cgi::setCgiEnvironement()
{
    Map headers = __request.__headerFeilds;
    headers["GATEWAY_INTERFACE"] = "CGI/1.1";
    headers["SERVER_NAME"] = "SERVER_NAME"; //tmp header value
    headers["SERVER_SOFTWARE"] = "WebServ-1337/1.0.0";
    headers["SERVER_PROTOCOL"] = "HTTP/1.1";
    headers["SERVER_PORT"] = "9001"; //tmp header value
    headers["QUERY_STRING"] = getQueryString();
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

void Cgi::cgiProcess(void)
{
    setCgiEnvironement();
	String file = wsu::generateTokenId() + ".html";

    int child, status, pid;
	
	int fd = open(file.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0)
        throw ErrorResponse(500, __location, "open fail");

 	pid = fork();
    if (pid < 0)
        throw ErrorResponse(500, __location, "fork error");

    if (!pid)
        execute(__explorer.__fullPath.c_str(), fd);

    while (!(child = waitpid(pid, &status, WNOHANG)) && (std::time(NULL) - __start) < CGI_TIMEOUT)
        ;
    if (!child)
        kill(pid, SIGKILL), throw ErrorResponse(408, __location, "Request Time-out");
    if (WIFEXITED(pid) && WEXITSTATUS(status))
        throw ErrorResponse(500, __location, "wait error");
    close(fd);
    readFromFile(file);
	unlink(file.c_str());
}

/*-----------------------getters----------------------------*/

String &Cgi::getBody()
{
    return __body;
}
