#pragma once

#include <cstddef>

namespace games
{

template <typename T>
struct ListNode
{
    ListNode* prev;
    ListNode* next;
    T data;
};

template <typename T>
struct List
{
    size_t size() const
    {
        return size_;
    }

private:
    size_t size_;
};

struct CListNode
{
    void* data;
    CListNode* next;
};

typedef CListNode CList;

CList* create_list();
void destory_list(CList* list);

void append_list(void* data);

} // namespace games
