#include <iostream>
#include <cstring>
#include "prints.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <string.h>
#include <netdb.h>
using namespace std;
char * local_dir_path;

int read_data(int s, char *buf, int n) {
    int bcount;       /* counts bytes read */
    int br;               /* bytes read this pass */
    bcount= 0;
    br= 0;

    while (bcount < n) { /* loop until full buffer */
        br = read(s, buf, n-bcount);
        if (br > 0)  {
            bcount += br;
            buf += br;
        }
        if (br < 1) {
            return(-1);
        }
    }
    return(bcount);
}

int establish(unsigned short portnum) {
    char myname[HOST_NAME_MAX+1];
    int s;
    struct sockaddr_in sa;
    struct hostent *hp;
//hostnet initialization
    gethostname(myname, HOST_NAME_MAX);
    hp = gethostbyname(myname);
    if (hp == NULL)
        return(-1);
//sockaddrr_in initlization
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = hp->h_addrtype;
/* this is our host address */
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
/* this is our port number */
    sa.sin_port= htons(portnum);
    /* create socket */
    if ((s= socket(AF_INET, SOCK_STREAM, 0)) <
        0)
        return(-1);
    if (bind(s , (struct sockaddr *)&sa , sizeof(struct
            sockaddr_in)) < 0) {
        close(s);
        return(-1);
    }
    printf(SERVERS_BIND_IP_STR,inet_ntoa(sa.sin_addr));
    listen(s, 3); /* max 3 of queued connects */
    return(s);
}

int get_connection(int s) {
    int t; /* socket of connection */
    if ((t = accept(s,NULL,NULL)) < 0)
        return -1;
    return t;
}

int IP_Addresses(char * IP_adrress,char * IP_port) {
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3490);
    inet_aton(IP_adrress, &(my_addr.sin_addr));
    memset(&(my_addr.sin_zero), '\0', 8);
    return 0;
}

int call_socket(char *hostname, unsigned short portnum) {
    struct sockaddr_in sa;
    struct hostent *hp;
    int s;
    if ((hp= gethostbyname (hostname)) == NULL)
    {
        return(-1);
    }
    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr , hp->h_addr ,hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short)portnum);
    if ((s = socket(hp->h_addrtype,
                    SOCK_STREAM,0)) < 0) {
        return(-1);
    }
    if (connect(s, (struct sockaddr *)&sa , sizeof(sa))
        < 0) {
        close(s);
        return(-1);
    }
    return(s);
}

int handleClientRequest(int clientSock){
    char ip[1000];
    char upORDown[1000];
    char path[1000];
    char remote[1000];

    read(clientSock,ip,1000);
    read(clientSock,upORDown,1000);
    read(clientSock,path,1000);
    read(clientSock,remote,1000);
    printf(CLIENT_IP_STR,ip);
    printf(CLIENT_COMMAND_STR,upORDown[0]);
    printf(FILENAME_STR,path);
    printf(FILE_PATH_STR,remote);

    return 0;
}

int server(int serverSockfd)
{
    int MAX_CLIENTS = 3;
    fd_set clientsfds;
    fd_set readfds;
    int t;
    FD_ZERO(&clientsfds);
    FD_SET(serverSockfd, &clientsfds);
    FD_SET(STDIN_FILENO, &clientsfds);
    while (true) {
        cout<<(WAIT_FOR_CLIENT_STR);
        readfds = clientsfds;
        if (select(MAX_CLIENTS+1, &readfds, NULL,
                   NULL, NULL) < 0) {
            return -1;
        }
        if (FD_ISSET(serverSockfd, &readfds)) {
            //will also add the client to the clientsfds
            t = get_connection(serverSockfd);
            //handleClientRequest(t);


        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {

            char ip[1000];
            //int n = 0;
            //bzero(ip, sizeof(ip));
            read(STDIN_FILENO,ip,1000);
            //printf(ip);
            ip[4] = '\0';   /// to do - find diferent way for comapring ip to quit (and change name of ip...)
            if (strcmp(ip,"quit") == 0){
                printf("break");
                break;
            }

        }
        else {
            //will check each client if it’s in readfds
            //and then receive a message from him
            handleClientRequest(t);
        }
    }
}

int client_start(char *update_download, char * local_path,char * remote_name,char * numport, char * ip){
    char* endD;
    int t;
    auto portnum = (u_short)strtol(numport,&endD,10);
    t= call_socket(ip, portnum);
    if (t ==-1){
        return -1;
    }
    printf(CONNECTED_SUCCESSFULLY_STR);

    write(t,ip,1000);

    write(t,update_download,1000);
    write(t,local_path,1000);
    write(t,remote_name,1000);


    return 0;

}

int main(int argc, char *argv[]){

    if (strcmp(argv[1],"-s") == 0){
        char* end;
        auto portnum = (u_short)strtol(argv[3],&end,10);
        local_dir_path =argv[3];
        int s = establish(portnum);
        server(s);
    }
    else if (strcmp(argv[1],"-d")==0 ){
        client_start("d",argv[2],argv[3],argv[4],argv[5]);
    }
    else if (strcmp(argv[1],"-u")==0 ){
        client_start("u",argv[2],argv[3],argv[4],argv[5]);
    }

    return 0;
}