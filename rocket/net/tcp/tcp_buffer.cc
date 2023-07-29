#include <memory>
#include <string.h>
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/common/log.h"
namespace rocket
{
    TcpBuffer::TcpBuffer(int size) : m_size(size) {
        m_buffer.resize(size);
    }

    TcpBuffer::~TcpBuffer(){
        
    }

    //返回可读字节数
    int TcpBuffer::readAble(){
        return m_write_index - m_read_index;
    }

    //返回可写字节数
    int TcpBuffer::writeAble(){
        return m_buffer.size() - m_write_index;
    }

    int TcpBuffer::readIndex(){
        return m_read_index;
    }

    int TcpBuffer::writeIndex(){
        return m_write_index;
    }

    void TcpBuffer::writeToBuffer(const char* buf, int size){
        if(size > writeAble()){
            //调整buffer的大小，扩容
            int new_size = (int)(1.5 * (m_read_index + size));
            resizeBuffer(new_size);
        }
        memcpy(&m_buffer[m_write_index], buf, size);
        m_write_index += size;
    }


    void TcpBuffer::readFromBuffer(std::vector<char>& re, int size){
        if(readAble() == 0) return;
        int read_size = readAble() > size ? size : readAble();

        std::vector<char> tmp(read_size);
        memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

        re.swap(tmp);
        m_read_index += read_size;

        AdjustBuffer();
    }
    

    void TcpBuffer::resizeBuffer(int new_size){
        std::vector<char> tmp(new_size);
        int count = std::min(new_size, readAble());
        memcpy(&tmp[0], &m_buffer[m_read_index], count);
        m_buffer.swap(tmp);

        m_read_index = 0;
        m_write_index = m_read_index + count;
    }

    //回收调整(调整时机：read后、read_index改变后)
    void TcpBuffer::AdjustBuffer(){
        if(m_read_index < (int)(m_buffer.size() / 3)){
            return;
        }
        std::vector<char> buffer(m_buffer.size());
        int count = readAble();

        memcpy(&buffer[0], &m_buffer[m_read_index], count);
        m_buffer.swap(buffer);
        m_read_index = 0;
        m_write_index = m_read_index + count;

        buffer.clear();
    }

    void TcpBuffer::moveReadIndex(int size){
        int j = m_read_index + size;
        if(j >= m_buffer.size()){
            ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
            return;
        }
        m_read_index = j;
        AdjustBuffer();
    }

    void TcpBuffer::moveWriteIndex(int size){
        int j = m_write_index + size;
        if(j >= m_buffer.size()){
            ERRORLOG("moveWriteIndex error, invalid size %d, old_Write_index %d, buffer size %d", size, m_write_index, m_buffer.size());
            return;
        }
        m_write_index = j;
        //AdjustBuffer();
    }
} // namespace rocket