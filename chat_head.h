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
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"


char bufs[512] = "";//线程语句缓存
int len;

//用户状态结构体
struct user_socket {
    char username[20];  //用户名
    int status;        //在线为1，离线非1
};

//连接的线程结构体
typedef struct clientid{
    int cidnum;    //客户端id
    struct  sockaddr_in csock;    //客户端ip结构体
    struct user_socket user_status;
    struct clientid *next;    //链表的下一节
}cid_t;
cid_t *client_list;


cid_t * newLinkNode(int val){
    cid_t *cidlist = (cid_t *)malloc(sizeof(cid_t));
    cidlist->cidnum = val;
    cidlist->csock.sin_family = AF_INET;  // 设置地址族为 IPv4
    cidlist->csock.sin_addr.s_addr = htonl(INADDR_ANY);  // 设置 IP 地址为 INADDR_ANY，表示接收所有地址的连接
    cidlist->csock.sin_port = htons(0);
    strcpy(cidlist->user_status.username,"NULL");
    cidlist->user_status.status=1;
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

void display(cid_t *node)
{
    if(node->next == NULL){
        printf("链表为空!");
        return ;
    }

    cid_t *tmp = node->next;
    while(tmp != NULL){
        printf("user:%s clientid:%d IP:%s:%d ",tmp->user_status.username,tmp->cidnum,inet_ntoa(tmp->csock.sin_addr),ntohs(tmp->csock.sin_port));
        if(tmp->user_status.status == 1)
        {
            printf("在线\n");
        }else{
            printf("离线\n");
        }
        
        tmp = tmp->next;
    }

}

int deletenode(cid_t *delnode)
{
    if(client_list->next == NULL){
        perror("链表为空\n");
        return -1;
    }
    cid_t *cur = client_list;
    while(cur->next != NULL)
    {
        if(cur->next == delnode)
        {
            cur->next = cur->next->next;
            return 0;
        }else
        {
            cur = cur->next;
        }
    }
    return -1;
}





#endif