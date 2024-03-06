#include "chat_head.h"

cid_t *client_list; // 定义client_list变量

void * pthread_fun(void *arg)
{
    //cid是从主线程传过来的。
    cid_t *new_client = (cid_t *)arg;   
    int pcid = new_client->cidnum;
    //清空bufs
    bzero(bufs,sizeof(bufs));
    display(client_list);




    while(1)
    {
        //cid 要传递过来
        bzero(bufs,sizeof(bufs));
        len = recv(pcid,bufs,sizeof(bufs),0);
        bufs[len-1]='\0';
        printf("%s\n",bufs);
        if(!strncmp(bufs,"quit",4))
        {
            printf("客户端正在退出\n");
            new_client->user_status.status=0;
            if(!deletenode(new_client))
            {
                printf("客户端退出成功，删除节点成功\n");
            }
            else
            {
                printf("客户端退出失败，未删除节点\n");
            }
            display(client_list);
            break;
        }
        if(!strncmp(bufs,"userinfo:",9))
        {
            strtok(bufs,":");
            strcpy(new_client->user_status.username,strtok(NULL,":"));
            new_client->user_status.status=1;
        }

    }
    //shutdown(pcid,SHUT_RDWR);
    close(pcid);
    pcid = -1; 
    //线程退出
    pthread_exit((void *)0);
}

int Tcp_init(const char *ip, const char *port)
{
    //1.创建套接字
    int sevid = socket(AF_INET, SOCK_STREAM, 0);  //服务器sevid
    if(sevid < 0)
    {
        perror("socket create fail!\n");
        return -1;
    }
    printf("socket create success:%d\n",sevid);

    //2.绑定地址结构体
    struct sockaddr_in sevaddr = {0};
    sevaddr.sin_family = AF_INET;
    sevaddr.sin_addr.s_addr = inet_addr(ip);
    sevaddr.sin_port = htons(atoi(port));

    int bindinfo = bind(sevid, (struct sockaddr *)&sevaddr, sizeof(sevaddr));
    if(bindinfo < 0)
    {
        perror("bind error!\n");
        return -1;
    }
    printf("bind success!\n");

    listen(sevid,10); //最大接收10个客户端连接请求


    pthread_t tid = 0;
    socklen_t addrlen ;

    while(1)
    {
        cid_t *new_client = (cid_t*)malloc(sizeof(cid_t));
        new_client->next = NULL;
        addrlen = sizeof(new_client->csock);
        new_client->cidnum = accept(sevid,(struct sockaddr*)&new_client->csock,&addrlen);
        
        if(new_client->cidnum !=-1)
        {   
            new_client->user_status.status=1;//登录状态设置在线
            Insertend(client_list, new_client);
            pthread_create(&tid,NULL,pthread_fun,new_client);
            pthread_detach(tid);
            printf("用户登录到IP:%s:%d\n",inet_ntoa(new_client->csock.sin_addr),ntohs(new_client->csock.sin_port));
        }
    }
    close(sevid);
    return 0;
}

int main(int argc, char *argv[]){

    if(argc < 3){
        perror("useage:./server + ip + port");
        return -1;
    }


    client_list = newLinkNode(0);
    Tcp_init(argv[1], argv[2]);
    


    
    return 0;
}