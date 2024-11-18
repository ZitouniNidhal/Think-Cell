#include <iostream>
#include <map>
#include <utility>
#include <type_traits>

template<typename K, typename V>
class interval_map {
    friend void IntervalMapTest();
    V m_valBegin;
    std::map<K, V> m_map;

public:
    // Constructor associates whole range of K with val
    template<typename V_forward>
    interval_map(V_forward&& val)
    : m_valBegin(std::forward<V_forward>(val))
    {}

    // Assign value val to interval [keyBegin, keyEnd).
    // Overwrite previous values in this interval.
    // If !(keyBegin < keyEnd), this designates an empty interval, and assign must do nothing.
    template<typename V_forward>
    void assign(K const& keyBegin, K const& keyEnd, V_forward&& val)
        requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value)
    {
        // Handle empty interval case
        if (!(keyBegin < keyEnd)) return;

        // Find the position of keyBegin and keyEnd
        auto itLow = m_map.lower_bound(keyBegin);
        auto itHigh = m_map.lower_bound(keyEnd);

        // Value before keyBegin
        V valueAtBegin = (itLow == m_map.begin()) ? m_valBegin : std::prev(itLow)->second;

        // If value at keyBegin is the same as val, we don't need to insert it
        if (valueAtBegin == val) {
            if (itLow != m_map.end() && itLow->first == keyBegin) {
                // Remove redundant entry at keyBegin
                m_map.erase(itLow);
            }
        } else {
            // Insert or update keyBegin with val
            auto result = m_map.insert_or_assign(keyBegin, std::forward<V_forward>(val));
            itLow = result.first;  // Get the iterator from insert_or_assign
        }

        // Erase the range between keyBegin and keyEnd (exclusive)
        m_map.erase(itLow, itHigh);

        // Handle keyEnd: assign value if necessary
        if (itHigh != m_map.end() && itHigh->first == keyEnd) {
            if (itHigh->second == val) {
                // Remove redundant entry at keyEnd
                m_map.erase(itHigh);
            }
        } else {
            // Restore the value at keyEnd
            m_map[keyEnd] = valueAtBegin;
        }

        // Merge adjacent intervals if they have the same value
        if (itLow != m_map.begin() && std::prev(itLow)->second == itLow->second) {
            m_map.erase(std::prev(itLow));
        }
    }

    // Lookup the value associated with a key
    V const& operator[](K const& key) const {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin()) {
            return m_valBegin;
        } else {
            return (--it)->second;
        }
    }
};

int main() {
    interval_map<int, char> imap('A');
    imap.assign(1, 3, 'B');
    imap.assign(2, 4, 'C');
    
    // Expected values
    std::cout << imap[0] << " ";  // A
    std::cout << imap[1] << " ";  // B
    std::cout << imap[2] << " ";  // C
    std::cout << imap[3] << " ";  // C
    std::cout << imap[4] << " ";  // A
    
    return 0;
}
