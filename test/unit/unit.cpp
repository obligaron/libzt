// Comprehensive stress test for socket-like API

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fcntl.h>

#include <iostream>

#include "ZeroTierSDK.h"

#define PASSED         0
#define FAILED        -1

#define ECHO_INTERVAL  100000 // us
#define STR_SIZE       32

#define TEST_OP_N_BYTES    10
#define TEST_OP_N_SECONDS  11
#define TEST_OP_N_TIMES    12

#define TEST_MODE_CLIENT     20
#define TEST_MODE_SERVER     21

#define TEST_TYPE_SIMPLE     30
#define TEST_TYPE_SUSTAINED  31

char str[STR_SIZE];

// [] random
// [OK] simple client ipv4
// [OK] simple server ipv4
// [?] simple client ipv6
// [?] simple server ipv6
// [OK] sustained client ipv4
// [OK] sustained server ipv4
// [?] sustained client ipv6
// [?] sustained server ipv6
// [] comprehensive client ipv4
// [] comprehensive server ipv6

/* Performance Tests
 
Throughput
Memory Usage
Processor usage

socket API semantics 
 - Proper socket closure
 - Proper handling of blocking/non-blocking behaviour
 - replicate specific errno conditions and verify correctness
  
Network semantics
  - Multi-network handling
  - Address handling

ZeroTier-specific functionality

*/

/****************************************************************************/
/* SIMPLE CLIENT                                                            */
/****************************************************************************/

// 
int ipv4_tcp_client_test(struct sockaddr_in *addr, int port)
{
	int r, w, sockfd, err, len = strlen(str);
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_connect(sockfd, (const struct sockaddr *)addr, sizeof(addr))) < 0) {
		printf("error connecting to remote host (%d)\n", err);
	}
	w = zts_write(sockfd, str, len);
	r = zts_read(sockfd, rbuf, len);
	err = zts_close(sockfd);
	return (w == len && r == len && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
}

// 
int ipv6_tcp_client_test(struct sockaddr_in6 *addr, int port)
{
	int r, w, sockfd, err, len = strlen(str);
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_connect(sockfd, (const struct sockaddr *)addr, sizeof(addr))) < 0) {
		printf("error connecting to remote host (%d)\n", err);
	}
	w = zts_write(sockfd, str, len);
	r = zts_read(sockfd, rbuf, len);
	err = zts_close(sockfd);
	return (w == len && r == len && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
}




/****************************************************************************/
/* SIMPLE SERVER                                                            */
/****************************************************************************/

//
int ipv4_tcp_server_test(struct sockaddr_in *addr, int port)
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_bind(sockfd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0)) {
		printf("error binding to interface (%d)\n", err);
	}
	if((err = zts_listen(sockfd, 100)) < 0) {
		printf("error placing socket in LISTENING state (%d)\n", err);
	}
	// TODO: handle new address
	if((accfd = zts_accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr))) < 0) {
		printf("error accepting connection (%d)\n", err);
	}
	r = zts_read(accfd, rbuf, sizeof rbuf);
	w = zts_write(accfd, rbuf, len);
	zts_close(sockfd);
	zts_close(accfd);
	return (w == len && r == len && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
}

//
int ipv6_tcp_server_test(struct sockaddr_in6 *addr, int port)
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_bind(sockfd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0)) {
		printf("error binding to interface (%d)\n", err);
	}
	if((err = zts_listen(sockfd, 100)) < 0) {
		printf("error placing socket in LISTENING state (%d)\n", err);
	}
	// TODO: handle new address
	if((accfd = zts_accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr))) < 0) {
		printf("error accepting connection (%d)\n", err);
	}
	r = zts_read(accfd, rbuf, sizeof rbuf);
	w = zts_write(accfd, rbuf, len);
	zts_close(sockfd);
	zts_close(accfd);
	return (w == len && r == len && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
}





/****************************************************************************/
/* SUSTAINED CLIENT                                                         */
/****************************************************************************/

