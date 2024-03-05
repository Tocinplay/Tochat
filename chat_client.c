#include "../header.h"



int Tcp_init(int *id,const char *ip, const char *port)
{
    //1.创建套接字
    id = socket(AF_INET, SOCK_STREAM, 0);  //服务器sevid
    if(id < 0)
    {
        perror("socket create fail!\n");
        return -1;
    }
    printf("socket create success:%d\n",id);

    //2.绑定地址结构体
    struct sockaddr_in sevaddr = {0};
    sevaddr.sin_family = AF_INET;
    sevaddr.sin_addr.s_addr = inet_addr(ip);
    sevaddr.sin_port = htons(atoi(port));

    int bindinfo = bind(id, (struct sockaddr *)&sevaddr, sizeof(sevaddr));
    if(bindinfo < 0)
    {
        perror("bind error!\n");
        return -1;
    }
    printf("bind success!\n");


    close(id);
    return 0;
}

int main(int argc, char const *argv[])
{
    int clid = 0;
    Tcp_init(&clid,argv[0],argv[1]);
    
    return 0;
}
