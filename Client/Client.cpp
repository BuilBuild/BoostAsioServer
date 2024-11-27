/*
 * @Author: LeiJiulong
 * @Date: 2024-11-27 19:41:36
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-27 20:22:12
 * @Description: 
 */

#include<iostream>
#include<boost/asio.hpp>
#include<thread>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <string>

const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
const int HEAD_TOTAL = 4;

int main()
{
    try
    {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::endpoint remote_ep(boost::asio::ip::address::from_string("127.0.0.1"), 10086);
        boost::asio::ip::tcp::socket sock(ioc);
        boost::system::error_code ec;
        sock.connect(remote_ep, ec);
        if(ec)
        {
            std::cout << "connect failed, code is" << ec << std::endl;
            std::cout << "reason is: " << ec.what() << std::endl;
            return -1; 
        }
        Json::Value root;
        root["id"] = 1001;
        root["data"] = "hello world";
        std::string request = root.toStyledString();
        char send_data[MAX_LENGTH] = {0};
        short msgid = 1001;
        short msgid_host = boost::asio::detail::socket_ops::host_to_network_short(msgid);
        memcpy(send_data, &msgid_host, sizeof(short));
        // 转网络字节序
        short request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request.length());
        memcpy(send_data + sizeof(short), &request_host_length, sizeof(short));
        memcpy(send_data + 2*sizeof(short), request.c_str(), request.length());
        boost::asio::write(sock, boost::asio::buffer(send_data, request.length() + 2*sizeof(short)));
        std::cout << "begin to receive..." << std::endl;

        char reply_head[HEAD_TOTAL]={0};
        msgid = 0;
        std::size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL));
        memcpy(&msgid, reply_head, sizeof(short));
        short msglen = 0;
        memcpy(&msglen, reply_head+sizeof(short), sizeof(short));
        // 转为本地字节序
        msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);
        msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
        // 再次读取消息体
        char msg[MAX_LENGTH] = {0};
        std::size_t msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
        // 解析
        Json::Reader reader;
        reader.parse(std::string(msg, msg_length), root);
        std::cout << "msg id is " << root["id"] << "\nmsg data is: " << root["data"] << std::endl;
        getchar();
        

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    


    return 0;
}

