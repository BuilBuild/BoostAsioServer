/*
 * @Author: LeiJiulong
 * @Date: 2024-11-29 21:04:25
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-29 22:49:20
 * @Description:
 */
#include "Session.h"

Session::Session(boost::asio::io_context &ioc, Server *server) : socket_(ioc), server_(server), b_close_(false), b_head_parse(false)
{
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(a_uuid);
    recv_head_node_ = std::make_shared<MsgNode>(HEAD_DATA_LEN);
}

Session::~Session()
{
    std::cout << "Session " << uuid_ << " is destruct" << std::endl;
}

boost::asio::ip::tcp::socket &Session::getSocket()
{
    return socket_;
}

const std::string &Session::getUuid()
{
    return uuid_;
}

void Session::start()
{
    memset(data_, 0, MAX_LENGTH);
    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                            std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
}

void Session::send(const char *const msg, short max_length, short msgid)
{
    std::lock_guard<std::mutex> lock(send_mtx_);
    int send_que_size = send_queue_.size();
    if (send_que_size > MAX_SENDQUE)
    {
        std::cout << "session: " << uuid_ << " send que fulled, size is " << MAX_SENDQUE << std::endl;
        return;
    }
    send_queue_.push(std::make_shared<SendNode>(msg, max_length, msgid));
    if (send_que_size > 0)
    {
        return;
    }
    auto &msgnode = send_queue_.front();
    socket_.async_send(boost::asio::buffer(msgnode->data_, msgnode->total_len_),
                       std::bind(&Session::handleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::send(const std::string msg, short msgid)
{
    std::lock_guard<std::mutex> lock(send_mtx_);
    int send_que_size = send_queue_.size();
    if (send_que_size > MAX_SENDQUE)
    {
        std::cout << "session: " << uuid_ << " send que fulled, size is " << MAX_SENDQUE << std::endl;
        return;
    }
    send_queue_.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
    if (send_que_size > 0)
    {
        return;
    }
    auto &msgnode = send_queue_.front();
    socket_.async_send(boost::asio::buffer(msgnode->data_, msgnode->total_len_),
                       std::bind(&Session::handleWrite, this, std::placeholders::_1, shared_from_this()));
}

void Session::close()
{
    socket_.close();
    b_close_ = true;
}

std::shared_ptr<Session> Session::sharedSelf()
{
    return std::shared_ptr<Session>();
}

void Session::handleRead(const boost::system::error_code &ec, std::size_t bytes_transferred, std::shared_ptr<Session> shared_self)
{
    try
    {
        if (!ec)
        {
            int copy_len = 0;
            while (bytes_transferred > 0)
            {
                if (!b_head_parse)
                {
                    if (bytes_transferred + recv_head_node_->cur_len_ < HEAD_TOTAL_LENGTH)
                    {
                        memcpy(recv_head_node_->data_ + recv_head_node_->cur_len_, data_ + copy_len, bytes_transferred);
                        recv_head_node_->cur_len_ += bytes_transferred;
                        memset(data_, 0, MAX_LENGTH);
                        socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                                std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
                        return;
                    }

                    int head_remain = HEAD_TOTAL_LENGTH - recv_head_node_->cur_len_;
                    memcpy(recv_head_node_->data_ + recv_head_node_->cur_len_, data_ + copy_len, head_remain);
                    copy_len += head_remain;
                    bytes_transferred -= head_remain;
                    short msg_id = 0;
                    memcpy(&msg_id, recv_msg_node_->data_, HEAD_ID_LEN);
                    msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                    std::cout << "msg_id is " << msg_id << std::endl;
                    // 验证id 是否有效
                    if (msg_id > MAX_LENGTH)
                    {
                        std::cout << "invalid msg_id is " << msg_id << std::endl;
                        server_->clearSession(uuid_);
                        return;
                    }
                    short msg_len = 0;
                    memcpy(&msg_len, recv_head_node_->data_ + HEAD_ID_LEN, HEAD_DATA_LEN);
                    msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                    // 验证数据长度是否有效
                    if (msg_len > MAX_LENGTH)
                    {
                        std::cout << "invalid length of data is " << msg_len << std::endl;
                        server_->clearSession(uuid_);
                        return;
                    }
                    recv_msg_node_ = std::make_shared<RecvNode>(msg_len, msg_id);
                    if (bytes_transferred < msg_len)
                    {
                        memcpy(recv_msg_node_->data_ + recv_head_node_->cur_len_, data_ + copy_len, bytes_transferred);
                        recv_head_node_->cur_len_ += bytes_transferred;
                        memset(data_, 0, MAX_LENGTH);
                        socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                                std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
                        b_head_parse = true;
                    }

                    memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, msg_len);
                    recv_msg_node_->cur_len_ += msg_len;
                    bytes_transferred -= msg_len;
                    // recv_msg_node_ ->data_[]
                    /*
                    此处用逻辑队列处理

                    */

                    // 继续轮询剩余未处理数据
                    b_head_parse = false;
                    recv_head_node_->clear();
                    if (bytes_transferred <= 0)
                    {
                        memset(data_, 0, MAX_LENGTH);
                        socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                                std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
                        return;
                    }
                    continue;
                }

                // 已经处理完头部， 接收的数据仍不足，处理剩余数据
                int remain_msg = recv_msg_node_->total_len_ - recv_msg_node_->cur_len_;
                if (bytes_transferred < remain_msg)
                {
                    memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, bytes_transferred);
                    recv_msg_node_->cur_len_ += bytes_transferred;
                    memset(data_, 0, MAX_LENGTH);
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                            std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
                    return;
                }
                memcpy(recv_head_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, remain_msg);
                recv_msg_node_->cur_len_ += remain_msg;
                bytes_transferred -= remain_msg;
                copy_len += remain_msg;

                /*
                逻辑队列投递
                */

                // 继续轮询处理剩余未处理数据
                b_head_parse = false;
                recv_head_node_->clear();
                if (bytes_transferred <= 0)
                {Sessio
                    memcpy(data_, 0, MAX_LENGTH);
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                            std::bind(&Session::handleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));
                    return;
                }
                continue;
            }
        }
        else
        {
            std::cout << "heandle read failed, error is " << ec.what() << std::endl;
            close();
            server_->clearSession(uuid_);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Session::handleWrite(const boost::system::error_code &ec, std::shared_ptr<Session> shared_self)
{
    try
    {
        if (!ec)
        {
            std::lock_guard<std::mutex> lock(send_mtx_);
            send_queue_.pop();
            if (!send_queue_.empty())
            {
                auto &msgnode = send_queue_.front();
                socket_.async_send(boost::asio::buffer(msgnode->data_, msgnode->total_len_),
                                   std::bind(&Session::handleWrite, this, std::placeholders::_1, shared_from_this()));
            }
            else
            {
                close();
                server_->clearSession(uuid_);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

LogicNode::LogicNode(std::shared_ptr<Session> session, std::shared_ptr<RecvNode> recv_node) : session_(session), recv_node_(recv_node)
{
}
