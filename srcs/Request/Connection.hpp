#ifndef __CONNECTION_HPP__
# define __CONNECTION_HPP__

# include "../Response/Response.hpp"

typedef std::map< int, Server* >			t_Server;
typedef std::vector< Server * >				t_serVect;

class Connection
{
	private :
		int								__sd;
		BasicString						__data;
		t_connection_phase				__phase;
		Request							__request;
		Response						__response;
		t_Server						*__serversP;

		Server							*identifyServer();
		void							processRequest();
		void							identifyWorkers();
		void							processResponse();
		void							processCunkedBody();
		void							initializeTmpFiles();
		void							processDefinedBody();
		void							indentifyRequestBody();
		void							processMultiPartBody();
		void							mpBody( t_multipartsection &part );
		void							mpHeaders( t_multipartsection &part );

		Connection();
		Connection( const Connection &copy );
		Connection	&operator=( const Connection &assign );

	public:
		std::queue< BasicString >		__responseQueue;

		void							addData(const BasicString &input);
		void							setServers( t_Server &servers );
		int								getConnectionSocket();
		void							setSocket( int sd );
		void							processData();
		bool							close();


		Connection( int sd );
		~Connection();
};

#endif