// Maintain transfer for n_count OR n_count
int ipv4_tcp_client_sustained_test(struct sockaddr_in *addr, int port, int operation, int n_count, int delay)
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	int tot, n=0;
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_connect(sockfd, (const struct sockaddr *)addr, sizeof(addr))) < 0) {
		printf("error connecting to remote host (%d)\n", err);
	}
	//zts_fcntl(sockfd, F_SETFL, O_NONBLOCK);
	if(operation == TEST_OP_N_TIMES) {
		tot = len*n_count;
		for(int i=0; i<n_count; i++) {
			usleep(delay * 1000);
			n = zts_write(sockfd, str, len);
			if (n > 0)
				w += n;
			n = zts_read(sockfd, rbuf, len);
			if (n > 0)
				r += n;
		}
		err = zts_close(sockfd);
		return (r == tot && w == tot && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
	}
	if(operation == TEST_OP_N_BYTES) {
		tot = n_count;
		while(r < tot || w < tot) {
			usleep(delay * 1000);
			if (w < tot)
				n = zts_write(sockfd, str, n_count);
			if (n > 0)
				w += n;
			if (r < tot)
				n = zts_read(sockfd, rbuf, n_count);
			if (n > 0)
				r += n;
		}
		err = zts_close(sockfd);
		return (r == tot && w == tot && !err) ? PASSED : FAILED;
	}
	return FAILED;
}

// Maintain transfer for n_count OR n_count
int ipv6_tcp_client_sustained_test(struct sockaddr_in6 *addr, int port, int operation, int n_count, int delay) 
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	int tot, n=0;
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_connect(sockfd, (const struct sockaddr *)addr, sizeof(addr))) < 0) {
		printf("error connecting to remote host (%d)\n", err);
	}
	//zts_fcntl(sockfd, F_SETFL, O_NONBLOCK);
	if(operation == TEST_OP_N_TIMES) {
		tot = len*n_count;
		for(int i=0; i<n_count; i++) {
			usleep(delay * 1000);
			n = zts_write(sockfd, str, len);
			if (n > 0)
				w += n;
			n = zts_read(sockfd, rbuf, len);
			if (n > 0)
				r += n;
		}
		err = zts_close(sockfd);
		return (r == tot && w == tot && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
	}
	if(operation == TEST_OP_N_BYTES) {
		tot = n_count;
		while(r < tot || w < tot) {
			usleep(delay * 1000);
			if (w < tot)
				n = zts_write(sockfd, str, n_count);
			if (n > 0)
				w += n;
			if (r < tot)
				n = zts_read(sockfd, rbuf, n_count);
			if (n > 0)
				r += n;
		}
		err = zts_close(sockfd);
		return (r == tot && w == tot && !err) ? PASSED : FAILED;
	}
	return FAILED;
}





/****************************************************************************/
/* SUSTAINED SERVER                                                         */
/****************************************************************************/

// Maintain transfer for n_count OR n_count
int ipv4_tcp_server_sustained_test(struct sockaddr_in *addr, int port, int operation, int n_count, int delay)
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	int tot, n=0;
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_bind(sockfd, (struct sockaddr *)addr, (socklen_t)sizeof(struct sockaddr_in)) < 0)) {
		printf("error binding to interface (%d)\n", err);
	}
	if((err = zts_listen(sockfd, 1)) < 0) {
		printf("error placing socket in LISTENING state (%d)\n", err);
	}
	// TODO: handle new address
	if((accfd = zts_accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr))) < 0) {
		printf("error accepting connection (%d)\n", err);
	}
	//zts_fcntl(accfd, F_SETFL, O_NONBLOCK);
	if(operation == TEST_OP_N_TIMES) {
		tot = len*n_count;
		for(int i=0; i<n_count; i++) {
			usleep(delay * 1000);
			r += zts_read(accfd, rbuf, len);
			w += zts_write(accfd, rbuf, len);		
		}
		zts_close(sockfd);
		zts_close(accfd);
		return (r == tot && w == tot && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
	}
	if(operation == TEST_OP_N_BYTES) {
		tot = n_count;
		while(r < tot || w < tot) {
			usleep(delay * 1000);
			if (r < tot)
				n = zts_read(accfd, rbuf, n_count);
			if (n > 0)
				r += n;
			if (w < tot)
				n = zts_write(accfd, str, n_count);
			if (n > 0)
				w += n;
		}
		zts_close(sockfd);
		zts_close(accfd);
		return (r == tot && w == tot && !err) ? PASSED : FAILED;
	}
	return FAILED;
}

