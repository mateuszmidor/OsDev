/**
 *   @file: List.h
 *
 *   @date: Nov 10, 2017
 * @author: Mateusz Midor
 */

#ifndef KERNEL_KSTD_LIST_H_
#define KERNEL_KSTD_LIST_H_

#include "types.h"

namespace kstd {

/**
 * @brief   List node that encapsulates "next" pointer and list item "data"
 */
template <class T>
class ListNode {
public:
    T               data  {};
    ListNode<T>*    next  {};
};

/**
 * @brief   Iterator interface for List
 */
template <class T>
class ListIterator  {
    using ListItem = ListNode<T>;

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

private:
    ListItem* current   {};
};


/**
 * @brief   This class represents a forward list; simple implementation that fits our needs
 */
template <class T>
class List {
    using ListItem = ListNode<T>;

public:
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

        T data = m_head->data;
        remove(m_head->data);
        return data;
    }

    void remove(const T& t) {
        // nothing to remove
        if (!m_head)
            return;

        // case 1. removing head item
        if (m_head->data == t) {
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
            if (curr->data == t) {
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

private:
    ListItem*   m_head    = nullptr;
    u32         m_count   = 0;
};

} /* namespace kstd */

#endif /* KERNEL_KSTD_LIST_H_ */
