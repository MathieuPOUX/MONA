#include <list>
#include <map>
#include <iterator>

namespace Mona {

template<typename Key, typename Value, typename Compare = std::less<Key>>
struct InsertionMap : virtual Object {
    using key_type        = Key;
    using mapped_type     = Value;
    using value_type      = std::pair<const Key, Value>;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare     = Compare;
    struct value_compare {
        key_compare comp;
        bool operator()(const value_type& lhs, const value_type& rhs) const {
            return comp(lhs.first, rhs.first);
        }
    };

    using iterator =  typename std::list<value_type>::iterator;
    using const_iterator = typename std::list<value_type>::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // CONSTRUCTORS & ASSIGNMENT
    InsertionMap() {}
    InsertionMap(const InsertionMap& other) {
        for (const auto& n : other._order) {
            insert(n.first, n.second);
        }
    }

    InsertionMap& operator=(const InsertionMap& other) {
        if (this != &other) {
            clear();
            for (const auto& n : other._order) {
                insert(n.first, n.second);
            }
        }
        return *this;
    }

    InsertionMap(InsertionMap&& other) noexcept {
        swap(other);
    }

    InsertionMap& operator=(InsertionMap&& other) noexcept {
        if (this != &other) {
            clear();
            swap(other);
        }
        return *this;
    }

    void swap(InsertionMap& other) noexcept {
        _order.swap(other._order);
        _map.swap(other._map);
    }

    // ELEMENT ACCESS
    Value& operator[](const Key& key) {
        auto it = _map.lower_bound(key);
        if (it == _map.end() || it->first != key) {
            _order.emplace_back(key, Value{});
            auto list_it = std::prev(_order.end());
			_map.emplace_hint(it, list_it->first, list_it);
			return list_it->second;
        }
        return it->second->second;
    }

    Value& operator[](Key&& key) {
        auto it = _map.lower_bound(key);
        if (it == _map.end() || it->first != key) {
            _order.emplace_back(std::move(key), Value{});
            auto list_it = std::prev(_order.end());
            _map.emplace_hint(it, list_it->first, list_it);
            return list_it->second;
        }
        return it->second->second;
    }

    Value& at(const Key& key) {
        auto it = _map.find(key);
        if (it == _map.end()) {
            throw std::out_of_range("Key not found");
        }
        return it->second->second;
    }

    const Value& at(const Key& key) const {
        auto it = _map.find(key);
        if (it == _map.end()) {
            throw std::out_of_range("Key not found");
        }
        return it->second->second;
    }

    // ITERATORS
    iterator begin() { return iterator(_order.begin()); }
    iterator end() { return iterator(_order.end()); }
    const_iterator begin() const { return const_iterator(_order.begin()); }
    const_iterator end() const { return const_iterator(_order.end()); }
    const_iterator cbegin() const { return const_iterator(_order.cbegin()); }
    const_iterator cend() const { return const_iterator(_order.cend()); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    // CAPACITY
    bool empty() const noexcept {
        return _order.empty();
    }

    size_type size() const noexcept {
        return _order.size();
    }

    // MODIFIERS
    void clear() {
        _order.clear();
        _map.clear();
    }

    std::pair<iterator, bool> insert(const std::pair<Key, Value>& kv) {
        return insert(kv.first, kv.second);
    }

    std::pair<iterator, bool> insert(const Key& k, const Value& v) {
         auto it = _map.lower_bound(k);
        if (it != _map.end() && it->first == k) {
            // Key exists, return old value
            return {iterator(it->second), false};
        }
		// New key
		_order.emplace_back(k, v);
		auto list_it = std::prev(_order.end());
		_map.emplace_hint(it, k, list_it);
		return {iterator(list_it), true};
    }

    std::pair<iterator, bool> insert_or_assign(const Key& k, Value&& v) {
        auto it = _map.lower_bound(k);
        if (it != _map.end() && it->first == k) {
            it->second->second = std::move(v);
            return {iterator(it->second), false};
        }
		_order.emplace_back(k, std::move(v));
		auto list_it = std::prev(_order.end());
		_map.emplace_hint(it, k, list_it);
		return {iterator(list_it), true};
    }

    template <class... Args>
	std::pair<iterator, bool> emplace(const Key& key, Args&&... args) {
        auto it = _map.lower_bound(key);
        if (it != _map.end() && it->first == key) {
            return { iterator(it->second), false };
        }
        _order.emplace_back(key, std::forward<Args>(args)...);
        auto list_it = std::prev(_order.end());
        _map.emplace_hint(it, list_it->first, list_it);
        return { iterator(list_it), true };
    }

    template <class... Args>
    iterator emplace_hint(iterator hint, Args&&... args) {
        // We don't use the hint for optimization, just ignore it.
        return emplace(std::forward<Args>(args)...).first;
    }

    void erase(const Key& k) {
        auto it = _map.find(k);
        if (it != _map.end()) {
            _order.erase(it->second);
            _map.erase(it);
        }
    }

    iterator erase(iterator pos) {
        if (pos == end()) return end();
        _map.erase(pos->first);
        auto next = std::next(pos);
        _order.erase(pos);
        return iterator(next);
    }

    // LOOKUP
    iterator find(const Key& k) {
        auto it = _map.find(k);
        if (it == _map.end()) {
            return end();
        }
        return iterator(it->second);
    }

    const_iterator find(const Key& k) const {
        auto it = _map.find(k);
        if (it == _map.end()) {
            return cend();
        }
        return const_iterator(it->second);
    }

    iterator lower_bound(const Key& k) {
        auto it = _map.lower_bound(k);
        if (it == _map.end()) return end();
        return iterator(it->second);
    }

    const_iterator lower_bound(const Key& k) const {
        auto it = _map.lower_bound(k);
        if (it == _map.end()) return cend();
        return const_iterator(it->second);
    }

    iterator upper_bound(const Key& k) {
        auto it = _map.upper_bound(k);
        if (it == _map.end()) return end();
        return iterator(it->second);
    }

    const_iterator upper_bound(const Key& k) const {
        auto it = _map.upper_bound(k);
        if (it == _map.end()) return cend();
        return const_iterator(it->second);
    }

    key_compare key_comp() const {
        return _map.key_comp();
    }

    value_compare value_comp() const {
        return value_compare{_map.key_comp()};
    }
private:
    // The list preserves insertion order.
    std::list<value_type> _order;
    // The map provides O(log n) lookup by key.
    std::map<Key, typename std::list<value_type>::iterator, Compare> _map;
};

} // namespace Mona
