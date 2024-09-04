#ifndef _ACCOUNT_HPP
#define _ACCOUNT_HPP

#include "client.hpp"

class Account : private Client
{
private:
    int user_id;
    std::string user_name;
    std::string password;
    bool is_online;
    int user_name_length;
    int password_length;

public:
    Account() {}
    Account(const std::string &name, const std::string &pass, const int &id = -1) : user_name{name}, password{pass}, user_id{id}, is_online{false}, user_name_length{20}, password_length{20} {}
    int Register();                                  // 注册
    int Login();                                     // 登录
    int Logout();                                    // 注销账户
    int Check(int mode);                             // 是否已注册
    int Rename(const std::string &new_name);         // 修改用户名
    int Exit();                                      // 离线
    int ChangePassword(const std::string &new_pass); // 修改密码
    ~Account() {}
};

#endif