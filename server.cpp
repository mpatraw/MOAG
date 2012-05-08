#include "moag.h"
#include <unistd.h>
#include <math.h>





void sendChat(int to, int id, char cmd, const char* msg, unsigned char len){
    if(to<-1 || to>=MAX_CLIENTS || id<-1 || id>=MAX_CLIENTS || (id>=0 && !clients[id]))
        return;

    // Prepare chunk.
    moag::ChunkEnqueue8(MSG_CHUNK);
    moag::ChunkEnqueue8(id);
    moag::ChunkEnqueue8(cmd);
    moag::ChunkEnqueue8(len);
    for(int i=0;i<len;i++)
        moag::ChunkEnqueue8(msg[i]);

    // Send chunk.
    if(to!=-1){
        if(clients[to] && moag::SendChunk(clients[to], -1, 1)==-1)
            disconnect_client(to);
    }else{
        for(int i=0;i<MAX_CLIENTS;i++)
            if(clients[i] && moag::SendChunk(clients[i], -1, 0)==-1)
                disconnect_client(i);
    }

    // Clear the queue buffer.
    moag::SendChunk(NULL, -1, 1);
}








void handleMsg(int id, const char* msg, int len){
    if(msg[0]=='/' && msg[1]=='n' && msg[2]==' '){
        char notice[64]="  ";
        strcat(notice,tanks[id].name);
        len-=3;
        if(len>15)
            len=15;
        for(int i=0;i<len;i++)
            tanks[id].name[i]=msg[i+3];
        tanks[id].name[len]='\0';
        strcat(notice," is now known as ");
        strcat(notice,tanks[id].name);
        sendChat(-1,id,2,tanks[id].name,strlen(tanks[id].name));
        sendChat(-1,-1,3,notice,strlen(notice));
        return;
    }
    sendChat(-1,id,1,msg,len);
}



int main(int argc, char *argv[])
{
    if (moag::OpenServer(8080, MAX_CLIENTS) == -1) {
        printf("Failed to start server\n");
        return 1;
    }

    moag::SetServerCallback(client_connect, moag::CB_CLIENT_CONNECT);
    moag::SetServerCallback(server_update, moag::CB_SERVER_UPDATE);

    initGame();
    printf("Started server\n");

    while (1)
        moag::ServerTick();

    moag::CloseServer();

    return 0;
}

