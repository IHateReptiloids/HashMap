#include <list>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cmath>

static const size_t init_size = 16;
static const double max_ratio = 0.5;
static const double min_ratio = 0.1;
static const int grow_factor = 2;

template<typename K, typename V, typename Hasher = std::hash<K>>
class HashMap {
public:
    typedef typename std::list<std::pair<const K, V>>::iterator iterator;
    typedef typename std::list<std::pair<const K, V>>::const_iterator const_iterator;

private:
    size_t sz;
    std::list<std::pair<const K, V>> data;
    std::vector<iterator> pointers;
    Hasher hasher;

    void reallocate(size_t new_size) {
        std::list<std::pair<const K, V>> old_data(data);
        data.clear();
        pointers.assign(new_size, data.end());
        for (const auto &i : old_data) {
            size_t hash = hasher(i.first) % new_size;
            data.insert(pointers[hash], i);
            pointers[hash]--;
        }
    }

    void __insert(std::pair<K, V> elem) {
        sz++;
        size_t hash = hasher(elem.first) % pointers.size();
        data.insert(pointers[hash], elem);
        pointers[hash]--;
        if (sz / max_ratio > pointers.size()) {
            reallocate(pointers.size() * grow_factor);
        }
    }

public:
    HashMap(Hasher hash = Hasher())
        : sz(0)
        , pointers(init_size, data.end())
        , hasher(hash) {}

    template<typename Iter>
    HashMap(Iter begin, Iter end, Hasher hash = Hasher())
        : sz(std::distance(begin, end))
        , data(begin, end)
        , hasher(hash) {
        reallocate(static_cast<size_t>(ceil(sz / max_ratio + 1)));
    }

    HashMap(std::initializer_list<std::pair<K, V>> init_list, Hasher hash = Hasher())
        : HashMap(init_list.begin(), init_list.end(), hash) {}

    HashMap(const HashMap &map)
        : HashMap(map.begin(), map.end(), map.hash_function()) {}

    HashMap& operator=(const HashMap &map) {
        sz = map.sz;
        data = std::list<std::pair<const K, V>>(map.begin(), map.end());
        hasher = map.hasher;
        reallocate(map.pointers.size());
        return *this;
    }

    size_t size() const {
        return data.size();
    }

    bool empty() const {
        return data.empty();
    }

    Hasher hash_function() const {
        return hasher;
    }

    iterator find(K key) {
        size_t hash = hasher(key) % pointers.size();
        auto iter = pointers[hash];
        while (iter != data.end() && hasher(iter->first) % pointers.size() == hash) {
            if (iter->first == key) {
                return iter;
            }
            iter++;
        }
        return data.end();
    }

    const_iterator find(K key) const {
        size_t hash = hasher(key) % pointers.size();
        auto iter = pointers[hash];
        while (iter != data.end() && hasher(iter->first) % pointers.size() == hash) {
            if (iter->first == key) {
                return iter;
            }
            iter++;
        }
        return data.end();
    }

    void insert(std::pair<K, V> elem) {
        if (find(elem.first) == data.end()) {
            __insert(elem);
        }
    }

    void erase(K key) {
        auto iter = find(key);
        if (iter == data.end()) {
            return;
        }
        sz--;
        size_t hash = hasher(key) % pointers.size();
        if (pointers[hash] == iter) {
            pointers[hash]++;
            if (pointers[hash] == data.end() || hasher(pointers[hash]->first) % pointers.size() != hash) {
                pointers[hash] = data.end();
            }
        }
        data.erase(iter);
        if (sz / min_ratio < pointers.size()) {
            reallocate(pointers.size() / grow_factor);
        }
    }

    iterator begin() {
        return data.begin();
    }

    const_iterator begin() const {
        return data.cbegin();
    }

    iterator end() {
        return data.end();
    }

    const_iterator end() const {
        return data.cend();
    }

    void clear() {
        sz = 0;
        data.clear();
        pointers.assign(init_size, data.end());
    }

    V& operator[](K key) {
        auto iter = find(key);
        if (iter == data.end()) {
            __insert(std::make_pair(key, V()));
            return find(key)->second;
        }
        return iter->second;
    }

    const V& at(K key) const {
        auto iter = find(key);
        if (iter == data.end()) {
            throw std::out_of_range("");
        }
        return iter->second;
    }
};