// Maintain transfer for n_count OR n_count
int ipv6_tcp_server_sustained_test(struct sockaddr_in6 *addr, int port, int operation, int n_count, int delay)
{
	int w=0, r=0, sockfd, accfd, err, len = strlen(str);
	int tot, n=0;
	char rbuf[STR_SIZE];
	if((sockfd = zts_socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
		printf("error creating ZeroTier socket");
	}
	if((err = zts_bind(sockfd, (struct sockaddr *)addr, (socklen_t)sizeof(struct sockaddr_in)) < 0)) {
		printf("error binding to interface (%d)\n", err);
	}
	if((err = zts_listen(sockfd, 1)) < 0) {
		printf("error placing socket in LISTENING state (%d)\n", err);
	}
	// TODO: handle new address
	if((accfd = zts_accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr))) < 0) {
		printf("error accepting connection (%d)\n", err);
	}
	//zts_fcntl(accfd, F_SETFL, O_NONBLOCK);
	if(operation == TEST_OP_N_TIMES) {
		tot = len*n_count;
		for(int i=0; i<n_count; i++) {
			usleep(delay * 1000);
			r += zts_read(accfd, rbuf, len);
			w += zts_write(accfd, rbuf, len);		
		}
		zts_close(sockfd);
		zts_close(accfd);
		return (r == tot && w == tot && !err) && !strcmp(rbuf, str) ? PASSED : FAILED;
	}
	if(operation == TEST_OP_N_BYTES) {
		tot = n_count;
		while(r < tot || w < tot) {
			usleep(delay * 1000);
			if (r < tot)
				n = zts_read(accfd, rbuf, n_count);
			if (n > 0)
				r += n;
			if (w < tot)
				n = zts_write(accfd, str, n_count);
			if (n > 0)
				w += n;
		}
		zts_close(sockfd);
		zts_close(accfd);
		return (r == tot && w == tot && !err) ? PASSED : FAILED;
	}
	return FAILED;}





/****************************************************************************/
/* RANDOMIZED API TEST                                                      */
/****************************************************************************/

int random_api_test()
{
	// PASSED implies we didn't segfault or hang anywhere

	//
	int calls_made = 0;

	// how many calls we'll make
    int num_of_api_calls = 10;

/*
zts_socket()
zts_connect()
zts_listen()
zts_accept()
zts_bind()
zts_getsockopt()
zts_setsockopt()
zts_fnctl()
zts_close()
*/

	// variables which will be populated with random values
	int fd, arg_val;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;

	while(calls_made < num_of_api_calls)
	{
		fprintf(stderr, "calls_made = %d\n", calls_made);
		int random_call = 0;

/*
		switch(random_call)
		{
			default:
				printf()
		}
*/



		calls_made++;
	}
	return PASSED;
}


/****************************************************************************/
/* test driver, called from main()                                          */
/****************************************************************************/

