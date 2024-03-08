#include "chat_head.h"
#include "log_reg.h"
// 信号量
sem_t sem;

// 信号处理函数
void sigintHandler(int signo) {
  // 获取时间
  time_t timeValue;
  time(&timeValue);
  struct tm *currentTime = localtime(&timeValue);

  // 显示时间
  printf("当前时间：%d-%d-%d %d:%d:%d\n",
    currentTime->tm_year + 1900, currentTime->tm_mon + 1, currentTime->tm_mday,
    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);

  // 释放信号量
  sem_post(&sem);
}
void Group_Msg_Read(const char *group_msg){

    char formatted_time[64];  //格式化时间
    time_t json_time_value;   //时间结构体

    cJSON *json_gp = cJSON_Parse(group_msg);
    cJSON *json_name = NULL;  //username
    cJSON *json_info = NULL;  //received information
    cJSON *json_fuhao = NULL;  //personal style sign
    cJSON *json_time = NULL;
    cJSON *json_group = NULL;

    json_name = cJSON_GetObjectItem(json_gp, "name");
    json_info = cJSON_GetObjectItem(json_gp, "info");
    json_time = cJSON_GetObjectItem(json_gp,"time");
    json_fuhao = cJSON_GetObjectItem(json_gp, "fuhao");
    json_group = cJSON_GetObjectItem(json_gp, "group_id");

    json_time_value = (time_t)json_time->valueint;
    bzero(formatted_time,sizeof(formatted_time));
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", localtime(&json_time_value));
    printf("%s%s%s%s%s", json_name->valuestring, json_fuhao->valuestring, formatted_time, json_fuhao->valuestring, json_info->valuestring);
    cJSON_Delete(json_gp);
}

char *Json_Msg_Send(char *username,char *fuhao,int group,char *conn,int time,char *info)
{
    char *str = (char*)malloc(2048);

    cJSON *json_pointer = NULL;
    json_pointer = cJSON_CreateObject();// 创建一个链表数据对象。
    cJSON_AddStringToObject(json_pointer, "name", username);    // 1添加字符串类型到节点当中 姓名
    cJSON_AddStringToObject(json_pointer, "fuhao", fuhao);    // 添加字符串类型到节点当中
    cJSON_AddNumberToObject(json_pointer, "group_id", group);    //  添加群组类型到节点中
    cJSON_AddStringToObject(json_pointer, "conn", conn);
    cJSON_AddNumberToObject(json_pointer, "time", time);
    cJSON_AddStringToObject(json_pointer, "info", info);

    str = cJSON_Print(json_pointer);

    return str; 
}

void showMenu() {
  printf("请选择操作：\n");
  printf("1. 群聊\n");
  printf("2. 私聊\n");
  printf("3. 接收文件\n");
  printf("4. 秀\n");
  printf("5. 退出程序\n");
}

