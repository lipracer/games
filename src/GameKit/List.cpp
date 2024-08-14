#include "GameKit/List.h"

namespace games
{

CList* create_list()
{
    auto list = new CList();
    list->data = nullptr;
    list->next = nullptr;
    return list;
}

void destory_list(CList* list)
{
    while (list)
    {
        auto tmp = list->next;
        delete list;
        list = tmp;
    }
}

} // namespace games
