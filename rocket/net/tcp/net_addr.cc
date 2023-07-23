#include <string.h>

#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"

namespace rocket
{

IPNetAddr::IPNetAddr(const std::string& ip, uint16_t port):m_port(port), m_ip(ip){
    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;
    //将点分十进制的IPv4地址转换成网络字节序列的长整型
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    m_addr.sin_port = htons(m_port);
}

IPNetAddr::IPNetAddr(const std::string& addr){
    size_t i = addr.find_first_of(":");
    if(i == addr.npos) {
        ERRORLOG("invalid ipv4 addr %s", addr.c_str());
        return;
    }
    m_ip = addr.substr(0, i);
    //装换为整数
    m_port = std::atoi(addr.substr(i+1, addr.size() - i - 1).c_str());

    memset(&m_addr, 0, sizeof(m_addr));

    m_addr.sin_family = AF_INET;
    //将点分十进制的IPv4地址转换成网络字节序列的长整型
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    m_addr.sin_port = htons(m_port);
    
}

IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr){
    m_ip = inet_ntoa(m_addr.sin_addr);
    m_port = ntohs(m_addr.sin_port);
}

sockaddr* IPNetAddr::getSockAddr() {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

socklen_t IPNetAddr::getSockLen() {
    return sizeof(m_addr);
}

int IPNetAddr::getFamily() {
    return AF_INET;
}

std::string IPNetAddr::toString() {
    std::string re;
    re = m_ip + ":" + std::to_string(m_port);
    return re;
}

bool IPNetAddr::checkValid() {
    if(m_ip.empty())
        return false;
    if(m_port < 0 || m_port > 65536){
        return false;
    }
    //如果正确执行将返回一个无符号长整数型数。如果传入的字符串不是一个合法的IP地址，将返回INADDR_NONE
    if(inet_addr(m_ip.c_str()) == INADDR_NONE) {
        return false;
    }
    return true;
}
    
} // namespace rocket