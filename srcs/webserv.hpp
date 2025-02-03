#ifndef __WEBSERV_HPP__
#define __WEBSERV_HPP__

#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <ctime>
#include <poll.h>
#include <signal.h>
#include <netdb.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/types.h>
#include <cstdlib>

typedef std::string String;
typedef std::vector<String> t_svec;
typedef std::vector<pollfd> t_events;
typedef std::map<String, String>::iterator mapIterator;

#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define CYAN "\033[1;36m"
#define RESET "\033[1;0m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define MAGENTA "\033[1;35m"

#define JAVABIN "/usr/bin/java"
#define PYTHONBIN "/usr/bin/php"

#define CGI_TIMEOUT 15
#define CLIENT_TIMEOUT 15

#define MAX_EVENTS 1

#define READ_SIZE 8192
#define FORM_MAX_SIZE 1024
#define REQUEST_MAX_SIZE 4096
#define REQUEST_LINE_MAX_SIZE 4096
#define REQUEST_HEADERS_MAX_SIZE 8192
#define REQUEST_HEADER_FEILD_MAX_SIZE 4096

#define ERROR_PAGES_DIR_PATH "/essentials/pages"
#define SUCCESS_PAGE_PATH "essentials/success.html"
#define MIME_TYPES_DB_PATH "essentials/serversDB/mimetypes.csv"
#define ERROR_CODES_DB_PATH "essentials/serversDB/errorcodes.csv"
#define ERROR_PAGES_DB_PATH "essentials/serversDB/errorpages.csv"
#define DEFAULT_CONFIG_FILE_PATH "essentials/configuration_file.conf"

#define FORM_DATA "application/x-www-form-urlencoded"
#define MULTIPART_DATA_FORM "multipart/form-data"

#define LINE_BREAK "\r\n"
#define D_LINE_BREAK "\r\n\r\n"
#define PROTOCOLE_V "HTTP/1.1"
#define H_KEY_CHAR_SET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!#$%&\'*+-.^_`|~"
#define PRINTABLE " \t\n\r\v\f0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"
#define USAGE "\nUsage: ./webserv [OPTIONS] [configuration file]\n\nOptions:\n\t-l, --logs string\tlog events(\"debug\"|\"info\"|\"warn\"|\"error\"|\"fatal\"|\"all\")\n"

typedef enum e_endian
{
	SERVER,
	CONNECTION,
} t_endian;

typedef enum e_method
{
	OPTIONS,
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	TRACE,
	CONNECT,
} t_method;

typedef enum e_type
{
	FILE_,
	FOLDER,
} t_type;

typedef enum e_connectionType
{
	CLOSE,
	KEEP_ALIVE,
} t_connectionType;

typedef enum e_bodyType
{
	NONE,
	DEFINED,
	CHUNKED,
} t_bodyType;

typedef enum e_connection_phase {
    PROCESSING_REQUEST,
    IDENTIFY_WORKERS,
    PROCESSING_RESPONSE,
} t_connection_phase;

typedef enum e_request_phase {
    REQUEST_INIT,
    REQUEST_EXECUTE,
} t_request_phase;

typedef enum e_response_phase {
    PREPARING_RESPONSE,
    CGI_PROCESS,
    GET_PROCESS,
    POST_PROCESS,
    DELETE_PROCESS,
    RESPONSE_DONE,
} t_response_phase;

typedef enum e_get_file_operation {
    OPEN_FILE,
    READ_FILE,
    CLOSE_FILE,
} t_get_file_operation;

typedef enum e_get_phase {
    GET_INIT,
    GET_EXECUTE,
} t_get_phase;

typedef enum e_post_phase {
    POST_INIT,
    POST_EXECUTE,
} t_post_phase;

typedef enum e_content_type {
    FORM,
    MULTIPART,
    OTHER,
} t_content_type;

typedef enum e_multipartsection
{
    MP_INIT,
	MP_HEADERS,
	MP_BODY,
} t_multipartsection;

#endif
