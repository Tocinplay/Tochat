#include "chat_head.h"

cid_t *client_list; // 定义client_list变量

void * pthread_fun(void * arg)
{
    //cid是从主线程传过来的。   
    int pcid = *(int *)arg;
    //清空bufs
    bzero(bufs,sizeof(bufs));
    cJSON * json_res = NULL;
    cJSON * json_name = NULL;
    cJSON * json_code = NULL;
    cJSON * json_info = NULL;
    cJSON* json_sign = NULL;
    cJSON* fuhao = NULL;

    while(strncmp(bufs,"quit",4))
    {
        //cid 要传递过来
        bzero(bufs,sizeof(bufs));
        len = recv(pcid,bufs,sizeof(bufs),0);
        //解析bufs
        json_res = cJSON_Parse(bufs);
        //姓名
        json_name = cJSON_GetObjectItem(json_res,"name");
        //密码
        json_code = cJSON_GetObjectItem(json_res,"code");
        //信息
        json_info = cJSON_GetObjectItem(json_res,"info");
        //签名
        json_sign = cJSON_GetObjectItem(json_res,"sign");
        //符号
        fuhao = cJSON_GetObjectItem(json_res,"fuhao");
        printf("%s:%s-%s:%s",json_name->valuestring,json_sign->valuestring,fuhao->valuestring,json_info->valuestring);

    }
    //线程退出 - cid关闭是不是一件事？
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
        addrlen = sizeof(new_client->csock);
        new_client->cidnum = accept(sevid,(struct sockaddr*)&new_client->csock,&addrlen);
        if(new_client->cidnum !=-1)
        {
            Insertend(client_list, new_client);
            pthread_create(&tid,NULL,pthread_fun,&new_client->cidnum);
            pthread_detach(tid);
            printf("用户登录到IP:%s:%d\n",inet_ntoa(new_client->csock.sin_addr),ntohs(new_client->csock.sin_port));
        }
    }
    return 0;
}

int main(int argc, char *argv[]){

    int sevid; //服务器sevid
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    puts(hostname);
    client_list = newLinkNode(0);
    // Tcp_init(argv[1], argv[2]);
    
    // acceptlink(sevid);

    close(sevid);
    return 0;
}