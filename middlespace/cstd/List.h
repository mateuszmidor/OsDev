/**
 *   @file: List.h
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_SRC_LIST_H_
#define MIDDLESPACE_CSTD_SRC_LIST_H_

#include "types.h"

namespace cstd {
namespace details {
/**
 * @brief   List node that encapsulates "next" pointer and list item "data"
 */
template <class T>
class ListNode {
public:
    ListNode() : next(nullptr) {}
    ListNode(const T& data) : data(data), next(nullptr) {}
    T               data;
    ListNode<T>*    next;
};
}

/**
 * @brief   Iterator interface for List
 */
template <class T>
class ListIterator  {
    using ListItem = details::ListNode<T>;

public:
    ListIterator(ListItem* x) : current(x) {}

    ListIterator(const ListIterator& it) : current(it.current) {}

    ListIterator get_next() const {
        return ListIterator(current->next);
    }

    ListIterator& operator++() {
        if (current)
            current = current->next;
        return *this;
    }

    ListIterator operator++(int) {
        ListIterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const ListIterator& rhs) const {
        return current == rhs.current;
    }

    bool operator!=(const ListIterator& rhs) const {
        return current != rhs.current;
    }

    T& operator*() {
        return current->data;
    }

    const T& operator*() const {
        return current->data;
    }

private:
    ListItem* current   {};
};


/**
 * @brief   This class represents a forward list; simple implementation that fits our needs
 */
template <class T>
class List {
protected:
    using ListItem = details::ListNode<T>;
    ListItem*   m_head;
    u32         m_count;

public:
    List() {
        m_head = nullptr;
        m_count = 0;
    }

    List(const List& l) = delete;
    List& operator=(const List& l) = delete;

    List(List&& l) {
        this->m_count = l.m_count;
        l.m_count = 0;
        this->m_head = l.m_head;
        l.m_head = nullptr;
    }

    List& operator=(List&& l) {
        this->m_count = l.m_count;
        l.m_count = 0;
        this->m_head = l.m_head;
        l.m_head = nullptr;
        return *this;
    }

    virtual ~List() {
        ListItem* curr = m_head;

        while (curr) {
            ListItem* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    T& front() {
        return m_head->data;
    }

    const T& front() const {
        return m_head->data;
    }

    void push_front(const T& t) {
        ListItem* item = new ListItem();
        item->data = t;
        item->next = m_head;
        m_head = item;
        m_count++;
    }

    T pop_front() {
        if (!m_head)
            return {};

        auto first = begin();
        T data = *first;
        remove(first);
        return data;
    }

    const ListIterator<T> find(const T& value) const {
        ListItem* i = m_head;
        while (i)
            if (i->data == value)
                return ListIterator<T>(i);
            else
                i = i->next;

        return end();
    }

    void remove(const ListIterator<T>& it) {
        // nothing to remove
        if (!m_head)
            return;

        // case 1. removing head item
        if (m_head->data == *it) {
            ListItem* new_head = m_head->next;
            delete m_head;
            m_head = new_head;
            m_count--;
            return;
        }

        // case 2. removing middle or tail item
        ListItem* prev = m_head;
        ListItem* curr = m_head->next;

        while (curr) {
            if (curr->data == *it) {
                prev->next = curr->next;
                delete curr;
                m_count--;
                return;
            }
            else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    bool contains(const T& t) const {
        const ListItem* curr = m_head;

        while (curr)
            if (curr->data == t)
                return true;
            else
                curr = curr->next;

        return false;
    }

    u32 count() const {
        return m_count;
    }

    // iterator interface
    const ListIterator<T> begin() const {
        return ListIterator<T>(m_head);
    }

    const ListIterator<T> end() const {
        return ListIterator<T>(nullptr);
    }

};

} /* namespace cstd */

#endif /* MIDDLESPACE_CSTD_SRC_LIST_H_ */
