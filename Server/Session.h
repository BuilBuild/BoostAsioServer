/*
 * @Author: LeiJiulong
 * @Date: 2024-11-29 20:46:15
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-29 22:49:15
 * @Description: 
 */
#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>
#include <memory>
#include <string>
#include "MsgNode.h"


const int MAX_LENGTH = 1024*2;
const int MAX_SENDQUE = 1000;
class Server;
class LogicSystem;

class Session: public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &ioc, Server *Server);
    ~Session();
    boost::asio::ip::tcp::socket& getSocket();
    const std::string & getUuid();

    void start();
    void send(const char* const msg, short max_length, short msgid);
    void send(const std::string msg, short msgid);
    void close();
    std::shared_ptr<Session> sharedSelf();

private:
    void handleRead(const boost::system::error_code &ec, std::size_t bytes_transferred,std::shared_ptr<Session> shared_self);
    void handleWrite(const boost::system::error_code &ec, std::shared_ptr<Session> shared_self);
    boost::asio::ip::tcp::socket socket_;
    std::string uuid_;
    char data_[MAX_LENGTH];
    Server *server_;
    bool b_close_;
    std::queue<std::shared_ptr<SendNode>> send_queue_;
    std::mutex send_mtx_;
    std::shared_ptr<RecvNode> recv_msg_node_;
    bool b_head_parse;
    std::shared_ptr<MsgNode> recv_head_node_;

};

class LogicNode
{
    friend class LogicSystem;
public:
    LogicNode(std::shared_ptr<Session>, std::shared_ptr<RecvNode>);
private:
    std::shared_ptr<Session> session_;
    std::shared_ptr<RecvNode> recv_node_;

};