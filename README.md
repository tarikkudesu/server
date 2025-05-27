## Overview

This is an HTTP based, single threaded, non-blocking, `nginx` like web server built according to the c++98 standards.

## Description

-   This project is implemented in c++98, uses STL library and is compilable with a Makefile.
-   The server reads the configuration, parses its content and initiates the provided servers.
-   The server uses sockets for non-blocking bidirectional communication between the client and the server.
-   The server never uses `poll` system call in a non-blocking way and fairly processes and responds to the incoming requests.
-   The server uses one `poll` call to process all I/O operations between the client and the server and checks for read and write operations at the same time.
-   Because it operates in a non-blocking way, requests never hang forever.
-   The server is based on HTTP therefore it manually reads and parses the Request line and headers, then builds the appropriate HTTP response.
-   The server supports files uploads, form submission and chunked requests.
-   The server is fully capable of serving fully static websites.
-   The server supports the `GET`, `POST` and `DELETE` HTTP methods.
-   The server has CGI support for `php` and `java`.
-   The server also supports session management and cookies.
-   The server has a log support for all events, classified as debug, info, warn, error and fatal logs,

## Usage

```bash
./webserv [OPTIONS] [configuration file]

Options:
        -l, --logs string       log events("debug"|"info"|"warn"|"error"|"fatal"|"all")
```

## Configuration file

The configuration file supports the following structure:

```
server {
	location {

	}
}
```

The configuration file supports the following directives:

```
host:
-	context: server,
-	syntax: host 0.0.0.0;,
-	multiple directives: no,
-	multiple values: no,
-	default: 0.0.0.0,
-	required: no,

listen:
-	context: server,
-	syntax: listen port port port;,
-	multiple directives: yes,
-	multiple values: yes,
-	default: 8080,
-	required: no,

server_name:
-	context: server,
-	syntax: server_name domain domain domain;,
-	multiple directives: yes,
-	multiple values: yes,
-	default: -,
-	required: no,

client_body_buffer_size:
-	context: server,
-	syntax: client_body_buffer_size size;,
-	multiple directives: no,
-	multiple values: no,
-	default: 8k,
-	required: no,

root:
-	context: location,
-	syntax: root path,
-	multiple directives: no,
-	multiple values: no,
-	default: /,
-	required: no,

index:
-	context: location,
-	syntax: index file,
-	multiple directives: yes,
-	multiple values: yes,
-	default: index.html,
-	required: no,

allow_methods:
-	context: location,
-	syntax: allow_methods METHOD METHOD METHOD,
-	multiple directives: yes,
-	multiple values: yes,
-	default: GET POST DELETE,
-	required: no,

error_page:
-	context: location,
-	syntax: error_page code code code path,
-	multiple directives: yes,
-	multiple values: yes,
-	default: -,
-	required: no,

autoindex:
-	context: location,
-	syntax: autoindex on | off,
-	multiple directives: no,
-	multiple values: no,
-	default: off,
-	required: no,

return:
-	context: location,
-	syntax: return url,
-	multiple directives: yes,
-	multiple values: no,
-	default: -,
-	required: no,

cgi_extension:
-	context: location,
-	syntax: cgi_extension path,
-	multiple directives: yes,
-	multiple values: no,
-	default: -,
-	required: no
```

### Example

This is an example of a functional configuration:

```
server {
	listen	9000;
	server_name webserv.com;
	root Content/showcase;
	index home.html;
	authenticate home.html form.html;

	location assets {
		autoindex on;
	}

	location /php {
		root	.;
		alias	cgi-bin/php;
		index	index.php;
		cgi_pass /usr/bin/php;
	}

	location /java {
		root	cgi-bin;
		index	Main.java;
		cgi_pass /usr/bin/java;
	}

	location /uploads {
		client_body_buffer_size 104857600;
		autoindex on;
	}
}
```

## Project Build Process

### Architecture Overview

The project is built using a modular class architecture that follows the **Single Responsibility Principle**. This design approach ensures that each component has a distinct purpose and can be developed and tested independently.

![Class Architecture](img/Classhierarchy.jpeg)

### Communication Layer

The server's communication layer is built on top of the **Socket API**, making proper management of open file descriptor connections essential for optimal performance and resource utilization.

![Socket Communication](img/PollFds.jpeg)

### Event Loop Architecture

The server implements a main event loop responsible for two core functions:

-   **Connection Management**: Handling incoming connections and maintaining active sessions

![Event Loop](img/EventLoop.jpeg)

-   **Request Processing**: Parsing and responding to HTTP requests

![Request Processing](img/RequestFlow.jpeg)

### HTTP Response Support

The server supports standard HTTP response codes and formats, ensuring compatibility with web standards and client expectations.

![HTTP Responses](img/ResponseProcessingFlow.jpeg)

### Phase-Based Processing

The server employs a **phase-based processing architecture** that provides several key benefits:

-   **Fairness**: Ensures equitable processing time allocation across multiple requests
-   **Organization**: Structures the request-response cycle into manageable phases
-   **State Management**: Maintains processing state to resume operations where they left off
-   **Scalability**: Allows for efficient handling of concurrent requests

![Processing Phases](img/DataProcessingFlow.jpeg)
