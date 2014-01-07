// function prototypes
void  close_socks         (int close_socket);
int   getconn             (char *host, int port);
int   connectit           (struct sockaddr_in *saddr);
void  send_input          (int send_socket, char *send_buff);
int   infromsock          (int generic_socket, unsigned char *buffer,int  size);

