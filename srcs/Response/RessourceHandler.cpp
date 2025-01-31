#include "./RessourceHandler.hpp"

RessourceHandler::RessourceHandler()
{
}

RessourceHandler::RessourceHandler(const RessourceHandler &copy) : __type(copy.__type),
                                                                   __fullPath(copy.__fullPath),
                                                                   __location(copy.__location)
{
    *this = copy;
}
RessourceHandler &RessourceHandler::operator=(const RessourceHandler &assign)
{
    if (this != &assign)
    {
        __type = assign.__type;
        __location = assign.__location;
        __fullPath = assign.__fullPath;
    }
    return *this;
}
RessourceHandler::~RessourceHandler()
{
}
void RessourceHandler::loadType(const char *path)
{
    struct stat file_stat;
    if (stat(path, &file_stat) == -1)
        throw ErrorResponse(404, *__location, "the file does not exist on the server"); // to check what the exact status code and reason phrase
    else if (!(file_stat.st_mode & S_IRUSR))
        throw ErrorResponse(401, *__location, "don't have permission to read the file"); // to check what the exact status code and reason phrase
    if (S_ISDIR(file_stat.st_mode))
        __type = FOLDER;
    else
        __type = FILE_;
}
void	RessourceHandler::changeRequestedFile(String file)
{
	size_t pos = __fullPath.find_last_of("/");
	if (pos == String::npos)
		return ;
	__fullPath.erase(pos + 1);
	__fullPath = wsu::joinPaths(__fullPath, file);
}
void RessourceHandler::loadPathExploring(const String& uri)
{
    __fullPath = wsu::joinPaths(__location->__root, uri);
    loadType(__fullPath.c_str());
    if (__type == FOLDER)
    {
        t_svec::iterator it = __location->__index.begin();
        for (; it != __location->__index.end(); ++it)
        {
            String s = wsu::joinPaths(__fullPath, *it);
            if (!access(s.c_str(), F_OK))
            {
                __fullPath = s;
                __type = FILE_;
                return;
            }
        }
    }
}
void RessourceHandler::prepareRessource(Location& location, const String& uri)
{
    __fullPath.clear();
    __location = &location;
    if (wsu::__criticalOverLoad == true)
        throw ErrorResponse(503, *__location, "critical server overload");
    if (!__location->__return.empty())
        throw ErrorResponse(301, __location->__return, *__location);
    loadPathExploring(uri);
}

std::ostream &operator<<(std::ostream &o, RessourceHandler const &r)
{
    std::cout << "fullPath: " << r.__fullPath << "\n";
    std::cout << "type: ";
    if (r.__type == FOLDER)
        std::cout << "FOLDER\n";
    else
        std::cout << "FILE\n";
    std::cout << r.__location;
    return o;
}
