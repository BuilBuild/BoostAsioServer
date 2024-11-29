/*
 * @Author: LeiJiulong
 * @Date: 2024-11-27 20:54:23
 * @LastEditors: LeiJiulong && lei15557570906@outlook.com
 * @LastEditTime: 2024-11-29 22:26:17
 * @Description:
 */ 
#include "MsgNode.h"

RecvNode::RecvNode(short max_len, short msg_id): MsgNode(max_len), msg_id_(msg_id)
{
}

SendNode::SendNode(const char *msg, short max_len, short msg_id): MsgNode(max_len + HEAD_TOTAL_LENGTH),
msg_id_(msg_id)
{
    short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(data_, &msg_id_host, HEAD_ID_LEN);
    short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(data_ + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
    memcpy(data_ + HEAD_TOTAL_LENGTH, msg, max_len);
}
