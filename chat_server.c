#include "chat_head.h"

extern cid_t *client_list; // 定义client_list变量

void *pthread_fun(void *arg)
{
    // cid是从主线程传过来的。
    cid_t *new_client = (cid_t *)arg;
    int pcid = new_client->cidnum;
    int flag = 1;              //判断是不是第一次连接
    char formatted_time[64];  //格式化时间
    time_t json_time_value;   //时间结构体
    // char fh[5] = ""; 
    // bzero(fh,5);
    // 清空bufs
    bzero(bufs, sizeof(bufs));
    // display(client_list);

    //----------cJSON代码------------
    cJSON *json_res = NULL;  //result
    cJSON *json_name = NULL;  //username
    cJSON *json_info = NULL;  //received information
    // cJSON* json_sign = NULL;  //personal sign
    cJSON *json_fuhao = NULL;  //personal style sign
    cJSON *json_conn = NULL;  //link type
    cJSON *json_time = NULL;



    while (1)
    {
        // cid 要传递过来
        bzero(bufs, sizeof(bufs));
        len = recv(pcid, bufs, sizeof(bufs), 0);
        
        if (len <= 0)
        {
            break;
        }

        // ————————————————cJSON代码块————————————————
        json_res = cJSON_Parse(bufs);
        if (json_res == NULL)
        {
            continue;
        }

        // 姓名
        json_name = cJSON_GetObjectItem(json_res, "name");
        // 信息
        json_info = cJSON_GetObjectItem(json_res, "info");
        // 签名
        //  json_sign = cJSON_GetObjectItem(json_res,"sign");
        
        //time ---- 解析客户端发送过来的时间
        json_time = cJSON_GetObjectItem(json_res,"time");
        
        // 符号
        json_fuhao = cJSON_GetObjectItem(json_res, "fuhao");
        // bzero(fh,5);
        // strcat(fh, json_fuhao->valuestring);
        // 提示第一次连接
        if (flag)
        {
            
            json_conn = cJSON_GetObjectItem(json_res, "conn");


            // 如果是第一次连接就把用户名写入链表结构体中，不是第一次连接就不用写了
            if (!strncmp(json_conn->valuestring, "f1rst", 5))
            {
                // 把用户名信息存储到链表结构体中
                if (new_client->user_status.username[0] == '\0')
                {
                    strcat(new_client->user_status.username, json_name->valuestring);
                    display(client_list);
                    flag--;
                }
                cJSON_DeleteItemFromObject(json_res, "conn");
                // cJSON_Delete(json_res);
                continue;
            }
            
        }
        
        json_time_value = (time_t)json_time->valueint;
        
        bzero(formatted_time,sizeof(formatted_time));
        strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", localtime(&json_time_value));


        
          
        if (!strncmp(json_info->valuestring, "quit", 4))
        {
            printf("客户端正在退出\n");
            new_client->user_status.status = 0;
            if (!deletenode(new_client))
            {
                printf("客户端退出成功，删除节点成功\n");
                display(client_list);
            }
            else
            {
                printf("客户端退出失败，未删除节点\n");
            }

            break;
        }
        
        printf("%s%s%s%s%s", json_name->valuestring, json_fuhao->valuestring, formatted_time, json_fuhao->valuestring, json_info->valuestring);
        // while(1);
    }
    shutdown(pcid, SHUT_RDWR);
    close(pcid);
    pcid = -1;
    // 线程退出
    pthread_exit((void *)0);
}

int Tcp_init(const char *ip, const char *port)
{
    // 1.创建套接字
    int sevid = socket(AF_INET, SOCK_STREAM, 0); // 服务器sevid
    if (sevid < 0)
    {
        perror("socket create fail!\n");
        return -1;
    }
    printf("socket create success:%d\n", sevid);

    // 2.绑定地址结构体
    struct sockaddr_in sevaddr = {0};
    sevaddr.sin_family = AF_INET;
    sevaddr.sin_addr.s_addr = inet_addr(ip);
    sevaddr.sin_port = htons(atoi(port));

    int bindinfo = bind(sevid, (struct sockaddr *)&sevaddr, sizeof(sevaddr));
    if (bindinfo < 0)
    {
        perror("bind error!\n");
        return -1;
    }
    printf("bind success!\n");

    listen(sevid, 10); // 最大接收10个客户端连接请求

    pthread_t tid = 0;
    socklen_t addrlen;

    while (1)
    {
        cid_t *new_client = (cid_t *)malloc(sizeof(cid_t));
        new_client->next = NULL;
        bzero(&new_client->user_status, sizeof(struct user_socket));
        strcpy(new_client->user_status.username, "");
        addrlen = sizeof(new_client->csock);
        new_client->cidnum = accept(sevid, (struct sockaddr *)&new_client->csock, &addrlen);

        if (new_client->cidnum != -1)
        {
            new_client->user_status.status = 1; // 登录状态设置在线
            Insertend(client_list, new_client);
            pthread_create(&tid, NULL, pthread_fun, new_client);
            pthread_detach(tid);
            printf("用户登录到IP:%s:%d\n", inet_ntoa(new_client->csock.sin_addr), ntohs(new_client->csock.sin_port));
        }
    }
    close(sevid);
    return 0;
}

// 获取ip地址
char *getIPAddress()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *ipAddress = (char *)malloc(16 * sizeof(char));

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            if (strcmp(ifa->ifa_name, "lo") != 0)
            { // Exclude loopback interface
                strcpy(ipAddress, inet_ntoa(sa->sin_addr));
                break;
            }
        }
    }
    freeifaddrs(ifap);

    return ipAddress;
}

int main(int argc, char *argv[])
{

    // if (argc < 3)
    // {
    //     perror("useage:./server + ip + port");
    //     return -1;
    // }
    char *ipstr = getIPAddress();
    // puts(argv[1]);
    client_list = newLinkNode(0);
    Tcp_init(ipstr, argv[1]);

    return 0;
}