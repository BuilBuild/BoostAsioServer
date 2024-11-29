/*
 * @Author: LeiJiulong
 * @Date: 2024-11-27 20:38:25
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-29 22:27:01
 * @Description:
 */
#pragma once
#include <iostream>
#include <boost/asio.hpp>

const int HEAD_TOTAL_LENGTH = 4;
const int HEAD_ID_LEN  = 2;
const int HEAD_DATA_LEN = 2;


class LogicSystem;

class MsgNode
{
public:
    MsgNode(short max_len) : total_len_(max_len), cur_len_(0)
    {
        data_ = new char[max_len]();
    }

    void clear()
    {
        memset(data_, 0, total_len_);
        cur_len_ = 0;
    }

    ~MsgNode()
    {
        std::cout << "destruct MsgNode " << std::endl;
    }
    short cur_len_;
    short total_len_;
    char *data_;
};

class RecvNode: public MsgNode
{
    friend class LogicSystem;
public:
    RecvNode(short max_len, short msg_id);
private:
    short msg_id_;
};

class SendNode : public MsgNode
{
    friend class LogicSystem;
public:
    SendNode(const char* msg, short max_len, short msg_id);
private:
    short msg_id_;
};