#include "account.hpp"

#include <iostream>
#include <memory>
#include <ctime>
#include <cstring>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

int Account::Check(int mode = 0)
{
    int status = SUCCESS;
    if (mode == 0)
    {
        // 全部检查
        string sql_query = "select count(*) from user_info where user_id=" + std::to_string(user_id) + ";";
        if (mysql_query(database, sql_query.c_str()))
        {
            cerr << "ERROR: Failed to check.\n";
            status = ERROR;
        }

        MYSQL_RES *res = mysql_store_result(database);
        MYSQL_ROW row = mysql_fetch_row(res);

        if (row || row[0])
        {
            cerr << "ERROR: Failed to fetch row of the number of user_id.\n";
            status = ERROR;
        }
        if (atoi(row[0]) != 0)
            status = INVALID;

        if (user_name.length() > user_name_length)
            status = INVALID_USER_NAME;

        if (password.length() > password_length)
            status = INVALID_PASSWORD;
    }
    else if (mode == 1)
    { // 查用户名是否过长
        if (user_name.length() > user_name_length)
            status = INVALID_USER_NAME;
    }
    else if (mode == 2)
    { // 查密码是否过长
        if (password.length() > password_length)
            status = INVALID_PASSWORD;
    }

    return status;
}

int Account::Register()
{
    int status = SUCCESS;
    int flag = Check(0);
    if (flag != SUCCESS)
        status = flag;
    else
    {
        user_id = max_user_id + 1;
        string sql_query = "insert into user_info (user_id,user_name,password)"
                           "values(" +
                           std::to_string(user_id) + user_name + password + ");";
        if (mysql_query(database, sql_query.c_str()))
        {
            cerr << "ERROR: Failed to register.\n";
            user_id = -1;
            status = ERROR;
        }
        ++max_user_id;
    }

    return status;
}

int Account::Login()
{
    int status = SUCCESS;
    string sql_query = "select password from user_info where user_id=" + std::to_string(user_id) + ";";
    if (mysql_query(database, sql_query.c_str()))
    {
        cerr << "ERROR: Failed to login.\n";
        status = ERROR;
    }
    MYSQL_RES *res = mysql_store_result(database);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (row || row[0])
    {
        cerr << "ERROR: Failed to fetch row of the password.\n";
        status = ERROR;
    }
    if (password != string{row[0]})
        status = INVALID;

    if (status == SUCCESS)
        is_online = true;

    return status;
}

int Account::Logout()
{

    int status = SUCCESS;
    if (!is_online)
        status = INVALID;
    else
    {
        string sql_query = "delete from user_info where user_id=" + std::to_string(user_id) + ";";
        if (mysql_query(database, sql_query.c_str()))
        {
            cerr << "ERROR: Failed to logout.\n";
            status = ERROR;
        }
    }
    return status;
}

int Account::Exit()
{
    is_online = false;
    return SUCCESS;
}

int Account::Rename(const string &new_name)
{
    int status = SUCCESS;
    user_name = new_name;
    int flag = Check(1);
    if (flag != SUCCESS)
        status = flag;
    else
    {
        string sql_query = "update user_info set user_name='" + user_name + "' where user_id=" + std::to_string(user_id) + ";";
        if (mysql_query(database, sql_query.c_str()))
        {
            cerr << "ERROR: Failed to rename.\n";
            status = ERROR;
        }
    }

    return status;
}

int Account::ChangePassword(const string &new_pass)
{
    int status = SUCCESS;
    password = new_pass;
    int flag = Check(2);
    if (flag != SUCCESS)
        status = flag;
    else
    {
        string sql_query = "update user_info set password='" + password + "' where user_id=" + std::to_string(user_id) + ";";
        if (mysql_query(database, sql_query.c_str()))
        {
            cerr << "ERROR: Failed to change password.\n";
            status = ERROR;
        }
    }
    return status;
}