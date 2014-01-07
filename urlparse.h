#define URLFTPSIG       "ftp:"
#define URLHTTPSIG      "http:"
#define URLNNTPSIG      "nntp:"
#define URLGOPHERSIG    "gopher:"
#define URLFINGERSIG    "finger:"
#define URLPOPMAILSIG   "popmail:"
#define URLTIMESIG      "time:"
#define URLFILESIG      "file:"
#define URLTELNETSIG    "telnet:"

#define URLFTP     1
#define URLHTTP    2
#define URLNNTP    3
#define URLGOPHER  4
#define URLPOPMAIL 5
#define URLTIME    6
#define URLFINGER  7
#define URLFILE    8
#define URLTELNET  9

typedef struct           // used in:
{
   char type;            //http,nntp,ftp,popmail,time,gopher,finger,file,telnet
   char hostname[1024];  //http,nntp,ftp,popmail,time,gopher,finger     ,telnet
   long port;            //http,nntp,ftp,popmail,time,gopher,finger     ,telnet
   char path[1024];      //http,nntp,ftp             ,gopher       ,file,telnet
   char lookfor[20][128];//     nntp
   int  looknum;         //     nntp
   long startx;          //     nntp    ,popmail
   long endx;            //     nntp    ,popmail
   char username[128];   //          ftp,popmail            ,finger     ,telnet
   char password[128];   //          ftp,popmail                        ,telnet
   unsigned long tock;   /*                     ,time*/
} URLData_t, *URLData_p;

int URLParse(char *url, URLData_p pURLData);
int url_check_begin(char **url, URLData_p pURLData,char *begin);
int get_http_url(char **url, URLData_p pURLData);
int get_ftp_url (char **url, URLData_p pURLData);
int get_nntp_url(char **url, URLData_p pURLData);
int get_goph_url(char **url, URLData_p pURLData);
int get_time_url(char **url, URLData_p pURLData);
int get_mail_url(char **url, URLData_p pURLData);
int get_fing_url(char **url, URLData_p pURLData);
int get_file_url(char **url, URLData_p pURLData);
int get_telnet_url(char **url, URLData_p pURLData);
int parse_netrc(URLData_p pURLData);
int url_get_pass(char **url, URLData_p pURLData);
int url_get_host(char **url, URLData_p pURLData);
int url_get_port(char **url, URLData_p pURLData);
int url_get_netrc(URLData_p pURLData);


