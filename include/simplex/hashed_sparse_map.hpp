#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <simplex/hash_algorithms.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A hash map backed by individually allocated nodes with STL-like behaviors.
    /// @tparam key_t The key type used for lookups.
    /// @tparam value_t The mapped value type.
    /// @tparam hasher_t The hashing policy. Defaults to FNV-1A over the key bytes.
    ///
    /// Where hashed_dense_map keeps every entry in one contiguous array, the sparse map
    /// allocates each entry as its own node and threads the nodes into per-bucket collision
    /// chains. Nodes are never relocated once created, so pointers and references to a value
    /// stay valid across any number of subsequent insertions and removals -- only a rehash
    /// touches the bucket table, and even then the nodes themselves do not move. This mirrors
    /// the node-based behavior of std::unordered_map and trades the dense map's cache-friendly
    /// iteration for address stability.
    ///
    /// The bucket table grows in multiples of two once the load factor reaches one, and is
    /// never shrunk automatically. Iteration walks the bucket table, so the visitation order
    /// is unspecified and may change after a rehash.
    template <typename key_t, typename value_t, typename hasher_t = hashes::fnv1a<key_t>>
    class hashed_sparse_map
    {

        public:
            struct entry
            {
                key_t   key;
                value_t value;

                template <typename... Args> inline
                entry(const key_t &key, Args&&... args)
                    : key(key), value(std::forward<Args>(args)...) { }
            };

        private:
            struct node
            {
                entry  data;    // The stored key/value pair.
                node  *next;    // Next node sharing this bucket, or NULL at the chain tail.

                template <typename... Args> inline
                node(node *next, const key_t &key, Args&&... args)
                    : data(key, std::forward<Args>(args)...), next(next) { }
            };

            // Forward iterator that walks the bucket table, skipping empty buckets. Templated
            // so the same traversal serves both the mutable and const views; a nested class can
            // reach the enclosing map's private buckets/reserved members.
            template <typename qual_node_t, typename qual_map_t, typename qual_entry_t>
            class basic_iterator
            {

                public:
                    inline basic_iterator(qual_map_t *map, size_t bucket, qual_node_t *node)
                        : map(map), bucket(bucket), node(node) { }

                    inline qual_entry_t& operator*()  const { return this->node->data;  }
                    inline qual_entry_t* operator->() const { return &this->node->data; }

                    inline basic_iterator&
                    operator++()
                    {
                        this->node = this->node->next;
                        while (this->node == NULL)
                        {
                            this->bucket++;
                            if (this->bucket >= this->map->reserved) break;
                            this->node = this->map->buckets[this->bucket];
                        }
                        return *this;
                    }

                    inline bool operator==(const basic_iterator &o) const { return this->node == o.node; }
                    inline bool operator!=(const basic_iterator &o) const { return this->node != o.node; }

                private:
                    qual_map_t  *map;
                    size_t       bucket;
                    qual_node_t *node;

            };

        public:
            using iterator       = basic_iterator<node, hashed_sparse_map, entry>;
            using const_iterator = basic_iterator<const node, const hashed_sparse_map, const entry>;

        public:
            inline hashed_sparse_map() : buckets(NULL), reserved(0), count(0) { this->increase_reserves(); }
            inline virtual ~hashed_sparse_map()                              { this->release_memory();    }

            inline hashed_sparse_map(const hashed_sparse_map<key_t, value_t, hasher_t>& other)
                : buckets(NULL), reserved(0), count(0)
            {

                this->rehash(other.reserved);
                for (size_t i = 0; i < other.reserved; ++i)
                {
                    for (node *current = other.buckets[i]; current != NULL; current = current->next)
                    {
                        this->insert_node(current->data.key, current->data.value);
                    }
                }

            }

            inline hashed_sparse_map(hashed_sparse_map<key_t, value_t, hasher_t>&& other)
            {

                this->buckets  = other.buckets;
                this->reserved = other.reserved;
                this->count    = other.count;

                other.buckets  = NULL;
                other.reserved = 0;
                other.count    = 0;

            }

            inline hashed_sparse_map<key_t, value_t, hasher_t>&
            operator=(const hashed_sparse_map<key_t, value_t, hasher_t>& other)
            {

                if (this == &other) return *this;

                this->release_memory();
                this->rehash(other.reserved);

                for (size_t i = 0; i < other.reserved; ++i)
                {
                    for (node *current = other.buckets[i]; current != NULL; current = current->next)
                    {
                        this->insert_node(current->data.key, current->data.value);
                    }
                }

                return *this;

            }

            inline hashed_sparse_map<key_t, value_t, hasher_t>&
            operator=(hashed_sparse_map<key_t, value_t, hasher_t>&& other)
            {

                if (this == &other) return *this;

                this->release_memory();

                this->buckets  = other.buckets;
                this->reserved = other.reserved;
                this->count    = other.count;

                other.buckets  = NULL;
                other.reserved = 0;
                other.count    = 0;

                return *this;

            }

            inline size_t size() const          { return this->count;       }
            inline size_t capacity() const      { return this->reserved;     }
            inline bool empty() const           { return this->count == 0;   }

            inline iterator
            begin()
            {
                for (size_t i = 0; i < this->reserved; ++i)
                    if (this->buckets[i] != NULL) return iterator(this, i, this->buckets[i]);
                return this->end();
            }

            inline iterator end() { return iterator(this, this->reserved, NULL); }

            inline const_iterator
            begin() const
            {
                for (size_t i = 0; i < this->reserved; ++i)
                    if (this->buckets[i] != NULL) return const_iterator(this, i, this->buckets[i]);
                return this->end();
            }

            inline const_iterator end() const { return const_iterator(this, this->reserved, NULL); }

            inline bool
            contains(const key_t &key) const
            {
                return this->find_node(key) != NULL;
            }

            inline value_t*
            find(const key_t &key)
            {
                node *target = this->find_node(key);
                if (target == NULL) return NULL;
                return &target->data.value;
            }

            inline const value_t*
            find(const key_t &key) const
            {
                const node *target = this->find_node(key);
                if (target == NULL) return NULL;
                return &target->data.value;
            }

            inline value_t&
            get(const key_t &key)
            {
                node *target = this->find_node(key);
                SIMPLEX_ASSERT(target != NULL);
                return target->data.value;
            }

            inline const value_t&
            get(const key_t &key) const
            {
                const node *target = this->find_node(key);
                SIMPLEX_ASSERT(target != NULL);
                return target->data.value;
            }

            inline value_t&
            operator[](const key_t &key)
            {

                node *target = this->find_node(key);
                if (target == NULL)
                {
                    target = this->insert_node(key);
                }

                return target->data.value;

            }

            inline value_t&
            insert(const key_t &key, const value_t &value)
            {

                node *target = this->find_node(key);
                if (target != NULL)
                {
                    target->data.value = value;
                    return target->data.value;
                }

                target = this->insert_node(key, value);
                return target->data.value;

            }

            inline value_t&
            insert(const key_t &key, value_t &&value)
            {

                node *target = this->find_node(key);
                if (target != NULL)
                {
                    target->data.value = std::move(value);
                    return target->data.value;
                }

                target = this->insert_node(key, std::move(value));
                return target->data.value;

            }

            template <typename... Args> inline value_t&
            emplace(const key_t &key, Args&&... args)
            {

                // NOTE(Chris): Emplacement leaves an existing value untouched, mirroring the
                //              try-emplace semantics where the argument pack is only consumed
                //              when a new node is actually constructed.
                node *target = this->find_node(key);
                if (target != NULL) return target->data.value;

                target = this->insert_node(key, std::forward<Args>(args)...);
                return target->data.value;

            }

            inline bool
            remove(const key_t &key)
            {

                if (this->count == 0) return false;

                const size_t bucket = this->compute_bucket(key);
                node *current = this->buckets[bucket];
                node *previous = NULL;

                while (current != NULL && !(current->data.key == key))
                {
                    previous = current;
                    current = current->next;
                }

                if (current == NULL) return false;

                // Unlink the node from its collision chain, then release it. Other nodes keep
                // their addresses, so outstanding pointers into the map stay valid.
                if (previous == NULL) this->buckets[bucket] = current->next;
                else                  previous->next        = current->next;

                current->~node();
                simplex_memory_free(current);

                this->count--;
                return true;

            }

            inline void
            clear()
            {

                for (size_t i = 0; i < this->reserved; ++i)
                {
                    node *current = this->buckets[i];
                    while (current != NULL)
                    {
                        node *next = current->next;
                        current->~node();
                        simplex_memory_free(current);
                        current = next;
                    }
                    this->buckets[i] = NULL;
                }

                this->count = 0;

            }

            inline constexpr size_t
            get_minimum_reserved() const
            {
                if constexpr (sizeof(node) < 4)         return 64;
                else if constexpr (sizeof(node) < 8)    return 32;
                else if constexpr (sizeof(node) < 16)   return 16;
                else if constexpr (sizeof(node) < 32)   return 8;
                else if constexpr (sizeof(node) < 64)   return 4;
                else                                    return 1;
            }

        private:

            static inline size_t
            hash_of(const key_t &key)
            {
                return hasher_t::hash(key);
            }

            inline size_t
            compute_bucket(const key_t &key) const
            {
                // NOTE(Chris): reserved is always a power of two, so the mask selects the low
                //              bits of the hash in place of a modulo.
                return hash_of(key) & (this->reserved - 1);
            }

            inline node*
            find_node(const key_t &key) const
            {

                if (this->count == 0) return NULL;

                const size_t bucket = this->compute_bucket(key);
                node *current = this->buckets[bucket];

                while (current != NULL)
                {
                    if (current->data.key == key) return current;
                    current = current->next;
                }

                return NULL;

            }

            template <typename... Args> inline node*
            insert_node(const key_t &key, Args&&... args)
            {

                // Grow before linking so the new node lands in a bucket sized for the new table.
                if (this->count >= this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t bucket = this->compute_bucket(key);
                node *current = (node*)simplex_memory_alloc(sizeof(node));
                new (current) node(this->buckets[bucket], key, std::forward<Args>(args)...);
                this->buckets[bucket] = current;

                this->count++;
                return current;

            }

            inline void
            increase_reserves()
            {
                // NOTE(Chris): Presizing based on the size of the nodes for efficiency.
                const size_t target = (this->buckets == NULL) ? this->get_minimum_reserved()
                                                              : this->reserved * 2;
                this->rehash(target);
            }

            inline void
            rehash(size_t new_reserved)
            {

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned key_t / value_t support needs an aligned path.
                node **new_buckets = (node**)simplex_memory_alloc(new_reserved * sizeof(node*));
                for (size_t i = 0; i < new_reserved; ++i) new_buckets[i] = NULL;

                // Re-thread every existing node into the new table against the new mask. The
                // nodes are not moved or copied -- only their chain links are rewritten.
                for (size_t i = 0; i < this->reserved; ++i)
                {
                    node *current = this->buckets[i];
                    while (current != NULL)
                    {
                        node *next = current->next;
                        const size_t bucket = hash_of(current->data.key) & (new_reserved - 1);
                        current->next = new_buckets[bucket];
                        new_buckets[bucket] = current;
                        current = next;
                    }
                }

                if (this->buckets != NULL) simplex_memory_free(this->buckets);
                this->buckets  = new_buckets;
                this->reserved = new_reserved;

            }

            inline void
            release_memory()
            {

                if (this->buckets != NULL)
                {
                    this->clear();
                    simplex_memory_free(this->buckets);
                    this->buckets = NULL;
                }

                this->reserved = 0;
                this->count = 0;

            }

        private:
            node  **buckets;    // Bucket heads; first node in each chain, or NULL when empty.
            size_t  reserved;   // Bucket count (a power of two).
            size_t  count;      // Number of live nodes.

    };

};
