#pragma once
#include <utils/defs.hpp>
#include <simplex/hash_algorithms.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A stack-storage fixed-capacity set of unique keys with STL-like behaviors.
    /// @tparam key_t The element type stored in the set.
    /// @tparam capacity The maximum number of keys.
    /// @tparam hasher_t The hashing policy. Defaults to FNV-1A over the key bytes.
    ///
    /// The static_set is the fixed-capacity counterpart to dynamic_set: it performs no heap
    /// allocation and shares the same dense-storage-plus-bucket-table layout. Keys live in an
    /// in-place byte buffer and are only constructed when inserted and destroyed when removed,
    /// so non-trivially constructible types are supported. A fixed bucket table (sized to the
    /// next power of two at or above the capacity) maps a hashed key to the head of a collision
    /// chain, and a parallel chain array links the keys that share a bucket.
    ///
    /// Like static_array, exceeding the capacity asserts. Membership is resolved through hashing
    /// rather than ordering, so keys are not iterated in sorted order, and removal swaps the
    /// final key into the vacated slot to keep storage dense. Iteration is read-only because
    /// mutating a stored key would desynchronise it from its bucket chain.
    template <typename key_t, size_t capacity, typename hasher_t = hashes::fnv1a<key_t>>
    class static_set
    {

        public:
            static inline constexpr size_t invalid_index = (size_t)-1;

        public:
            inline static_set()
            {
                this->reset_buckets();
            }

            inline static_set(const static_set<key_t, capacity, hasher_t>& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->key_ptr(i)) key_t(*other.key_ptr(i));
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < bucket_count; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

            }

            inline static_set(static_set<key_t, capacity, hasher_t>&& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    key_t *previous = other.key_ptr(i);
                    new (this->key_ptr(i)) key_t(std::move(*previous));
                    previous->~key_t();
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < bucket_count; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;
                other.count = 0;
                other.reset_buckets();

            }

            inline ~static_set()
            {
                this->clear();
            }

            inline static_set<key_t, capacity, hasher_t>&
            operator=(const static_set<key_t, capacity, hasher_t>& other)
            {

                if (this == &other) return *this;

                this->clear();

                for (size_t i = 0; i < other.count; ++i)
                {
                    new (this->key_ptr(i)) key_t(*other.key_ptr(i));
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < bucket_count; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;

                return *this;

            }

            inline static_set<key_t, capacity, hasher_t>&
            operator=(static_set<key_t, capacity, hasher_t>&& other)
            {

                if (this == &other) return *this;

                this->clear();

                for (size_t i = 0; i < other.count; ++i)
                {
                    key_t *previous = other.key_ptr(i);
                    new (this->key_ptr(i)) key_t(std::move(*previous));
                    previous->~key_t();
                    this->chain[i] = other.chain[i];
                }

                for (size_t i = 0; i < bucket_count; ++i)
                {
                    this->buckets[i] = other.buckets[i];
                }

                this->count = other.count;
                other.count = 0;
                other.reset_buckets();

                return *this;

            }

            inline size_t size() const          { return this->count;               }
            inline size_t max_size() const       { return capacity;                  }
            inline bool empty() const           { return this->count == 0;          }
            inline bool full() const            { return this->count == capacity;   }

            // NOTE(Chris): Iteration is const-only -- a stored key cannot be mutated in place
            //              without invalidating the bucket chain that locates it.
            inline const key_t* begin() const   { return this->key_ptr(0);          }
            inline const key_t* end() const     { return this->key_ptr(this->count); }

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
                return this->key_ptr(index);
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

                while (index != invalid_index && !(*this->key_ptr(index) == key))
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
                    this->key_ptr(index)->~key_t();
                    new (this->key_ptr(index)) key_t(std::move(*this->key_ptr(last)));
                    this->chain[index] = this->chain[last];

                    // Repoint whatever referenced the relocated key at its new index.
                    const size_t moved_bucket = this->compute_bucket(*this->key_ptr(index));
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

                    this->key_ptr(last)->~key_t();

                }

                else
                {
                    this->key_ptr(index)->~key_t();
                }

                this->count--;
                return true;

            }

            inline void
            clear()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    this->key_ptr(i)->~key_t();
                }

                this->count = 0;
                this->reset_buckets();

            }

        private:

            // NOTE(Chris): The bucket table is masked rather than divided, so it has to be a
            //              power of two; size it to the next power of two at or above capacity.
            static inline constexpr size_t
            compute_bucket_count()
            {
                size_t result = 1;
                while (result < capacity) result *= 2;
                return result;
            }

            static inline constexpr size_t bucket_count = compute_bucket_count();

            static inline size_t
            hash_of(const key_t &key)
            {
                return hasher_t::hash(key);
            }

            inline size_t
            compute_bucket(const key_t &key) const
            {
                return hash_of(key) & (bucket_count - 1);
            }

            inline void
            reset_buckets()
            {
                for (size_t i = 0; i < bucket_count; ++i) this->buckets[i] = invalid_index;
            }

            inline key_t*       key_ptr(size_t index)       { return (key_t*)(&this->buffer[index * sizeof(key_t)]); }
            inline const key_t* key_ptr(size_t index) const { return (const key_t*)(&this->buffer[index * sizeof(key_t)]); }

            inline size_t
            find_index(const key_t &key) const
            {

                if (this->count == 0) return invalid_index;

                const size_t bucket = this->compute_bucket(key);
                size_t index = this->buckets[bucket];

                while (index != invalid_index)
                {
                    if (*this->key_ptr(index) == key) return index;
                    index = this->chain[index];
                }

                return invalid_index;

            }

            template <typename... Args> inline size_t
            append_key(Args&&... args)
            {

                SIMPLEX_ASSERT(this->count < capacity);

                const size_t index = this->count;
                new (this->key_ptr(index)) key_t(std::forward<Args>(args)...);

                const size_t bucket = this->compute_bucket(*this->key_ptr(index));
                this->chain[index] = this->buckets[bucket];
                this->buckets[bucket] = index;

                this->count++;
                return index;

            }

        private:
            alignas(key_t) std::byte buffer[capacity * sizeof(key_t)];  // Dense storage of live keys.
            size_t chain[capacity];                                     // Next index in the same bucket chain.
            size_t buckets[bucket_count];                               // Bucket heads, or invalid_index.
            size_t count = 0;                                           // Number of live keys.

    };

};
