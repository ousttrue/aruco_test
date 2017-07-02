#include "memorybuffer.h"


/////////////////////////////////////////////////////////////////////////////
MemoryBuffer::MemoryView::MemoryView(HANDLE hMap, unsigned int bytesize)
: m_hMap(hMap), m_size(bytesize), m_head(0), m_current(0)
{
    Open();
}

MemoryBuffer::MemoryView::~MemoryView()
{
    Close();
}

void MemoryBuffer::MemoryView::Open()
{
    if(m_head){
        Close();
    }
    if(!m_hMap){
        return;
    }
    m_head=(unsigned char *)MapViewOfFile(m_hMap, 
            m_size ?  FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 
            0, 0, m_size);
    if(!m_head){
        return;
    }
    m_current=m_head;
}

void MemoryBuffer::MemoryView::Close()
{
    if(m_head){
        UnmapViewOfFile(m_head);
        m_head=0;
        m_current=0;
    }
}


/////////////////////////////////////////////////////////////////////////////
MemoryBuffer::MemoryBuffer(const std::string &name, unsigned int size)
: m_hMap(0), m_name(name), m_size(size)
{
    m_hMap=OpenFileMapping(FILE_MAP_READ, FALSE, name.c_str());
    if(!m_hMap){
        m_hMap=CreateFileMapping(INVALID_HANDLE_VALUE,
                NULL,
                PAGE_READWRITE,
                0,
                size,
                name.c_str());
    }
    if(!m_hMap){
        return;
    }
    m_size=size;
    m_name=name;

    m_view.reset(new MemoryView(m_hMap, m_size));
}

MemoryBuffer::~MemoryBuffer()
{
    m_view=nullptr;

    if(m_hMap){
        CloseHandle(m_hMap);
        m_hMap=0;
    }
}

void MemoryBuffer::Write(const unsigned char *p, unsigned int bytesize)
{
	if(!m_view){
		return;
	}
    m_view->Write(p, bytesize);
}

unsigned char *MemoryBuffer::GetPointer()
{
	if(!m_view){
		return nullptr;
	}
    return m_view->GetPointer();
}
