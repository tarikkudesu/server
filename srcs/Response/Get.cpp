#include "Get.hpp"

Get::Get(t_response_phase& phase, Request& request)	:	__request(request),
														__responsePhase(phase),
														__phase(GET_IN)
{
}

Get::Get(const Get &copy) // a implementer
{
    *this = copy;
}

// a implementer
Get &Get::operator=(const Get &assign)
{
    if (this != &assign)
    {
    }
    return *this;
}

// a implementer
Get::~Get()
{
}

// void Get::readFile(void)
// {
//     std::ifstream file(explorer.getPath().c_str(), std::ios::binary);
//     if (!file.is_open())
//         throw ErrorResponse(500, explorer.__location, "Internal Server Error");
//     char buffer[4096];
//     while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
//         this->body.push_back(BasicString(buffer, file.gcount()));
//     file.close();
// }


bool	Get::authenticated()
{
	if (__request.__headers.__cookie.empty())
		return false;
	t_svec cookies = wsu::splitByChar(__request.__headers.__cookie, ';');
	for (t_svec::iterator it = cookies.begin(); it != cookies.end(); it++)
	{
		t_svec cook = wsu::splitByChar(*it, '=');
		if (cook.size() == 2 && __token.authentified(cook[1]))
			return true;
	}
	return false;
}

void Get::executeGet(RessourceHandler &explorer, Location &location, BasicString &body)
{
    if (__phase == GET_IN)
    {
        if (explorer.__type == FILE_)
        {
			if (explorer.__fullPath.compare(location.__authenticate[0]) && !authenticated())
				explorer.changeRequestedFile(location.__authenticate[1]);
			this->__file.open(explorer.__fullPath.c_str());
			if (!__file.is_open())
			{
				__responsePhase = PREPARING_RESPONSE;
				throw ErrorResponse(403, location, "Internal Server Error");
			}
			__phase = DURING_GET;
            // open File
            /*****************************************************************
             * IN CASE OF AN ERROR SET __RESPONSEPHASE = PREPARING_RESPONSE; *
             *****************************************************************/
            // std::ifstream file(explorer.getPath().c_str(), std::ios::binary);
            // if (!file.is_open())
            //     throw ErrorResponse(500, explorer.__location, "Internal Server Error");
        }
        else if (location.__autoindex)
        {
            __responsePhase = PREPARING_RESPONSE;
            t_svec directories;
            DIR *dir = opendir(explorer.__fullPath.c_str());
            if (!dir)
                throw ErrorResponse(500, *explorer.__location, "could not open directory");
            struct dirent *entry;
            while ((entry = readdir(dir)))
            {
                directories.push_back(entry->d_name);
                directories.push_back(" ");
            }
            closedir(dir);
            body = wsu::buildListingBody(explorer.__fullPath, directories);
        }
        else
            throw ErrorResponse(403, "Forbidden");
    }
    if (__phase == DURING_GET)
    {
		char buffer[READ_SIZE];
		__file.read(buffer, sizeof(buffer));
        if (__file.eof())
        {
            __phase = GET_OUT;
            __responsePhase = PREPARING_RESPONSE;
        }
		if (__file.gcount() > 0)
			throw BasicString(buffer, __file.gcount());
    }
    if (__phase == GET_OUT)
    {
        __file.close();
        __phase = GET_IN;
    }
}
