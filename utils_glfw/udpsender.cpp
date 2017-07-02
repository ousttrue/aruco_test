#include <winsock2.h>
#pragma comment(lib,"wsock32.lib")
#include "udpsender.h"
#include <vector>

/*
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
*/


template<typename T>
static void printMatrix(const T *m)
{
    std::cout 
        << "[" << m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << "]" << std::endl
        << "[" << m[4] << ", " << m[5] << ", " << m[6] << ", " << m[7] << "]" << std::endl
        << "[" << m[8] << ", " << m[9] << ", " << m[10] << ", " << m[11] << "]" << std::endl
        << "[" << m[12] << ", " << m[13] << ", " << m[14] << ", " << m[15] << "]" << std::endl
        << std::endl
        ;
}


static void sendUDP(const char *buf, int len, const char* ip, int port)
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip);

    sendto(sock, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));

    closesocket(sock);
}


class BinaryBuffer
{
    std::vector<char> m_buf;
    size_t m_pos;
public:
    BinaryBuffer()
        : m_pos(0)
    {
    }
    ~BinaryBuffer()
    {
    }

    template<typename T>
        void write(T t)
        {
            for(int i=0; i<sizeof(T); ++i){
                m_buf.push_back(0);
            }
            T *p=(T*)&m_buf[m_pos];
            *p=t;
            m_pos+=sizeof(T);
        }

    template<typename T>
        void write(const T *p, size_t count)
        {
            for(int i=0; i<sizeof(T)*count; ++i){
                m_buf.push_back(0);
            }
            std::copy(p, p+count, (T*)&m_buf[m_pos]);
            m_pos+=sizeof(T)*count;
        }

    char *pointer(){
        if(m_buf.empty()){
            return 0;
        }
        return &m_buf[0];
    }

    size_t size(){ return m_buf.size(); }
};


void sendMatrix(const float m[16])
{
    BinaryBuffer buf;
    buf.write<DWORD>(0x39303930);

    // count
    buf.write<DWORD>(1);

    // id
    buf.write<DWORD>(1);

    const float factor=10.0f;

    //auto m4=glm::make_mat4(m);

    // scale only
    float f[16]={
        m[0], m[1], m[2], m[3],
        m[4], m[5], m[6], m[7],
        m[8], m[9], m[10], m[11],

        m[12] * factor, 
        m[13] * factor, 
        m[14] * factor, 
        m[15]
    };
    buf.write<float>(f, 16);

	//printMatrix(f);

    if(buf.size()==0){
        return;
    }

    sendUDP(buf.pointer(), buf.size(), "127.0.0.1", 2532);
}

