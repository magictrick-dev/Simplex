#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <simplex/hash_algorithms.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A set of unique keys backed by densely packed storage with STL-like behaviors.
    /// @tparam key_t The element type stored in the set.
    /// @tparam hasher_t The hashing policy. Defaults to FNV-1A over the key bytes.
    ///
    /// The dynamic_set mirrors the structure of hashed_dense_map, minus the mapped value:
    /// keys are stored contiguously in a single growing array which keeps iteration and the
    /// working set cache-friendly regardless of insertion order. A separate bucket table maps
    /// a hashed key to the head of a collision chain, and a parallel chain array links the
    /// keys that share a bucket. Both links are stored as indices rather than pointers so the
    /// dense storage can be relocated freely as it grows.
    ///
    /// Unlike std::set, membership is resolved through hashing rather than ordering, so the
    /// stored keys are not iterated in sorted order. The backing storage grows in multiples
    /// of two and is never shrunk automatically. Removal swaps the final key into the vacated
    /// slot to keep the storage dense, which means iteration order is not stable across
    /// removals. Iteration is read-only because mutating a stored key would desynchronise it
    /// from its bucket chain.
    template <typename key_t, typename hasher_t = hashes::fnv1a<key_t>>
    class dynamic_set
    {

        public:
            static inline constexpr size_t invalid_index = (size_t)-1;

        public:
            inline dynamic_set() : keys(NULL), chain(NULL), buckets(NULL), reserved(0), count(0) { this->increase_reserves(); }
            inline virtual ~dynamic_set()                                                        { this->release_memory();    }

            inline dynamic_set(const dynamic_set<key_t, hasher_t>& other)
                : keys(NULL), chain(NULL), buckets(NULL), reserved(0), count(0)
            {

                this->allocate_buffers(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->keys + i) key_t(other.keys[i]);
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < other.reserved; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

            }

            inline dynamic_set(dynamic_set<key_t, hasher_t>&& other)
            {

                this->keys      = other.keys;
                this->chain     = other.chain;
                this->buckets   = other.buckets;
                this->reserved  = other.reserved;
                this->count     = other.count;

                other.keys      = NULL;
                other.chain     = NULL;
                other.buckets   = NULL;
                other.reserved  = 0;
                other.count     = 0;

            }

            inline dynamic_set<key_t, hasher_t>&
            operator=(const dynamic_set<key_t, hasher_t>& other)
            {

                if (this == &other) return *this;

                this->release_memory();
                this->allocate_buffers(other.reserved);

                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->keys + i) key_t(other.keys[i]);
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < other.reserved; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

                return *this;

            }

            inline dynamic_set<key_t, hasher_t>&
            operator=(dynamic_set<key_t, hasher_t>&& other)
            {

                if (this == &other) return *this;

                this->release_memory();

                this->keys      = other.keys;
                this->chain     = other.chain;
                this->buckets   = other.buckets;
                this->reserved  = other.reserved;
                this->count     = other.count;

                other.keys      = NULL;
                other.chain     = NULL;
                other.buckets   = NULL;
                other.reserved  = 0;
                other.count     = 0;

                return *this;

            }

            inline size_t size() const          { return this->count;          }
            inline size_t capacity() const      { return this->reserved;        }
            inline bool empty() const           { return this->count == 0;      }

            // NOTE(Chris): Iteration is const-only -- a stored key cannot be mutated in place
            //              without invalidating the bucket chain that locates it.
            inline const key_t* begin() const   { return this->keys;            }
            inline const key_t* end() const     { return this->keys + count;    }

            inline bool
            contains(const key_t &key) const
            {
                return this->find_index(key) != invalid_index;
            }

            inline const key_t*
            find(const key_t &key) const
            {
                const size_t index = this->find_index(key);
                if (index == invalid_index) return NULL;
                return &this->keys[index];
            }

            // NOTE(Chris): Mirrors std::set::insert -- returns true when a new key was added
            //              and false when the key already existed.
            inline bool
            insert(const key_t &key)
            {

                if (this->find_index(key) != invalid_index) return false;
                this->append_key(key);
                return true;

            }

            inline bool
            insert(key_t &&key)
            {

                if (this->find_index(key) != invalid_index) return false;
                this->append_key(std::move(key));
                return true;

            }

            template <typename... Args> inline bool
            emplace(Args&&... args)
            {

                // NOTE(Chris): The key has to be materialised before it can be hashed, so the
                //              argument pack is consumed up front and the resulting key is then
                //              moved into storage only if it is not already present.
                key_t key(std::forward<Args>(args)...);
                if (this->find_index(key) != invalid_index) return false;
                this->append_key(std::move(key));
                return true;

            }

            inline bool
            remove(const key_t &key)
            {

                if (this->count == 0) return false;

                const size_t bucket = this->compute_bucket(key);
                size_t index = this->buckets[bucket];
                size_t previous = invalid_index;

                while (index != invalid_index && !(this->keys[index] == key))
                {
                    previous = index;
                    index = this->chain[index];
                }

                if (index == invalid_index) return false;

                // Unlink the target key from its collision chain.
                if (previous == invalid_index) this->buckets[bucket] = this->chain[index];
                else                            this->chain[previous] = this->chain[index];

                const size_t last = this->count - 1;
                if (index != last)
                {

                    // Relocate the final key into the vacated slot to keep storage dense.
                    this->keys[index].~key_t();
                    new (this->keys + index) key_t(std::move(this->keys[last]));
                    this->chain[index] = this->chain[last];

                    // Repoint whatever referenced the relocated key at its new index.
                    const size_t moved_bucket = this->compute_bucket(this->keys[index]);
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

                    this->keys[last].~key_t();

                }

                else
                {
                    this->keys[index].~key_t();
                }

                this->count--;
                return true;

            }

            inline void
            clear()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    this->keys[i].~key_t();
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
                if constexpr (sizeof(key_t) < 4)        return 64;
                else if constexpr (sizeof(key_t) < 8)   return 32;
                else if constexpr (sizeof(key_t) < 16)  return 16;
                else if constexpr (sizeof(key_t) < 32)  return 8;
                else if constexpr (sizeof(key_t) < 64)  return 4;
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
                    if (this->keys[index] == key) return index;
                    index = this->chain[index];
                }

                return invalid_index;

            }

            template <typename... Args> inline size_t
            append_key(Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t index = this->count;
                new (this->keys + index) key_t(std::forward<Args>(args)...);

                const size_t bucket = this->compute_bucket(this->keys[index]);
                this->chain[index] = this->buckets[bucket];
                this->buckets[bucket] = index;

                this->count++;
                return index;

            }

            inline void
            allocate_buffers(size_t size)
            {

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned key_t support needs an aligned path.
                this->keys     = (key_t*)simplex_memory_alloc(size * sizeof(key_t));
                this->chain    = (size_t*)simplex_memory_alloc(size * sizeof(size_t));
                this->buckets  = (size_t*)simplex_memory_alloc(size * sizeof(size_t));
                this->reserved = size;

            }

            inline void
            increase_reserves()
            {

                // NOTE(Chris): Presizing based on the size of the keys for efficiency.
                if (this->keys == NULL)
                {
                    const size_t minimum = this->get_minimum_reserved();
                    this->allocate_buffers(minimum);
                    for (size_t i = 0; i < minimum; ++i) this->buckets[i] = invalid_index;
                    return;
                }

                const size_t new_reserved = this->reserved * 2;
                key_t  *new_keys    = (key_t*)simplex_memory_alloc(new_reserved * sizeof(key_t));
                size_t *new_chain   = (size_t*)simplex_memory_alloc(new_reserved * sizeof(size_t));
                size_t *new_buckets = (size_t*)simplex_memory_alloc(new_reserved * sizeof(size_t));

                for (size_t i = 0; i < new_reserved; ++i) new_buckets[i] = invalid_index;

                // NOTE(Chris): The bucket count changed with the resize, so the chains have
                //              to be rebuilt against the new mask while the keys migrate.
                for (size_t i = 0; i < this->count; ++i)
                {
                    key_t *previous = this->keys + i;
                    key_t *current  = new_keys + i;
                    new (current) key_t(std::move(*previous));
                    previous->~key_t();

                    const size_t bucket = hash_of(*current) & (new_reserved - 1);
                    new_chain[i] = new_buckets[bucket];
                    new_buckets[bucket] = i;
                }

                simplex_memory_free(this->keys);
                simplex_memory_free(this->chain);
                simplex_memory_free(this->buckets);

                this->keys     = new_keys;
                this->chain    = new_chain;
                this->buckets  = new_buckets;
                this->reserved = new_reserved;

            }

            inline void
            release_memory()
            {

                if (this->keys != NULL)
                {
                    for (size_t i = 0; i < this->count; ++i)
                    {
                        this->keys[i].~key_t();
                    }

                    simplex_memory_free(this->keys);
                    simplex_memory_free(this->chain);
                    simplex_memory_free(this->buckets);

                    this->keys    = NULL;
                    this->chain   = NULL;
                    this->buckets = NULL;
                }

                this->reserved = 0;
                this->count = 0;

            }

        private:
            key_t  *keys;       // Dense, contiguous storage of live keys.
            size_t *chain;      // Parallel to keys; next index in the same bucket chain.
            size_t *buckets;    // Bucket heads; first key index for each bucket, or invalid.
            size_t  reserved;   // Capacity of keys/chain and the bucket count (a power of two).
            size_t  count;      // Number of live keys.

    };

};
