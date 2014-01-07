#define HTTP    1
#define FTP     2
#define NNTP    3
#define GOPHER  4
#define FINGER  5
#define TIME    6
#define POPMAIL 7
#define FILES   8
#define TELNET  9

typedef struct line // btree
{
   char *text;
   struct line *left;
   struct line *right;
}NODE;

typedef struct list
{
   char *text;
   struct list *next;
}LIST;

typedef struct {
   int  Fflag;        // the individual thread's event semaphore
   int  hostp;        // FTP server port num for data
   char host[255];    // IP of FTP Data server
   char lhost[1024];  // long hostname
   int  ls_list;      // flag to get dir instead of file
} FTP_DATA;

/* Function Prototypes */
void   main                (int argc, char **argv);   // begin da program
void  handle_popmail      (void);
void  handle_finger       (void);
void  handle_file         (void);
void  handle_time         (void);
void  handle_gopher       (void);
void  handle_newslist     (void);
void  handle_ftp          (void);
void  handle_http         (void);
void  handle_telnet       (void);
void  handle_nntp         (void);                  // in nntp
int   listgroup           (int reset_startx);      // in nntp
void  watch               (char *seeme);           // if watch_server is set, this prints out the requests & server responses
void  writef              (char *fee);             // actually writes the file
void  _Optlink ftp_con    (void *args);
void  _Optlink check_time (void *foo);
char  *unhtml             (char *linein);
int   getopt              (int argc, char **argv, char *opts);
void  parseOptions        (int argc, char **argv, char** option_output);
void  bannr               (void);
void  usage               (void);
void  error_out           (char *erro);
void  warning_out         (char *erro);
void  close_exit          (void);
int   getaline            (int sock, char *data);
void  write_file          (void *);
int   getnews             (char *action, int remain);
char  *makeupper          (char **maybelower);
void  bug                 (void);
void  showurl             (void);
void  clear_file          (void);
void  log_to_file         (char *line_to_log);
int   uudecode            (void);
void  check_options       (void);
void  reverse_slashes     (char **pathstring);
int   get_site            (char *html,int len);
void  sig_handler         (int sig);

int   find_glob           (char *pattern);
int   glob                (char *pattern, char *checkme);

NODE  *allocate           (char *text);                         //btree
NODE  *add_to_tree        (NODE *, NODE *);                     //btree
void  write_sorted_tree   (NODE *);                             //btree
NODE  *add_node           (char *text);                         //btree

int   add_to_list         (char *addme);                        // linked list LIST
void  print_list          (void);                               // linked list LIST
char  *get_next_from_list (void);