/*
*
* path      = place where ZT keys, and config files will be stored 
* nwid      = network for app to join
* type      = simple, sustained
* protocol  = 4, 6
* mode      = client, server
* addr      = ip address string
* port      = integer
* operation = n_times, n_seconds, n_bytes, etc
* n_count   = number of operations of type
* delay     = delay between each operation
*
*/
int do_test(std::string path, std::string nwid, int type, int protocol, int mode, std::string ipstr, int port, int operation, int n_count, int delay)
{
	struct hostent *server;
    struct sockaddr_in6 addr6;
	struct sockaddr_in addr;

	printf("\npath      = %s\n", path.c_str());
	printf("nwid      = %s\n", nwid.c_str());
	printf("type      = %d\n", type);
	printf("protocol  = %d\n", protocol);
	printf("mode      = %d\n", mode);
	printf("ipstr     = %s\n", ipstr.c_str());
	printf("port      = %d\n", port);
	printf("operation = %d\n", operation);
	printf("n_count   = %d\n", n_count);
	printf("delay     = %d\n\n", delay);

	/****************************************************************************/
	/* SIMPLE                                                                   */
	/****************************************************************************/

	// SIMPLE
	// performs a one-off test of a particular subset of the API
	// For instance (ipv4 client, ipv6 server, etc)
	if(type == TEST_TYPE_SIMPLE) {		
		if(mode == TEST_MODE_CLIENT) {

			std::cout << "connecting to " << ipstr << " on port " << port << std::endl;
			// IPv4
			if(protocol == 4) {
				addr.sin_addr.s_addr = inet_addr(ipstr.c_str());
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv4_tcp_client_test(&addr, port);
			}
			// IPv6
			if(protocol == 6) {
				server = gethostbyname2(ipstr.c_str(),AF_INET6);
    			memset((char *) &addr6, 0, sizeof(addr6));
    			addr6.sin6_flowinfo = 0;
    			addr6.sin6_family = AF_INET6;
    			memmove((char *) &addr6.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
    			addr6.sin6_port = htons(port);
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv6_tcp_client_test(&addr6, port);
			}
		}

		if(mode == TEST_MODE_SERVER) {

			//printf("serving on port %s\n", port);
			// IPv4
			if(protocol == 4) {
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = inet_addr(ipstr.c_str());
				// addr.sin_addr.s_addr = htons(INADDR_ANY);
				addr.sin_family = AF_INET;
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv4_tcp_server_test(&addr, port);
			}
			// IPv6
			if(protocol == 6) {

    struct hostent *server;
	server = gethostbyname2("fde5:cd7a:9e1c:0fd2:7299:9369:4d7b:feff",AF_INET6);
	if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }

	memset((char *) &addr6, 0, sizeof(addr6));
    addr6.sin6_flowinfo = 0;
    addr6.sin6_family = AF_INET6;
    memmove((char *) &addr6.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
    addr6.sin6_port = htons(port);

    /*
				server = gethostbyname2(ipstr.c_str(),AF_INET6);
    			memset((char *) &addr6, 0, sizeof(addr6));
    			addr6.sin6_flowinfo = 0;
    			addr6.sin6_family = AF_INET6;
    			addr6.sin6_port = htons(port);
				//addr6.sin6_addr = in6addr_any;
    			memmove((char *) &addr6.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				printf("ipstr = %s\n", ipstr.c_str());

	*/
				return ipv6_tcp_server_test(&addr6, port);
			}
		}
	}

	/****************************************************************************/
	/* SUSTAINED                                                                */
	/****************************************************************************/

	// ./unit zt2 c7cd7c9e1b0f52a2 simple 4 client 10.9.9.40 8787 n_seconds 10 50
	// ./unit zt2 c7cd7c9e1b0f52a2 simple 4 client 10.9.9.40 8787 n_bytes 100 50
	// ./unit zt2 c7cd7c9e1b0f52a2 simple 4 client 10.9.9.40 8787 n_times 100 50

	// SUSTAINED
	// Performs a stress test for benchmarking performance
	if(type == TEST_TYPE_SUSTAINED) {
		if(mode == TEST_MODE_CLIENT) {

			//printf("connecting to %s on port %d\n", ipstr, port);
			// IPv4
			if(protocol == 4) {
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = inet_addr(ipstr.c_str());
				addr.sin_family = AF_INET;
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv4_tcp_client_sustained_test(&addr, port, operation, n_count, delay);
			}
			// IPv6
			if(protocol == 6) {
				server = gethostbyname2(ipstr.c_str(),AF_INET6);
    			memset((char *) &addr6, 0, sizeof(addr6));
    			addr6.sin6_flowinfo = 0;
    			addr6.sin6_family = AF_INET6;
    			memmove((char *) &addr6.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
    			addr6.sin6_port = htons(port);
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv6_tcp_client_sustained_test(&addr6, port, operation, n_count, delay);
			}
		}

		if(mode == TEST_MODE_SERVER)
		{
			//printf("serving on port %d\n", port);
			// IPv4
			if(protocol == 4) {
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = inet_addr(ipstr.c_str());
				// addr.sin_addr.s_addr = htons(INADDR_ANY);
				addr.sin_family = AF_INET;
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv4_tcp_server_sustained_test(&addr, port, operation, n_count, delay);
			}
			// IPv6
			if(protocol == 6) {
				server = gethostbyname2(ipstr.c_str(),AF_INET6);
    			memset((char *) &addr6, 0, sizeof(addr6));
    			addr6.sin6_flowinfo = 0;
    			addr6.sin6_family = AF_INET6;
    			addr6.sin6_port = htons(port);
				addr6.sin6_addr = in6addr_any;
    			//memmove((char *) &addr6.sin6_addr.s6_addr, (char *) server->h_addr, server->h_length);
				//printf(" running (%d) test as ipv=%d\n", mode, protocol);
				return ipv6_tcp_server_sustained_test(&addr6, port, operation, n_count, delay);
			}
		}
	}
	return 0;
}




/****************************************************************************/
/* main (calls test driver: do_test(...))                                   */
/****************************************************************************/

// zt2 c7cd7c9e1b0f52a2 simple 4 client 10.9.9.40 8787 n_seconds 10 50
// int do_test(std::string path, std::string nwid, int type, int protocol, int mode, char *ipstr, int port, int operation, int n_count, int delay)

int main(int argc , char *argv[])
{
    if(argc < 3) {
        printf("usage: ./unit <path> <nwid> <simple|sustained|random> <4|6> <client|server> <port> <operation> <count> <delay>\n");     
        return 1;
    }

    int err       = 0;
	int type      = 0;
    int protocol  = 0;
    int mode      = 0;
    int port      = 0;
    int operation = 0;
	int n_count   = 0;
	int delay     = 0;

	std::string path  = argv[1];
	std::string nwid  = argv[2];
	std::string stype = argv[3];
	std::string ipstr, ipstr6;

	memcpy(str, "welcome to the machine", 22);

	// If we're performing a non-random test, join the network we want to test on
	// and wait until the service initializes the SocketTap and provides an address
	if(stype == "simple" || stype == "sustained" || stype == "comprehensive") {
		zts_start(path.c_str());
		printf("waiting for service to start...\n");
		while(!zts_service_running())
			sleep(1);
		printf("joining network...\n");
		zts_join_network(nwid.c_str());
		printf("waiting for address assignment...\n");
		while(!zts_has_address(nwid.c_str()))
			sleep(1);
		printf("complete\n");
	}

	// SIMPLE
	// performs a one-off test of a particular subset of the API
	// For instance (ipv4 client, ipv6 server, etc)
	if(stype == "simple")
	{
		// Parse args
		type     = TEST_TYPE_SIMPLE;
		protocol = atoi(argv[4]);
		if(!strcmp(argv[5],"client"))
			mode = TEST_MODE_CLIENT;
		if(!strcmp(argv[5],"server"))
			mode = TEST_MODE_SERVER;
		ipstr = argv[6];
		port = atoi(argv[7]);
		
		// Perform test
	    if((err = do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay)) == PASSED)
	    	fprintf(stderr, "PASSED\n");
	    else
	    	fprintf(stderr, "FAILED\n");
	    return err;
	}

	// SUSTAINED
	// Performs a stress test for benchmarking performance
	if(stype == "sustained")
	{
		type     = TEST_TYPE_SUSTAINED;
		protocol = atoi(argv[4]);
		if(!strcmp(argv[5],"client"))
			mode = TEST_MODE_CLIENT;
		if(!strcmp(argv[5],"server"))
			mode = TEST_MODE_SERVER;
		ipstr = argv[6];
		port = atoi(argv[7]);


		std::string s_operation = argv[ 8];  // n_count, n_count, n_count
		n_count  = atoi(argv[ 9]); // 10, 100, 1000, ...
		delay    = atoi(argv[10]); // 100 (in ms)
		
		if(s_operation == "n_times")
			operation = TEST_OP_N_TIMES;
		if(s_operation == "n_bytes")
			operation = TEST_OP_N_BYTES;
		if(s_operation == "n_seconds")
			operation = TEST_OP_N_SECONDS;

		// Perform test
	    if((err = do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay)) == PASSED)
	    	fprintf(stderr, "PASSED\n");
	    else
	    	fprintf(stderr, "FAILED\n");
	    return err;
	}

	/****************************************************************************/
	/* COMPREHENSIVE                                                            */
	/****************************************************************************/

	// ./unit zt2 c7cd7c9e1b0f52a2 comprehensive client ipv4 ipv6 9009
	// ./unit zt2 c7cd7c9e1b0f52a2 comprehensive server ipv4 ipv6 9009

	// COMPREHENSIVE
	// Tests ALL API calls
	if(stype == "comprehensive")
	{	
		// Parse args
		type     = TEST_TYPE_SIMPLE;
		if(!strcmp(argv[4],"client"))
			mode = TEST_MODE_CLIENT;
		if(!strcmp(argv[4],"server"))
			mode = TEST_MODE_SERVER;
		ipstr = argv[5];
		ipstr6 = argv[6];
		port = atoi(argv[7]);

		/* Each host must operate as the counterpoint to the other, thus, each mode 
		 * will call the same test helper functions in different orders
		 * Additionally, the test will use the preset paremeters below for the test:
		 */

		 int test = 0;
		 printf("comprehensive\n");
		 printf("test = %d\n", test);
		 test = !test;
		 printf("test = %d\n", test);

		delay     =  0;
		n_count   = 10;
		type      = TEST_TYPE_SIMPLE;
		operation = TEST_OP_N_TIMES;

		// IPV4
		protocol = 4;
		// perform first test arrangement
		do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay);
		sleep(1);
		do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay);
		sleep(1);
		// swtich modes
		if(mode == TEST_MODE_SERVER)
			mode = TEST_MODE_CLIENT;
		else if(mode == TEST_MODE_CLIENT)
			mode = TEST_MODE_SERVER;
		// perform second test arrangement
		do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay);
		sleep(1);
		do_test(path, nwid, type, protocol, mode, ipstr, port, operation, n_count, delay);

		// IPV6
		protocol = 6;
		// perform first test arrangement
		do_test(path, nwid, type, protocol, mode, ipstr6, port, operation, n_count, delay);
		sleep(1);
		do_test(path, nwid, type, protocol, mode, ipstr6, port, operation, n_count, delay);
		sleep(1);
		// swtich modes
		if(mode == TEST_MODE_SERVER)
			mode = TEST_MODE_CLIENT;
		else if(mode == TEST_MODE_CLIENT)
			mode = TEST_MODE_SERVER;
		// perform second test arrangement
		do_test(path, nwid, type, protocol, mode, ipstr6, port, operation, n_count, delay);
		sleep(1);
		do_test(path, nwid, type, protocol, mode, ipstr6, port, operation, n_count, delay);


		/*
		ipv4_tcp_client_test
		ipv6_tcp_client_test
		ipv4_tcp_server_test
		ipv6_tcp_server_test
		ipv4_tcp_client_sustained_test
		ipv6_tcp_client_sustained_test
		ipv4_tcp_server_sustained_test
		ipv6_tcp_server_sustained_test
		*/
	}


	/****************************************************************************/
	/* RANDOM                                                                   */
	/****************************************************************************/

	// RANDOM
	// performs random API calls with plausible (and random) arguments/data
	if(stype == "random")
	{
		random_api_test();
	}

	while(1)
		sleep(1);
	return 0;
}