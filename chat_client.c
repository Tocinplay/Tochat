#include "chat_head.h"




int main(int argc, char const *argv[])
{
    //判断输入是否正确
    if(argc < 3){
        perror("usage:./client + IP + port\n");
        return -1;
    }

    //初始化套接字，判断是否成功
    int clid = socket(AF_INET,SOCK_STREAM,0);
    if(clid == -1){
        perror("socket create fail\n");
        return -1;
    }
    printf("socket create success\n");

    //connect
    struct sockaddr_in caddr = {0};
    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(atoi(argv[2]));
    caddr.sin_addr.s_addr = inet_addr(argv[1]);
    if(connect(clid,(struct sockaddr*)&caddr,sizeof(caddr)) < 0 )
    {
        perror("connect error\n");
        close(clid);
        return -3;
    }
    printf("connect successful\n");

    char bufs[128] = "";
    //------------------------------------------------------//
    cJSON * json_pointer = NULL;
    //创建一个链表数据对象。
    json_pointer = cJSON_CreateObject();
    //1添加字符串类型到节点当中 姓名
    cJSON_AddStringToObject(json_pointer,"name","jack");
    //2 添加整型数据 密码
    cJSON_AddNumberToObject(json_pointer,"code",12345);
    //3 添加字符串类型到节点当中 信息
    //cJSON_AddStringToObject(json_pointer,"info","hello linux");
    //4 添加字符串类型到节点当中 签名
    cJSON_AddStringToObject(json_pointer,"sign","all is well");
    //添加字符串类型到节点当中 
    cJSON_AddStringToObject(json_pointer,"fuhao",">");
    //数据整理
    char * str = NULL ; 

    //信息节点
    cJSON * json_info = NULL;

    //循环读写
    while(1)
    {
        bzero(bufs,sizeof(bufs));
        //先写 - 从键盘读数据再写给对面
        read(STDIN_FILENO,bufs,sizeof(bufs)-1);   
        cJSON_AddStringToObject(json_pointer,"info",bufs);
        //jason格式转换成字符串格式
        str = cJSON_Print(json_pointer);
        send(clid,str,strlen(str),0);
        if (strncmp(bufs, "quit", 4) == 0) {
            break;
        }
        //发送过去之后，进行删除节点。
        cJSON_DeleteItemFromObject(json_pointer, "info");
    }
    //
    shutdown(clid,SHUT_RDWR);
    cJSON_Delete(json_pointer);

    return 0;
}
