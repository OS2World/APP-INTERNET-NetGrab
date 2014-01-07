#include <types.h>
#include <netinet\in.h>
#include <sys\socket.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
#include <conio.h> // just for cprintf
#include "sockets.h"

/*
//struct sockaddr_in saddr;          //the address that we're connecting to.

In the Internet namespace, a socket address consists of a host address and a port on that host. In addition, the protocol you choose serves effectively as a part of the
address because local port numbers are meaningful only within a particular protocol.

The data type for representing socket addresses in the Internet namespace is defined in the header file `netinet/in.h'.

Data Type: struct sockaddr_in
     This is the data type used to represent socket addresses in the Internet namespace. It has the following members:

     short int sin_family
         This identifies the address family or format of the socket address. You should store the value of AF_INET in this member. See section Socket
         Addresses.
     struct in_addr sin_addr
         This is the Internet address of the host machine. See section Host Addresses, and section Host Names, for how to get a value to store here.
     unsigned short int sin_port
         This is the port number. See section Internet Ports.

When you call bind or getsockname, you should specify sizeof (struct sockaddr_in) as the length parameter if you are using an Internet namespace
socket address.





// ** creating a socket **
// int socket (int namespace, int style, int protocol)
//
// The return value from socket is the file descriptor for the new socket, or -1 in case of error.
// namespace for internet is AF_INET
// style is SOCK_STREAM for tcp
// style is SOCK_DGRAM for udp?
// style SOCK_RAW provides access to low-level network protocols and interfaces.
//  zero is usually right for protocol.


Function: int connect (int socket, struct sockaddr *addr, size_t length)
     The connect function initiates a connection from the socket with file descriptor socket to the socket whose address is specified by the addr and length
     arguments. (This socket is typically on another machine, and it must be already set up as a server.) See section Socket Addresses, for information about how
     these arguments are interpreted.

     Normally, connect waits until the server responds to the request before it returns. You can set nonblocking mode on the socket socket to make connect
     return immediately without waiting for the response. See section File Status Flags, for information about nonblocking mode.

     The normal return value from connect is 0. If an error occurs, connect returns -1. The following errno error conditions are defined for this function:

*/




/***********************************************************/
/*                        CLOSE_SOCKS                      */
/* shuts down a specific socket                            */
/* inputs: socket number                                   */
/* outputs: none                                           */
/***********************************************************/
void close_socks(int close_socket) // shuts down a specific socket
{
   shutdown(close_socket, 2);  // who cares - were outta here.
   soclose(close_socket);      // ditto.
   return;
}

/***********************************************************/
/*                           SEND_INPUT                    */
/* sends buffer out of socket                              */
/* inputs: socket number, buffer                           */
/* outputs: none                                           */
/***********************************************************/

void send_input(int send_socket,char *send_buff)
{
   send(send_socket ,send_buff, strlen(send_buff),0x0);
}

/***********************************************************/
/*                         GETALINE                        */
/* pulls in a single line from tcpip socket                */
/* inputs: socket number, buffer passed by reference       */
/* outputs: passed (1)or failed (0) to read a line         */
/***********************************************************/

int getaline(int sock, char *data) // builds a single line from the socket input
{
   unsigned char input[2];
   int rval,counter=0;

   input[0] = 0;
   while ( input[0] != '\n' )
   {
      if ((rval = recv(sock, input, 1,0x0)) < 1)  // if we didn't get anything
         return 0;
      data[counter++] = input[0];
   }
   data[counter]=0;
   return 1; // we got it
}


/***********************************************************/
/*                GETCONN                                  */
/* connects to an address  either named or by ip           */
/* inputs:  host(name or ip), port number                  */
/* outputs: socket #  (or -1 if failed to connect)         */
/***********************************************************/

int getconn(char *host, int port)
{
   struct hostent *temp;
   struct sockaddr_in saddr;

   if(isdigit(host[0]))  //checks to make sure that we have an ip, not a name
   {
      if((saddr.sin_addr.s_addr = inet_addr(host)) == (u_short)INADDR_NONE)
         return -1;
   }
   else // gets the ip from the hostname
   {
      if((temp = gethostbyname(host)) == (struct hostent *)0)
         return -1;// could not resolve hostname
      memcpy((char *)&(saddr.sin_addr),(char *)temp->h_addr,sizeof(struct in_addr));
   }

   saddr.sin_family=(short)AF_INET;
   saddr.sin_port = htons((u_short)port);
   return connectit(&saddr);   // socket number
}

/***********************************************************/
/*                CONNECTIT                                */
/*  opens u                                                       */
/*                                                         */
/*                                                         */
/***********************************************************/
int connectit(struct sockaddr_in *saddr)
{
   int generic_socket;
   struct sockaddr *Psaddr;

   Psaddr = (struct sockaddr *) saddr; // cast saddr from sockaddr_in to sockaddr

   if((generic_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      return -1;
   if(connect(generic_socket, Psaddr,sizeof(struct sockaddr_in)) == -1)
      return -1;
   else
      return generic_socket;
}

/***********************************************************/
/*               INFROMSOCK                                */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/

int infromsock(int generic_socket, unsigned char *buffer,int  size)
{
   int numchars;

   if ((numchars = recv(generic_socket, buffer, size, 0x0)) <= 0 )
   { /* panic for now */
      if( errno == 12 )
        return 0;
      else
        return -1;
   }
   if (numchars == 0)  /* foreign end closed the connection be graceful */
      return -1; /* tell the calling proc to deal with it. */
   buffer[numchars] = '\000';
   return numchars;
}

