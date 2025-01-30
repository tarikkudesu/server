#include "Get.hpp"

Get::Get(t_response_phase phase) : __responsePhase(phase),
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

void Get::executeGet(RessourceHandler &explorer, const Location &location, BasicString &body)
{
    if (__phase == GET_IN)
    {
        if (explorer.__type == FILE_)
        {
            // open File
            /*****************************************************************
             * IN CASE OF AN ERROR SET __RESPONSEPHASE = PREPARING_RESPONSE; *
             *****************************************************************/
            // std::ifstream file(explorer.getPath().c_str(), std::ios::binary);
            // if (!file.is_open())
            //     throw ErrorResponse(500, explorer.__location, "Internal Server Error");
            __phase = DURING_GET;
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
        // insert in body
        // char buffer[4096];
        // while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
        //     this->body.push_back(BasicString(buffer, file.gcount()));
        if (__file.eof())
        {
            __phase = GET_OUT;
            __responsePhase = PREPARING_RESPONSE;
        }
        // throw __body;
    }
    if (__phase == GET_OUT)
    {
        // close file
        // file.close();
        __phase = GET_IN;
    }
}
