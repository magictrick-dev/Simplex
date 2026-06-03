#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <simplex/hash_algorithms.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A hash map backed by densely packed storage with STL-like behaviors.
    /// @tparam key_t The key type used for lookups.
    /// @tparam value_t The mapped value type.
    /// @tparam hasher_t The hashing policy. Defaults to FNV-1A over the key bytes.
    ///
    /// Entries are stored contiguously in a single growing array, which keeps iteration
    /// and the working set cache-friendly regardless of insertion order. A separate
    /// bucket table maps a hashed key to the head of a collision chain, and a parallel
    /// chain array links the entries that share a bucket. Both links are stored as
    /// indices rather than pointers so the dense storage can be relocated freely as it
    /// grows.
    ///
    /// Like the other dynamic containers, the backing storage grows in multiples of two
    /// and is never shrunk automatically. Removal swaps the final entry into the vacated
    /// slot to keep the storage dense, which means iteration order is not stable across
    /// removals.
    template <typename key_t, typename value_t, typename hasher_t = hashes::fnv1a<key_t>>
    class hashed_dense_map
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

            static inline constexpr size_t invalid_index = (size_t)-1;

        public:
            inline hashed_dense_map() : entries(NULL), chain(NULL), buckets(NULL), reserved(0), count(0) { this->increase_reserves(); }
            inline virtual ~hashed_dense_map()                                                           { this->release_memory();    }

            inline hashed_dense_map(const hashed_dense_map<key_t, value_t, hasher_t>& other)
                : entries(NULL), chain(NULL), buckets(NULL), reserved(0), count(0)
            {

                this->allocate_buffers(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->entries + i) entry(other.entries[i]);
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < other.reserved; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

            }

            inline hashed_dense_map(hashed_dense_map<key_t, value_t, hasher_t>&& other)
            {

                this->entries   = other.entries;
                this->chain     = other.chain;
                this->buckets   = other.buckets;
                this->reserved  = other.reserved;
                this->count     = other.count;

                other.entries   = NULL;
                other.chain     = NULL;
                other.buckets   = NULL;
                other.reserved  = 0;
                other.count     = 0;

            }

            inline hashed_dense_map<key_t, value_t, hasher_t>&
            operator=(const hashed_dense_map<key_t, value_t, hasher_t>& other)
            {

                if (this == &other) return *this;

                this->release_memory();
                this->allocate_buffers(other.reserved);

                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->entries + i) entry(other.entries[i]);
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < other.reserved; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

                return *this;

            }

            inline hashed_dense_map<key_t, value_t, hasher_t>&
            operator=(hashed_dense_map<key_t, value_t, hasher_t>&& other)
            {

                if (this == &other) return *this;

                this->release_memory();

                this->entries   = other.entries;
                this->chain     = other.chain;
                this->buckets   = other.buckets;
                this->reserved  = other.reserved;
                this->count     = other.count;

                other.entries   = NULL;
                other.chain     = NULL;
                other.buckets   = NULL;
                other.reserved  = 0;
                other.count     = 0;

                return *this;

            }

            inline size_t size() const          { return this->count;           }
            inline size_t capacity() const      { return this->reserved;         }
            inline bool empty() const           { return this->count == 0;       }
            inline entry* begin()               { return this->entries;          }
            inline entry* end()                 { return this->entries + count;  }
            inline const entry* begin() const   { return this->entries;          }
            inline const entry* end() const     { return this->entries + count;  }

            inline bool
            contains(const key_t &key) const
            {
                return this->find_index(key) != invalid_index;
            }

            inline value_t*
            find(const key_t &key)
            {
                const size_t index = this->find_index(key);
                if (index == invalid_index) return NULL;
                return &this->entries[index].value;
            }

            inline const value_t*
            find(const key_t &key) const
            {
                const size_t index = this->find_index(key);
                if (index == invalid_index) return NULL;
                return &this->entries[index].value;
            }

            inline value_t&
            get(const key_t &key)
            {
                const size_t index = this->find_index(key);
                SIMPLEX_ASSERT(index != invalid_index);
                return this->entries[index].value;
            }

            inline const value_t&
            get(const key_t &key) const
            {
                const size_t index = this->find_index(key);
                SIMPLEX_ASSERT(index != invalid_index);
                return this->entries[index].value;
            }

            inline value_t&
            operator[](const key_t &key)
            {

                size_t index = this->find_index(key);
                if (index == invalid_index)
                {
                    index = this->append_entry(key);
                }

                return this->entries[index].value;

            }

            inline value_t&
            insert(const key_t &key, const value_t &value)
            {

                size_t index = this->find_index(key);
                if (index != invalid_index)
                {
                    this->entries[index].value = value;
                    return this->entries[index].value;
                }

                index = this->append_entry(key, value);
                return this->entries[index].value;

            }

            inline value_t&
            insert(const key_t &key, value_t &&value)
            {

                size_t index = this->find_index(key);
                if (index != invalid_index)
                {
                    this->entries[index].value = std::move(value);
                    return this->entries[index].value;
                }

                index = this->append_entry(key, std::move(value));
                return this->entries[index].value;

            }

            template <typename... Args> inline value_t&
            emplace(const key_t &key, Args&&... args)
            {

                // NOTE(Chris): Emplacement leaves an existing value untouched, mirroring
                //              the try-emplace semantics where the argument pack is only
                //              consumed when a new entry is actually constructed.
                size_t index = this->find_index(key);
                if (index != invalid_index) return this->entries[index].value;

                index = this->append_entry(key, std::forward<Args>(args)...);
                return this->entries[index].value;

            }

            inline bool
            remove(const key_t &key)
            {

                if (this->count == 0) return false;

                const size_t bucket = this->compute_bucket(key);
                size_t index = this->buckets[bucket];
                size_t previous = invalid_index;

                while (index != invalid_index && !(this->entries[index].key == key))
                {
                    previous = index;
                    index = this->chain[index];
                }

                if (index == invalid_index) return false;

                // Unlink the target entry from its collision chain.
                if (previous == invalid_index) this->buckets[bucket] = this->chain[index];
                else                            this->chain[previous] = this->chain[index];

                const size_t last = this->count - 1;
                if (index != last)
                {

                    // Relocate the final entry into the vacated slot to keep storage dense.
                    this->entries[index].~entry();
                    new (this->entries + index) entry(std::move(this->entries[last]));
                    this->chain[index] = this->chain[last];

                    // Repoint whatever referenced the relocated entry at its new index.
                    const size_t moved_bucket = this->compute_bucket(this->entries[index].key);
                    if (this->buckets[moved_bucket] == last)
                    {
                        this->buckets[moved_bucket] = index;
                    }
                    else
                    {
                        size_t scan = this->buckets[moved_bucket];
                        while (this->chain[scan] != last) scan = this->chain[scan];
                        this->chain[scan] = index;
                    }

                    this->entries[last].~entry();

                }

                else
                {
                    this->entries[index].~entry();
                }

                this->count--;
                return true;

            }

            inline void
            clear()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    this->entries[i].~entry();
                }

                for (size_t i = 0; i < this->reserved; ++i)
                {
                    this->buckets[i] = invalid_index;
                }

                this->count = 0;

            }

            inline constexpr size_t
            get_minimum_reserved() const
            {
                if constexpr (sizeof(entry) < 4)        return 64;
                else if constexpr (sizeof(entry) < 8)   return 32;
                else if constexpr (sizeof(entry) < 16)  return 16;
                else if constexpr (sizeof(entry) < 32)  return 8;
                else if constexpr (sizeof(entry) < 64)  return 4;
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
                // NOTE(Chris): reserved is always a power of two, so the mask selects the
                //              low bits of the hash in place of a modulo.
                return hash_of(key) & (this->reserved - 1);
            }

            inline size_t
            find_index(const key_t &key) const
            {

                if (this->count == 0) return invalid_index;

                const size_t bucket = this->compute_bucket(key);
                size_t index = this->buckets[bucket];

                while (index != invalid_index)
                {
                    if (this->entries[index].key == key) return index;
                    index = this->chain[index];
                }

                return invalid_index;

            }

            template <typename... Args> inline size_t
            append_entry(const key_t &key, Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t index = this->count;
                new (this->entries + index) entry(key, std::forward<Args>(args)...);

                const size_t bucket = this->compute_bucket(key);
                this->chain[index] = this->buckets[bucket];
                this->buckets[bucket] = index;

                this->count++;
                return index;

            }

            inline void
            allocate_buffers(size_t size)
            {

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned key_t / value_t support needs an aligned path.
                this->entries  = (entry*)simplex_memory_alloc(size * sizeof(entry));
                this->chain    = (size_t*)simplex_memory_alloc(size * sizeof(size_t));
                this->buckets  = (size_t*)simplex_memory_alloc(size * sizeof(size_t));
                this->reserved = size;

            }

            inline void
            increase_reserves()
            {

                // NOTE(Chris): Presizing based on the size of the entries for efficiency.
                if (this->entries == NULL)
                {
                    const size_t minimum = this->get_minimum_reserved();
                    this->allocate_buffers(minimum);
                    for (size_t i = 0; i < minimum; ++i) this->buckets[i] = invalid_index;
                    return;
                }

                const size_t new_reserved = this->reserved * 2;
                entry *new_entries  = (entry*)simplex_memory_alloc(new_reserved * sizeof(entry));
                size_t *new_chain   = (size_t*)simplex_memory_alloc(new_reserved * sizeof(size_t));
                size_t *new_buckets = (size_t*)simplex_memory_alloc(new_reserved * sizeof(size_t));

                for (size_t i = 0; i < new_reserved; ++i) new_buckets[i] = invalid_index;

                // NOTE(Chris): The bucket count changed with the resize, so the chains have
                //              to be rebuilt against the new mask while the entries migrate.
                for (size_t i = 0; i < this->count; ++i)
                {
                    entry *previous = this->entries + i;
                    entry *current  = new_entries + i;
                    new (current) entry(std::move(*previous));
                    previous->~entry();

                    const size_t bucket = hash_of(current->key) & (new_reserved - 1);
                    new_chain[i] = new_buckets[bucket];
                    new_buckets[bucket] = i;
                }

                simplex_memory_free(this->entries);
                simplex_memory_free(this->chain);
                simplex_memory_free(this->buckets);

                this->entries  = new_entries;
                this->chain    = new_chain;
                this->buckets  = new_buckets;
                this->reserved = new_reserved;

            }

            inline void
            release_memory()
            {

                if (this->entries != NULL)
                {
                    for (size_t i = 0; i < this->count; ++i)
                    {
                        this->entries[i].~entry();
                    }

                    simplex_memory_free(this->entries);
                    simplex_memory_free(this->chain);
                    simplex_memory_free(this->buckets);

                    this->entries = NULL;
                    this->chain   = NULL;
                    this->buckets = NULL;
                }

                this->reserved = 0;
                this->count = 0;

            }

        private:
            entry  *entries;    // Dense, contiguous storage of live key/value pairs.
            size_t *chain;      // Parallel to entries; next index in the same bucket chain.
            size_t *buckets;    // Bucket heads; first entry index for each bucket, or invalid.
            size_t  reserved;   // Capacity of entries/chain and the bucket count (a power of two).
            size_t  count;      // Number of live entries.

    };

};
