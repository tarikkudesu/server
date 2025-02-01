#include "Get.hpp"

Get::Get(Request &request, t_response_phase &phase) : request(request),
                                                      __responsePhase(phase),
                                                      explorer(NULL),
                                                      location(NULL),
                                                      server(NULL),
                                                      __phase(GET_IN)
{
}
Get::Get(const Get &copy) : request(copy.request),
                            __responsePhase(copy.__responsePhase),
                            explorer(copy.explorer),
                            location(copy.location),
                            server(copy.server),
                            __phase(copy.__phase)
{
    *this = copy;
}
Get &Get::operator=(const Get &assign)
{
    if (this != &assign)
    {
        this->request = assign.request;
        this->__responsePhase = assign.__responsePhase;
        this->explorer = assign.explorer;
        this->location = assign.location;
        this->server = assign.server;
        this->__phase = assign.__phase;
    }
    return *this;
}
Get::~Get()
{
    reset();
}

void Get::reset()
{
    wsu::info("GET out phase");
    if (__file.is_open())
        __file.close();
    __phase = GET_IN;
    __responsePhase = PREPARING_RESPONSE;
}
bool Get::authenticated()
{
    if (request.__headers.__cookie.empty())
        return false;
    t_svec cookies = wsu::splitByChar(request.__headers.__cookie, ';');
    for (t_svec::iterator it = cookies.begin(); it != cookies.end(); it++)
    {
        t_svec cook = wsu::splitByChar(*it, '=');
        if (cook.size() == 2 && server->authentified(cook[1]))
            return true;
    }
    return false;
}
void Get::getInPhase(BasicString &body)
{
    wsu::info("GET in phase");
    if (explorer->__type == FILE_)
    {
        if (location->__authenticate.size())
        {
            if (explorer->__fullPath.compare(location->__authenticate[0]) && !authenticated())
                explorer->changeRequestedFile(location->__authenticate[1]);
        }
        this->__file.open(explorer->__fullPath.c_str(), std::ios::binary);
        if (!__file.is_open())
            throw ErrorResponse(403, *location, "could not open file");
        wsu::info("opened file : " + explorer->__fullPath);
        __phase = DURING_GET;
    }
    else if (location->__autoindex)
    {
        __responsePhase = PREPARING_RESPONSE;
        t_svec directories;
        DIR *dir = opendir(explorer->__fullPath.c_str());
        if (!dir)
            throw ErrorResponse(500, *location, "could not open directory");
        struct dirent *entry;
        while ((entry = readdir(dir)))
        {
            directories.push_back(entry->d_name);
            directories.push_back(" ");
        }
        closedir(dir);
        body = wsu::buildListingBody(explorer->__fullPath, directories);
        __phase = GET_OUT;
    }
    else
        throw ErrorResponse(403, "Forbidden");
}
void Get::duringGetPhase(BasicString &body)
{
    wsu::info("During GET phase");
    char buffer[100];
    wsu::ft_bzero(buffer, 100);
    __file.read(buffer, sizeof(buffer));
    String k(buffer, __file.gcount());
    if (__file.eof())
        __phase = GET_OUT;
    if (__file.gcount() > 0)
        body = k;
}
void Get::setWorkers(RessourceHandler &explorer, Location &location, Server &server)
{
    this->location = &location;
    this->explorer = &explorer;
    this->server = &server;
}
void Get::executeGet(BasicString &body)
{
    if (!explorer || !location || !server)
    {
        wsu::fatal("no objects to be referenced");
        return;
    }
    try
    {
        if (__phase == GET_IN)
            getInPhase(body);
        if (__phase == DURING_GET)
            duringGetPhase(body);
        if (__phase == GET_OUT)
            reset();
    }
    catch (ErrorResponse &e)
    {
        reset();
        throw e;
    }
}
