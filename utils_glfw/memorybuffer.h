#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include <algorithm>

class MemoryBuffer
{
    HANDLE m_hMap;
    std::string m_name;
    unsigned int m_size;

public:
    class MemoryView
    {
        HANDLE m_hMap;
        unsigned int m_size;
        unsigned char *m_head;
        unsigned char *m_current;

        explicit MemoryView(const MemoryView &);
        MemoryView& operator=(const MemoryView &);
    public:
        explicit MemoryView(HANDLE hMap, unsigned int bytesize);
        ~MemoryView();
        unsigned char* GetPointer(){ 
            //if(!m_head){
			{
                Open();
            }
            return m_head; 
        }
        void Open();
        void Close();
        template<typename T>
            void Write(T n)
            {
                if(!m_current){
                    Open();
                }
                if(!m_current){
                    return;
                }
                *((T*)m_current)=n;
                Flush(sizeof(T));
            }

        void Write(const unsigned char *p, unsigned int size)
        {
            if(!m_current){
                Open();
            }
            if(!m_current){
                return;
            }
            std::copy(p, p+size, m_current);
            Flush(size);
        }

    private:
        void Flush(unsigned int n)
        {
            FlushViewOfFile(m_current, n);
            //m_current+=n;
        }
    };

private:
    std::unique_ptr<MemoryView> m_view;

public:
    MemoryBuffer(const std::string &name, unsigned int size=0);
    ~MemoryBuffer();
	void Write(const unsigned char *p, unsigned int bytesize);
    unsigned char *GetPointer();
};

