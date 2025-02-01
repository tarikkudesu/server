#include "Get.hpp"

Get::Get(Request &request, t_response_phase &phase) : request(request),
                                                      __responsePhase(phase),
                                                      explorer(NULL),
                                                      location(NULL),
                                                      server(NULL),
                                                      __phase(OPEN_FILE)
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
    __phase = OPEN_FILE;
    __responsePhase = RESPONSE_DONE;
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
void Get::getInPhase()
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
    __phase = READ_FILE;
}
void Get::duringGetPhase(BasicString &body)
{
    wsu::info("During GET phase");
    char buffer[100];
    wsu::ft_bzero(buffer, 100);
    __file.read(buffer, sizeof(buffer));
    String k(buffer, __file.gcount());
    if (__file.eof())
        __phase = CLOSE_FILE;
    if (__file.gcount() > 0)
        body = k;
}
void Get::setWorkers(FileExplorer &explorer, Location &location, Server &server)
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
        if (__phase == OPEN_FILE)
            getInPhase();
        if (__phase == READ_FILE)
            duringGetPhase(body);
        if (__phase == CLOSE_FILE)
            reset();
    }
    catch (ErrorResponse &e)
    {
        reset();
        throw e;
    }
}
