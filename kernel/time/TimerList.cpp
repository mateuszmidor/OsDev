/**
 *   @file: TimerList.cpp
 *
 *   @date: Nov 14, 2017
 * @author: Mateusz Midor
 */

#include "Timer.h"
#include "TimerList.h"

namespace ktime {

/**
 * @brief   Insert timer "t" into the list in a sorted manner
 */
void TimerList::push_sorted_ascending_by_expire_time(Timer* t) {
    ListItem* new_item = new ListItem(t);

    // case 1. adding head item
    if (!m_head) {
        m_head = new_item;
        m_count++;
        return;
    }

    // case 2. adding middle item
    ListItem* prev = m_head;
    ListItem* curr = m_head->next;

    while (curr) {
        if (curr->data->remaining_ticks > t->remaining_ticks) { // current expires after "t"
            prev->next = new_item;
            new_item->next = curr;
            m_count++;
            return;
        }
        else {
            prev = curr;
            curr = curr->next;
        }
    }

    // case 3. adding tail item
    prev->next = new_item;
    m_count++;
}

/**
 * @brief   Get timer of given tid
 */
Timer* TimerList::get_by_tid(u32 timer_id) {
    for (Timer* t : *this)
        if (t->timer_id == timer_id)
            return t;

    return nullptr;
}

} /* namespace time */
