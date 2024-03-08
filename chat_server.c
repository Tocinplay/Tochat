#include "chat_head.h"

extern cid_t *client_list; // 定义client_list变量
extern qun_t *qun_list;
//限定立即锁。
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void Msg_Read(const char *msg,char *username,char *fuhao,int *group,char *conn,int *time,char *info){
    //----------cJSON代码------------
    cJSON *json_res = cJSON_Parse(msg);  //result
    cJSON *json_name = NULL;  //username
    cJSON *json_fuhao = NULL;  //personal style sign
    cJSON *json_group = NULL;
    cJSON *json_conn = NULL;  //link type
    cJSON *json_time = NULL;
    cJSON *json_info = NULL;  //received information

    // 姓名
    json_name = cJSON_GetObjectItem(json_res, "name");
    // 符号
    json_fuhao = cJSON_GetObjectItem(json_res, "fuhao");
    //群聊
    json_group = cJSON_GetObjectItem(json_res, "group_id");
    //连接conn之后进行私聊可以使用这个
    json_conn = cJSON_GetObjectItem(json_res, "conn");
    //time ---- 解析客户端发送过来的时间
    json_time = cJSON_GetObjectItem(json_res,"time");
    // 信息
    json_info = cJSON_GetObjectItem(json_res, "info");

    strcpy(username,json_name->valuestring);
    strcpy(fuhao,json_fuhao->valuestring);
    *group = json_fuhao->valueint;
    strcpy(conn,json_conn->valuestring);
    *time = json_time->valueint;
    strcpy(info,json_info->valuestring);

}

void *pthread_fun(void *arg)
{
    // cid是从主线程传过来的。
    cid_t *new_client = (cid_t *)arg;
    int pcid = new_client->cidnum;
    int flag = 1;              //判断是不是第一次连接
    char formatted_time[64];  //格式化时间
    time_t json_time_value;   //时间结构体

    // 清空bufs
    bzero(bufs, sizeof(bufs));
    // display(client_list);
    
    char username[20] = {0};
    char fuhao[10] = {0};
    int group = 0;
    char conn[10] = {0};
    int time = 0;
    char info[2048] = {0};


    while (1)
    {
        // cid 要传递过来
        bzero(bufs, sizeof(bufs));
        len = recv(pcid, bufs, sizeof(bufs), 0);
        if (len <= 0)
        {
            break;
        }
        //解析cJSON代码块
        Msg_Read(bufs,username,fuhao,&group,conn,&time,info);

        FILE *fp = fopen("dbdir/log.txt","a+");
        if (fp == NULL) {
            // 文件打开失败
            perror("打开文件失败！");
            exit(1);
        }
        
        
        // flock(fp, LOCK_EX);
       
        // fprintf(fp,"%s%s%s%s%s", json_name->valuestring, json_fuhao->valuestring, formatted_time, json_fuhao->valuestring, json_info->valuestring);

        //时间处理函数
        json_time_value = (time_t)time;
        bzero(formatted_time,sizeof(formatted_time));
        strftime(formatted_time, sizeof(formatted_time), "%Y/%m/%d %H:%M:%S", localtime(&json_time_value));

        //加锁
        //阻塞属性，加锁写文件
        if(pthread_mutex_trylock(&mutex))
        {
            perror("加锁失败!\n");
        }
        printf("加锁成功!\n");

        // 提示第一次连接
        if (flag)
        {          
            // 如果是第一次连接就把用户名写入链表结构体中，不是第一次连接就不用写了
            if (!strncmp(conn, "f1rst", 5))
            {
                fprintf(fp,"ID:%d 用户%s于%s连接到服务器, IP:%s:%d\n",new_client->cidnum,username,formatted_time,inet_ntoa(new_client->csock.sin_addr), ntohs(new_client->csock.sin_port));
                // 把用户名信息存储到链表结构体中
                if (new_client->user_status.username[0] == '\0')
                {
                    
                    strcat(new_client->user_status.username, username);
                    display(client_list);
                    flag--;
                }
                //解锁
                if(pthread_mutex_unlock(&mutex))
                {
                    perror("解锁失败!\n");
                }
                printf("解锁成功!\n");
                fclose(fp);
                continue;
            }
            
        }
        
        

        if (!strncmp(info, "quit", 4))
        {
            printf("客户端正在退出\n");
            fprintf(fp,"ID:%d 用户%s于%s退出服务器, IP:%s:%d\n",new_client->cidnum,username,formatted_time,inet_ntoa(new_client->csock.sin_addr), ntohs(new_client->csock.sin_port));
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
            //解锁
            if(pthread_mutex_unlock(&mutex))
            {
                perror("解锁失败!\n");
            }
            printf("解锁成功!\n");
            fclose(fp);
            break;
        }
        fprintf(fp,"CID:%d %s%s%s%s%s",new_client->cidnum, username, fuhao, formatted_time, fuhao, info);
        //解锁
        if(pthread_mutex_unlock(&mutex))
        {
            perror("解锁失败!\n");
        }
        printf("解锁成功!\n");
        
        fclose(fp);

        //群发
        if(group == 10086){
            // while(qun_list->next);
            cid_t *tmp = client_list->next;
            while(tmp != NULL){
                if(tmp != new_client && tmp->cidnum != new_client->cidnum){
                    send(tmp->cidnum, bufs, sizeof(bufs), 0);
                }
                tmp=tmp->next;

            }
            
        }
        //私发
        if (strncmp(conn, "f1rst", 5)){
            cid_t *tmp = client_list->next;
            int fs = 1;
            while(tmp != NULL){
                char *strname = tmp->user_status.username;
                if(tmp != new_client && !strcmp(conn,strname)){
                    send(tmp->cidnum, bufs, sizeof(bufs), 0);
                    fs=0;
                }
                tmp=tmp->next;
            }
            if(fs){
                char *fserr="对方未接受到你的信息！";
                send(new_client->cidnum,fserr,sizeof(fserr),0);
            }
        }

        
        
        
        printf("%s%s%s%s%s", username, fuhao, formatted_time, fuhao, info);
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
    client_list->cidnum=sevid;
    printf("socket create success:%d\n", sevid);

    // 2.绑定地址结构体
    struct sockaddr_in sevaddr = {0};
    sevaddr.sin_family = AF_INET;
    sevaddr.sin_addr.s_addr = inet_addr(ip);
    sevaddr.sin_port = htons(atoi(port));
    client_list->csock = sevaddr;


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
    // qun_list->chatnum=0;
    // qun_list->human=client_list;
    // qun_list->next=NULL;
    Tcp_init(ipstr, argv[1]);

    return 0;
}