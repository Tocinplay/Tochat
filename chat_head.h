#ifndef __CHAT_HEAD_H__
#define __CHAT_HEAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "cJSON.h"

int sevid; //服务器sevid
char bufs[512] = "";//线程语句缓存
int len;
typedef struct clientid{
    int cidnum;
    struct  sockaddr_in csock;
    struct clientid *next;
}cid_t;

cid_t * newLinkNode(int val){
    cid_t *cidlist;
    cidlist = (cid_t *)malloc(sizeof(cid_t));
    cidlist->cidnum = val;
    cidlist->next = NULL;
    return cidlist;
}

//尾插法
int Insertend(cid_t *tail,cid_t *insertnode){
    while(tail->next != NULL){
        tail=tail->next;
    }
    tail->next=insertnode;
    if(insertnode->next != NULL){
        insertnode->next = NULL;
    }
    return 0;
}

int acceptlink();

#endif