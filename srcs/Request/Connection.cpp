#include "Connection.hpp"

Connection::Connection() : __sd(-1),
                           __request(__phase),
                           __response(__phase),
                           __serversP(NULL),
                           __phase(PROCESSING_REQUEST)
{
    wsu::debug("Connection default constructor");
}
Connection::Connection(int sd) : __sd(sd),
                                 __request(__phase),
                                 __response(__phase),
                                 __serversP(NULL)
{
    wsu::debug("Connection single para constructor");
}
Connection::Connection(const Connection &copy) : __request(copy.__request),
                                                 __response(copy.__response)
{
    wsu::debug("Connection copy constructor");
    *this = copy;
}
Connection &Connection::operator=(const Connection &assign)
{
    wsu::debug("Connection copy assignement operator");
    if (this != &assign)
    {
        __sd = assign.__sd;
        __phase = assign.__phase;
        __serversP = assign.__serversP;
        __responseQueue = assign.__responseQueue;
    }
    return *this;
}
Connection::~Connection()
{
    wsu::debug("Connection destructor");
}

void Connection::setServers(t_Server &servers)
{
    this->__serversP = &servers;
}
int Connection::getSock()
{
    return __sd;
}
/*****************************************************************************
 *                                  METHODS                                  *
 *****************************************************************************/
Server *Connection::identifyServer()
{
    t_serVect tmpMapP, tmpMapH;
    for (t_Server::iterator it = this->__serversP->begin(); it != this->__serversP->end(); it++)
    {
        if (it->second->getServerPort() == this->__request.__headers.__port)
            tmpMapP.push_back(it->second);
    }
    for (t_serVect::iterator it = tmpMapP.begin(); it != tmpMapP.end(); it++)
    {
        if ((*it)->amITheServerYouAreLookingFor(this->__request.__headers.__host) == false)
            tmpMapH.push_back(*it);
    }
    if (tmpMapH.empty())
        return this->__serversP->begin()->second;
    return tmpMapH.at(0);
}
void Connection::identifyWorkers()
{
    Server *server = identifyServer();
    Location &location = server->identifyLocation(__request.__URI);
    __response.setupWorkers(__request, *server, location);
    this->__phase = PROCESSING_RESPONSE;
}
/**********************************************************************************
 *                                  PROCESS DATA                                  *
 **********************************************************************************/
void Connection::proccessData(BasicString input)
{
    this->__data.join(input);
    try
    {
        wsu::info("proccessing request");
        if (__phase == PROCESSING_REQUEST)
            __request.processData(__data);
        if (__phase == IDENTIFY_WORKERS)
            identifyWorkers();
        if (__phase == PROCESSING_RESPONSE)
            __response.processData(__data);
    }
    catch (ErrorResponse &e)
    {
        this->__responseQueue.push(e.getResponse());
        __phase = PROCESSING_REQUEST;
    }
    catch (BasicString &e)
    {
        this->__responseQueue.push(e);
    }
    catch (wsu::persist &e)
    {
        wsu::error("presist");
    }
    catch (std::exception &e)
    {
    }
}
