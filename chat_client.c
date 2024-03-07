#include "chat_head.h"
#include "log_reg.h"

int main(int argc, char const *argv[])
{
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

    // 登录
    printf("请输入(不超过20位)用户名:");
    char username[20]="root";
    // fgets(username, 20, stdin);
    // username[strcspn(username, "\n")] = '\0'; // 换行符替换为终止符

    printf("请输入(不超过20位)密码:");
    char password[20]="over";
    // fgets(password, 20, stdin);
    // password[strcspn(password, "\n")] = '\0'; // 换行符替换为终止符

    // 登录成功返回

    if (login_user(username, password) == -1)
    {
        return -3; // 登录失败，返回0则登录成功
    }

    // 初始化套接字，判断是否成功
    int clid = socket(AF_INET, SOCK_STREAM, 0);
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

    char bufs[128] = "";
    //------------------------------------------------------//
    cJSON *json_pointer = NULL;
    // 创建一个链表数据对象。
    json_pointer = cJSON_CreateObject();
    // 1添加字符串类型到节点当中 姓名
    cJSON_AddStringToObject(json_pointer, "name", username);
    //时间
    time_t tm1 = 0;

    // 添加字符串类型到节点当中
    cJSON_AddStringToObject(json_pointer, "fuhao", ">");
    //  添加群组类型到节点中
    cJSON_AddStringToObject(json_pointer, "group_id","10086");
    // 数据整理
    char *str = NULL;

    // 信息节点
    //第一次发送过去，请求更新节点,保存用户名
    cJSON_AddStringToObject(json_pointer, "conn", "f1rst");
    str = cJSON_Print(json_pointer);
    // puts(str);
    send(clid, str, strlen(str), 0);
    cJSON_DeleteItemFromObject(json_pointer, "conn");
    str = NULL;

    // 循环读写
    while (1)
    {
        bzero(bufs, sizeof(bufs));
        // 先写 - 从键盘读数据再写给对面
        read(STDIN_FILENO, bufs, sizeof(bufs) - 1);

        time_t tm2 = time(&tm1);
        cJSON_AddNumberToObject(json_pointer, "time",tm2);

        cJSON_AddStringToObject(json_pointer, "info", bufs);
        // puts(bufs);
        // jason格式转换成字符串格式
        str = cJSON_Print(json_pointer);
        send(clid, str, strlen(str), 0);
        // send(clid,bufs,strlen(bufs),0);
        if (strncmp(bufs, "quit", 4) == 0)
        {
            break;
        }
        // 发送过去之后，进行删除节点。
        cJSON_DeleteItemFromObject(json_pointer, "info");
        cJSON_DeleteItemFromObject(json_pointer, "time");
        
    }
    //
    shutdown(clid, SHUT_RDWR);
    cJSON_Delete(json_pointer);

    return 0;
}