int main(int argc, char const *argv[])
{
    // 初始化信号量
    sem_init(&sem, 0, 1);
    // 注册信号处理函数
    signal(SIGINT, sigintHandler);

    // 判断输入是否正确
    if (argc < 3)
    {
        perror("usage:./client + IP + port\n");
        return -1;
    }

    if (access("./dbdir/users.db", F_OK))
    { // 数据库不存在就创建
        if (!init_db())
        {
            printf("用户数据库创建成功\n");
        }
        else
        {
            printf("用户数据库创建失败\n");
            return -2;
        }
    }
    else
    {
        int rc = sqlite3_open("./dbdir/users.db", &db);
        if (rc)
        {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            return -2;
        }
    }
    int choice1;
    char username[20] = "";
    char password[20] = "";
    int count = 0;
    /*------------------------------------------*/

    char bufs[2048] = "";
    char bufrec[2048] = "";


    // 时间
    time_t tm1 = 0;
    time_t tm2 = time(&tm1);
    // 数据整理
    char *str = NULL;
    int qunnum = 10086;
    char yonghu[20] = {0};
    int clid;

    while (1) {
        // 显示菜单
        printf("请选择操作：\n");
        printf("1. 登录\n");
        printf("2. 注册\n");
        printf("3. 退出\n");

        // 获取用户输入
        scanf("%d", &choice1);
        getchar();

        switch (choice1) {
            case 1:
                    // 登录
                    printf("请输入(不超过20位)用户名:");
                    
                    fgets(username, 20, stdin);
                    username[strcspn(username, "\n")] = '\0'; // 换行符替换为终止符

                    printf("请输入(不超过20位)密码:");
                    
                    fgets(password, 20, stdin);
                    password[strcspn(password, "\n")] = '\0'; // 换行符替换为终止符

                    // 登录成功返回

                    if (login_user(username, password) == -1)
                    {
                        perror("登录失败");
                        return -3; // 登录失败，返回0则登录成功
                    }
                goto logtrue;
                break;
            case 2:
                // 注册
                printf("请输入(不超过20位)用户名:");
                    
                fgets(username, 20, stdin);
                username[strcspn(username, "\n")] = '\0'; // 换行符替换为终止符

                printf("请输入(不超过20位)密码:");
                
                fgets(password, 20, stdin);
                password[strcspn(password, "\n")] = '\0'; // 换行符替换为终止符
                int userCount = register_user(username, password);
                if(userCount == -1)
                {
                    printf("注册失败");
                }
                break;
            case 3:
                // 退出
                printf("正在退出...\n");
                exit(0);
            default:
                // 无效输入
                printf("无效输入，请重新选择操作：\n");
                break;
        }

    }
logtrue:

    // 初始化套接字，判断是否成功
    clid = socket(AF_INET, SOCK_STREAM, 0);
    if (clid == -1)
    {
        perror("socket create fail\n");
        return -4;
    }
    printf("socket create success\n");

    // connect
    struct sockaddr_in caddr = {0};
    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(atoi(argv[2]));
    caddr.sin_addr.s_addr = inet_addr(argv[1]);
    if (connect(clid, (struct sockaddr *)&caddr, sizeof(caddr)) < 0)
    {
        perror("connect error\n");
        close(clid);
        return -3;
    }
    printf("connect successful\n");

    // poll 多路复用
    struct pollfd pfds[2] = {0};
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;

    pfds[1].fd = clid;
    pfds[1].events = POLLIN;



    // 信息节点
    // --------------------------------------------------------------
    // 第一次发送过去，请求更新节点,保存用户名
    str = Json_Msg_Send(username,">",qunnum,"f1rst",tm2,"");
    // puts(str);
    send(clid, str, strlen(str), 0);
    str = NULL;
    // --------------------------------------------------------------
while(1){

    showMenu();
    int choice;
    // 获取用户输入
    scanf("%d", &choice);

    // 使用 switch 语句处理用户选择
    switch (choice) {
        case 1:
            // 群聊功能
            printf("请输入群号：");
            scanf("%d",&qunnum);
            goto link;
            break;
        case 2:
            // 私聊功能
            printf("请输入想要发给的用户名：");
            scanf("%s",yonghu);
            goto link;
            break;
        case 3:
            // 接收文件功能
            printf("接收文件功能暂未开发，敬请期待！\n");
            break;
        case 4:
            
            system("cmatrix");
            break;
        case 5:
            // 退出程序
            printf("正在退出程序...\n");
            exit(0);
            break;
        default:
            // 无效输入
            printf("无效输入，请重新选择操作：\n");
            break;
        }
    }

link:
    // 循环读写
    while (1)
    {
        count = poll(pfds, 2, 3000);



        //发送信息
        if (pfds[0].revents == POLLIN)
        {
            bzero(bufs, sizeof(bufs));
            // 先写 - 从键盘读数据再写给对面
            read(STDIN_FILENO, bufs, sizeof(bufs) - 1);

            tm2 = time(&tm1);
            
            // jason格式转换成字符串格式
            str = Json_Msg_Send(username,">",10086,"",tm2,bufs);
            send(clid, str, strlen(str), 0);
            if (strncmp(bufs, "quit", 4) == 0)
            {
                break;
            }
            
            count = 0; //发送完毕之后置零
        }

        //接收信息
        if (pfds[1].revents == POLLIN){
            bzero(bufrec,sizeof(bufrec));
            len = recv(clid, bufrec, sizeof(bufrec), 0);
            Group_Msg_Read(bufrec);
            
        }
    }
    //
    shutdown(clid, SHUT_RDWR);
    // cJSON_Delete(json_pointer);
    // 销毁信号量
    sem_destroy(&sem);
    return 0;
}
