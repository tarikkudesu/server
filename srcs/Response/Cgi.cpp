
#include "Cgi.hpp"

typedef std::map<String, String> Map;

/*=---------------------constructors-----------------------*/

Cgi::Cgi(FileExplorer &explorer, Request &request, Location &location, BasicString &body) : __explorer(explorer),
                                                                                            __location(location),
																							__request(request),
                                                                                            __reqBody(body),
                                                                                            __start(std::time(NULL)),
                                                                                            __body("")
{
	wsu::debug("Cgi param constructor");
	__file = wsu::generateTimeBasedFileName();
	std::cout << __file << "\n";
	__stream.open(__file.c_str());
	if (!__stream.is_open())
		throw ErrorResponse(500, __location, "can't open cgi stream");
    cgiProcess();
}

Cgi::~Cgi()
{
	wsu::debug("Cgi destructor");
	__stream.close();
	unlink(__file.c_str());
    clear();
}

Cgi& Cgi::operator=(const Cgi& cgi)
{
	if (this != &cgi)
	{
		this->__body = cgi.__body;
		this->__file = cgi.__file;
		this->__start = cgi.__start;
		this->__request = cgi.__request;
		this->__location = cgi.__location;
	}	
	return *this;
}

Cgi::Cgi(const Cgi& cgi) :	__explorer(cgi.__explorer),
							__location(cgi.__location),
							__request(cgi.__request)
{
	*this = cgi;
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



void Cgi::readFromFile()
{
	String buffer;
	std::ifstream file(__file);
	if (!file.is_open())
        throw ErrorResponse(500, __location, "open fail");
	while (std::getline(file, buffer))
		__body.append(buffer);
}

String Cgi::getQueryString()
{
    String query = __request.__method == POST ? __reqBody.to_string() : __request.__queryString;
	return wsu::decode(query);

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
    int child, status, pid;
	
	int fd = open(__file.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
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
    readFromFile();
}

/*-----------------------getters----------------------------*/

String &Cgi::getBody()
{
    return __body;
}
