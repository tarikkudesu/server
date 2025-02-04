# webserv

### host:
    - context: server
    - syntax: host 0.0.0.0;
    - multiple directives: no
    - multiple values: no
    - default: 0.0.0.0
    - required: no

### listen:
    - context: server
    - syntax: listen 8080 8081 8081;
    - multiple directives: yes
    - multiple values: yes
    - default: 8080
    - required: no

### server_name:
    - context: server
    - syntax: server_name domain1 domain2 domain2;
    - multiple directives: yes
    - multiple values: yes
    - default: -
    - required: no

### client_body_buffer_size:
    - context: server
    - syntax: client_body_buffer_size size;
    - multiple directives: overrided
    - multiple values: no
    - default: 8k
    - required: no

### root:
    - context: location
    - syntax: root path
    - multiple directives: no
    - multiple values: no
    - default: /
    - required: no

### index:
    - context: location
    - syntax: index file
    - multiple directives: yes
    - multiple values: yes
    - default: index.html
    - required: no

### allow_methods:
    - context: location
    - syntax: allow_methods METHOD METHOD METHOD
    - multiple directives: yes
    - multiple values: yes
    - default: GET POST DELETE
    - required: no

### error_page:
    - context: location
    - syntax: error_page code code path
    - multiple directives: yes
    - multiple values: yes
    - default: -
    - required: no

### autoindex:
    - context: location
    - syntax: autoindex on | off
    - multiple directives: overrided
    - multiple values: no
    - default: off
    - required: no

### return :
    - context: location
    - syntax: return url
    - multiple directives: overrided
    - multiple values: no
    - default: -
    - required: no

### alias :
    - context: location
    - syntax: return url
    - multiple directives: no
    - multiple values: no
    - default: -
    - required: no

### cgi_extension :
    - context: location
    - syntax: cgi_extension path
    - multiple directives: overrided
    - multiple values: no
    - default: -
    - required: no

# Mandatory part

## Check the code and ask questions

- Launch the installation of siege with homebrew.
- Ask explanations about the basics of an HTTP server.
- Ask what function the group used for I/O Multiplexing.
- Ask for an explanation of how does select() (or equivalent) work.
- Ask if they use only one select() (or equivalent) and how they've managed the server to accept and the client to read/write.
- The select() (or equivalent) should be in the main loop and should check file descriptors for read and write AT THE SAME TIME. If not, the grade is 0 and the evaluation process ends now.
- There should be only one read or one write per client per select() (or equivalent). Ask the group to show you the code from the select() (or equivalent) to the read and write of a client.
- Search for all read/recv/write/send on a socket and check that, if an error is returned, the client is removed.
- Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or 0 values is not enough, both should be checked).
- If errno is checked after read/recv/write/send, the grade is 0 and the evaluation process ends now.
- Writing or reading ANY file descriptor without going through the select() (or equivalent) is strictly FORBIDDEN.
- The project must compile without any re-link issue. If not, use the 'Invalid compilation' flag.
- If any point is unclear or is not correct, the evaluation stops.

## Configuration

In the configuration file, check whether you can do the following and
test the result:

- Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes is wrong, don't give any related points.
- Setup multiple servers with different ports.
- Setup multiple servers with different hostnames (use something like: curl --resolve example.com:80:127.0.0.1 http://example.com/).
- Setup default error page (try to change the error 404).
- Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit").
- Setup routes in a server to different directories.
- Setup a default file to search for if you ask for a directory.
- Setup a list of methods accepted for a certain route (e.g., try to delete something with and without permission).

## Basic checks

Using telnet, curl, prepared files, demonstrate that the following
features work properly:

- GET, POST and DELETE requests should work.
- UNKNOWN requests should not result in a crash.
- For every test you should receive the appropriate status code.
- Upload some file to the server and get it back.

## Check CGI

Pay attention to the following:

- The server is working fine using a CGI.
- The CGI should be run in the correct directory for relative path file access.
- With the help of the students you should check that everything is working properly. You have to test the CGI with the "GET" and "POST" methods.
- You need to test with files containing errors to see if the error handling works properly. You can use a script containing an infinite loop or an error; you are free to do whatever tests you want within the limits of acceptability that remain at your discretion. The group being evaluated should help you with this.

The server should never crash and an error should be visible in case of a problem.

## Port issues

- In the configuration file setup multiple ports and use different websites. Use the browser to ensure that the configuration works as expected and shows the right website.
- In the configuration, try to setup the same port multiple times. It should not work.
- Launch multiple servers at the same time with different configurations but with common ports. Does it work? If it does, ask why the server should work if one of the configurations isn't functional. Keep going.

## Siege & stress test

- Use Siege to run some stress tests.
- Availability should be above 99.5% for a simple GET on an empty page with a siege -b on that page.
- Verify there is no memory leak (Monitor the process memory usage. It should not go up indefinitely).
- Check if there is no hanging connection.
- You should be able to use siege indefinitely without having to restart the server (take a look at siege -b).
- When conducting load tests using the siege command be carefull it depend of your OS), it is crucial to limit the number of connections per second by specifying options such as -c (number of clients), -d (maximum wait time before a client reconnects), and -r (number of attempts). The choice of these parameters is at the discretion of the evaluator, however, it is imperative to reach an agreement with the person being evaluated to ensure a fair and transparent assessment of the web server's performance.

# Bonus

Evaluate the bonus part if, and only if, the mandatory part has been entirely and perfectly done, and the error management handles unexpected or bad usage. In case all the mandatory points were not passed during the defense, bonus points must be totally ignored.

## Cookies and session

There is a working session and cookies system on the webserver.

## CGI

There is more than one CGI system.