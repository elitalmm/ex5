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
#define invalidFileNameClint 0
#define invalidFileNameSever 1
#define canNotOpenClint 2
#define canNotOpenSever 3
#define READ 1
#define WRITE 0
#define sysCallFaild ("sysCallFaild\n")

int read_data(int s, char *buf, int n, int mode)
{
    int bcount;       /* counts bytes read */
    int br;               /* bytes read this pass */
    bcount= 0;
    br= 0;

    while (bcount < n) { /* loop until full buffer */
        if (mode)
        {
            br = read(s, buf, n-bcount);
            cout << "br: " << br << endl;
        }
        else
            br = write(s, buf, n-bcount);
        if (br > 0)  {
            bcount += br;
            cout << "bcount" << bcount << endl;
            buf += br;
        }
        if (br == 0)
        {
            break;
        }
        if (br < 1) {
            return(-1);
        }
    }
    cout << "lastbcount" << bcount << endl;
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

std::string makePath(char * severDir,char * fileName){
    std::string str;
    str.append(severDir);
    str.append("/");
    str.append(fileName);
    return str;
}

int handle_upload_sever(int clientSock, char * pathToPrint){
    char revbuf[1000];
    FILE *fr = fopen(pathToPrint, "w");
    if (fr != nullptr){
        cout << "hi" << endl;
        bzero(revbuf, 1000);
        int fr_block_sz = 0;
        while ((fr_block_sz = read_data(clientSock, revbuf, 1000, READ)) > 0) {
            int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if (write_sz < fr_block_sz) {
                printf(sysCallFaild);
                return -1;
            }
            bzero(revbuf, 1000);
            if (fr_block_sz == 0) {
                break;
            }
        }
        if (fr_block_sz < 0) {
            printf(sysCallFaild);
            return -1;
        }
        fclose(fr);
    }
    else{
        printf(sysCallFaild);
    }

}


int handleClientRequest(int clientSock){
    char ip[1000];
    char upORDown[1000];
    char path[1000];
    char fileName[1000];
    int isOk[4] = {0,0,0,0};
    int handle;

    //get file name,up \down
    read(clientSock,ip,1000);
    read(clientSock,upORDown,1000);
    read (clientSock,isOk,4);
    read(clientSock,fileName,1000);

    if (strlen(fileName)+ strlen(local_dir_path)+1 >4095 ){

        isOk[invalidFileNameSever] =1;
    }
    write(clientSock,isOk,4);
    std::string pathStr = makePath(local_dir_path,fileName);
    char pathToPrint[pathStr.length() + 1];
    strcpy(pathToPrint, pathStr.c_str());
    read(clientSock,path,1000);

    //print
    printf(CLIENT_IP_STR,ip);
    printf(CLIENT_COMMAND_STR,upORDown[0]);
    printf(FILENAME_STR,fileName);
    printf(FILE_PATH_STR,pathToPrint);


    //invalidFileNameClint or invalidFileNameSever
    if ((isOk[invalidFileNameClint] ==1) or (isOk[invalidFileNameSever] ==1)){
        printf(FILE_NAME_ERROR_STR);
        printf(FAILURE_STR);
        return -1;
    }

    //if u and no canNotOpenClint -upload
    if (upORDown[0] == 'u') {
        read (clientSock,isOk,4);
        cout<< isOk[canNotOpenClint]<<endl;
        if (isOk[canNotOpenClint] == 0 ) {
            handle = handle_upload_sever(clientSock,pathToPrint);
        }
    }

    if (upORDown[0] == 'd') {
//        read (clientSock,isOk,4);
//        if (isOk[canNotOpenClint] ==0 ) {
//            handle = handle_upload_sever(clientSock,pathToPrint);
//        }
    }

   if (isOk[canNotOpenClint]==1){
        printf(REMOTE_FILE_ERROR_STR);
        printf(FAILURE_STR);
    }
    else if (isOk[canNotOpenSever]==1){
        printf(MY_FILE_ERROR_STR);
        printf(FAILURE_STR);
    }
    else if ( handle == -1){
       printf(sysCallFaild);
    }
    else {
        printf(SUCCESS_STR);
    }
    close(clientSock);
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
            cin >> ip;
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
    return 0;
}

int handle_upload_clint(int s, char *local_path, FILE* file)
{
    // open file and read
    /*Receive File from Client */
    char senbuf[1000];
//    FILE *fs = fopen(local_path, "r");
    if(file == nullptr) {
        printf(sysCallFaild);
        cout << "1";
        return -1;
    }
    else
    {
        bzero(senbuf, 1000);
        int fs_block_sz;
        while ((fs_block_sz = fread(senbuf, sizeof(char), 1000, file)) > 0)
        {
            if (read_data(s, senbuf, fs_block_sz, WRITE) < 0) {
                printf(FAILURE_STR);
                return -1;
            }
            bzero(senbuf, 1000);
        }
    }
    fclose(file );
    return 0;
}

int client_start(char *update_download, char * local_path,char * remote_name,char * numport, char * ip){
    char* endD;
    int t;
    int isOk[4] ={0,0,0,0};


    auto portnum = (u_short)strtol(numport,&endD,10);
    t= call_socket(ip, portnum);
    if (t ==-1){
        return -1;
    }
    printf(CONNECTED_SUCCESSFULLY_STR);
    write(t,ip,1000);
    write(t,update_download,1000);

    // is file name =remote_name ok by clint
    std::string s = remote_name;
    if (strlen(remote_name) > 255 or (s.find('/') != std::string::npos)){
        isOk[invalidFileNameClint] = 1;
    }
    write(t,isOk,4);
    write(t,remote_name,1000);
    // is file name =remote_name ok by sever
    read (t,isOk,4);

    write(t,local_path,1000);


    if (strcmp(update_download,"u") == 0 )
    {
        FILE *fs = fopen(local_path, "r");
        if (fs == nullptr){
            isOk[canNotOpenClint] = 1;
            write(t,isOk,4);
        }
        else {
            int handle = handle_upload_clint(t, remote_name, fs);
        }
    }



    if (isOk[invalidFileNameClint] ==1 or isOk[invalidFileNameSever] ==1){
        printf(FILE_NAME_ERROR_STR);
        cout<<(FAILURE_STR);
    }
    else if (isOk[canNotOpenClint]){
        printf(MY_FILE_ERROR_STR);
        printf(FAILURE_STR);
    }
    else if (isOk[canNotOpenSever]){
        printf(REMOTE_FILE_ERROR_STR);
        printf(FAILURE_STR);
    }

    else {
        printf(SUCCESS_STR);
    }


    return 0;

}

int main(int argc, char *argv[]){
    char* UPLOAD = (char*)"u";
    char* DOWNLOAD = (char*)"d";
    if (strcmp(argv[1],"-s") == 0){
        char* end;
        auto portnum = (u_short)strtol(argv[3],&end,10);
        local_dir_path =argv[2];
        int s = establish(portnum);
        server(s);
    }
    else if (strcmp(argv[1],"-d")==0 ){
        client_start(DOWNLOAD,argv[2],argv[3],argv[4],argv[5]);
    }
    else if (strcmp(argv[1],"-u")==0 ){
        client_start(UPLOAD,argv[2],argv[3],argv[4],argv[5]);
    }

    return 0;
}