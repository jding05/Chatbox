// selectserver.c -- a cheezy multiperson chat server
// 7.2

// to start the server
// run ./selectserver

// to start the connection: 2 types (on different terminal)
// 1. run $> telnet hostname 4444  ** issue with Crtl C ** doesn't terminate the program
// 2. run $> nc hostname 4444
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "4444" // port we're listening on

//--------------------------------------------------------------
// additional itoa_base()
char    *ft_itoa_base(int nbr)
{
	int		len;
	long	n_tmp;
	char	*str;

	if (nbr == -2147483648)
		return ("-2147483648");
	len = 0;
	n_tmp = nbr;
	while (n_tmp)
	{
		n_tmp /= 10;
		len += 1;
	}
	if (nbr < 0)
	{
		len += 1;
		nbr *= -1;
	}
	if (!(str = (char *)malloc(sizeof(char) * len + 1)))
		return (NULL);
	str[len--] ='\0';
	while (nbr)
	{
		str[len] = (nbr % 10) + '0';
		nbr /= 10;
		len--;
	}
	return (str);
}
//------------------------------------------------------------------
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	fd_set		master; // master file descriptor list 
	fd_set		read_fds; // temp file descriptor list for select()
	int			fdmax; // maximum file descriptor number
	int			listener; // listening socket descriptor
	int			newfd; // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t	addrlen;
	char		buf[256]; // buffer for client data
	int			nbytes;
	char		remoteIP[INET6_ADDRSTRLEN];
 	int			yes=1; // for setsockopt() SO_REUSEADDR, below
 	int			i, j, rv;
	struct addrinfo hints, *ai, *p;
	char		*cfd; // for newfd to print, need to change to itoa() use


	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);
 // get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
		fprintf(stderr, "chatbox: %s\n", gai_strerror(rv));
		exit(1);
	}
 	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
			continue;
 // lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
 			continue;
 		}
		break;
	}
 // if we got here, it means we didn't get bound
	if (p == NULL)
	{
		fprintf(stderr, "chatbox: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(ai); // all done with this
 // listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}
 // add the listener to the master set
	FD_SET(listener, &master);
 // keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one
 // main loop
//	while(1)
	read_fds = master;
	while(select(fdmax+1, &read_fds, NULL,NULL,NULL) > -1)
	{
	// run through the existing connections looking for data to read
		i = -1;
		while (++i <= fdmax)
		{
			if (FD_ISSET(i, &read_fds)) // we got one!!
			{
				if (i == listener) // handle new connections
				{
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
					if (newfd == -1)
						perror("accept");
					else
					{
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) // keep track of the max
							fdmax = newfd;
						printf("\033[0;33mchatbox: new connection from %s on socket %d\n\033[0m",
 						inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
						send(newfd, "your socket fd is: ", 19, 0);
						// this part new itoa() to replace cfd
						cfd = ft_itoa_base(newfd);
						send(newfd, cfd, strlen(cfd), 0);
						send(newfd, "\n", 1, 0);
						send(newfd, "\033[0;35m[Session Start ...]\n\033[0m$>", 34, 0); 
					}
				}
				else // handle data from a client
				{
					if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
					{// got error or connection closed by client
						if (nbytes == 0) // connection closed
 							printf("\033[0;31mchatbox: socket %d hung up\n\033[0m", i);
					   	else
							perror("recv");
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					}
					else // we got some data from a client
					{
						j = 0;
						while (j <= fdmax)
						{
 							if (FD_ISSET(j, &master)) // send to everyone!
							{
 								if (j != listener && j != i) // except the listener and ourselves
								{
 									if (send(j, buf, nbytes, 0) == -1)
										 perror("send");
									send(j, "\033[0;33mmsg from client: \033[0m", 29, 0);
									send(j, ft_itoa_base(i), strlen(ft_itoa_base(i)), 0);
									send(j, "\n$>", 3, 0);	
								}
								else if (j == listener)
									printf("msg from client:%d -->%s", i, buf);
								else if (j == i)
									send(i, "\033[0;31myour msg has been sent\033[0m\n$>", 40, 0);	
							}
							j++;
						}
					}
				} // END handle data from client
	  	 	} // END got new incoming connection
		//	i++;
		} // END looping through file descriptors
		read_fds = master;
	} // END while(1)--and you thought it would never end!
	return 0;
}
