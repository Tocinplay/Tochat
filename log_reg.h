#ifndef __LOG_REG_H__
#define __LOG_REG_H__
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <unistd.h>
#include <string.h>


/*
 * users表结构：
 * - id: 主键，自增长的整数，用于唯一标识每个用户
 * - username: 用户名，唯一，用于登录
 * - password: 密码，用于登录
 * - nickname: 昵称，默认值为'Unknown'
 * - gender: 性别，默认值为'Unknown'
 * - age: 年龄，默认值为0
 * - signature: 个性签名，默认为'Unknown'
 */
sqlite3 *db; // sqlite3数据库判断用户账号密码

int init_db()
{   //新建用户数据库函数
    int rc = sqlite3_open("./dbdir/users.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "ERROR: can't open sqlite users.db:%s\n", sqlite3_errmsg(db));
        return -1;
    }

    const char *create_table_sql = 
    "CREATE TABLE IF NOT EXISTS users (\
        id INTEGER PRIMARY KEY,\
        username TEXT UNIQUE,\
        password TEXT,\
        nickname TEXT DEFAULT 'Unknown',\
        gender TEXT DEFAULT 'Unknown',\
        age INTEGER DEFAULT 0,\
        signature TEXT DEFAULT 'Unknown' )";

    char *error_message;
    rc = sqlite3_exec(db, create_table_sql, NULL, 0, &error_message);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "ERROR: can't create table users:%s\n", error_message);
        sqlite3_free(error_message);
        sqlite3_close(db);
        return -1;
    }
    return 0;
}
int login_user(const char *username, const char *password)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM users WHERE username = ? AND password = ?";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

        // 执行查询
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // 登录成功
            printf("Login successful!\n");
            return 0;
        } else {
            // 登录失败
            printf("Login failed. Invalid username or password.\n");
            return -1;
        }

        // 释放资源
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Error preparing SQL statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
}

int register_user(const char *username, const char *password)
{
    // 执行SQL插入
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

        // 执行插入
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            // 注册成功
            printf("Registration successful!\n");
            return 0;
        } else {
            // 注册失败
            printf("Registration failed. Username already exists.\n");
            return -1;
        }

        // 释放资源
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Error preparing SQL statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
}
#endif