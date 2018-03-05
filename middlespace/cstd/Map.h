/**
 *   @file: Map.h
 *
 *   @date: Jan 3, 2018
 * @author: Mateusz Midor
 */

#ifndef MIDDLESPACE_CSTD_SRC_MAP_H_
#define MIDDLESPACE_CSTD_SRC_MAP_H_

#include <algorithm>
#include "Vector.h"

namespace cstd {

/**
 * @brief   Primitive and minimal Map with O(n) access time, to be replaced later
 */
template <class K, class V>
class Map {
    using KeyValue = std::pair<K, V>;
    using KeyValues = vector<KeyValue>;
    using Iterator = typename KeyValues::iterator;
    using ConstIterator = typename KeyValues::const_iterator;

public:
    V& operator[](const K& key) {
        auto it = find(key);
        if (it != key_values.end())
            return it->second;

        key_values.emplace_back(key, V{});
        return key_values.back().second;
    }

    Iterator find(const K& key) {
        auto pred = [&key] (const KeyValue& kv) { return kv.first == key;};
        return std::find_if(begin(), end(), pred);
    }

    ConstIterator find(const K& key) const {
        auto pred = [&key] (const KeyValue& kv) { return kv.first == key;};
        return std::find_if(cbegin(), cend(), pred);
    }

    Iterator find_by_val(const V& val) {
        auto pred = [&val] (const KeyValue& kv) { return kv.second == val;};
        return std::find_if(begin(), end(), pred);
    }

    void erase(Iterator it) {
        key_values.erase(it);
    }

    Iterator begin() { return key_values.begin(); }
    Iterator end() { return key_values.end(); }
    ConstIterator cbegin() const { return key_values.cbegin(); }
    ConstIterator cend() const { return key_values.cend(); }


private:
    KeyValues key_values;
};

} /* namespace cstd */

#endif /* MIDDLESPACE_CSTD_SRC_MAP_H_ */
