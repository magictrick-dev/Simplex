#pragma once
#include <utils/defs.hpp>
#include <concepts>
#include <type_traits>

#if defined(_MSC_VER)
#   include <intrin.h>
#endif

namespace spx
{

    namespace hashes
    {

        // ---------------------------------------------------------------------------------
        //      FNV-1A
        // ---------------------------------------------------------------------------------
        // Fowler-Noll-Vo, the byte-at-a-time xor/multiply hash. Cheap with no setup cost,
        // which makes it a sensible default for the short, fixed-size keys a dense map sees.

        template <typename type_t>
        requires std::is_trivially_copyable<type_t>::value && std::has_unique_object_representations<type_t>::value
        struct fnv1a
        {

            static inline constexpr uint64_t prime  = 0x100000001B3;
            static inline constexpr uint64_t offset = 0xcbf29ce484222325;

            static inline constexpr uint64_t
            hash(const type_t& value)
            {

                const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                uint64_t hash = offset;

                for (size_t i = 0; i < sizeof(type_t); ++i)
                {
                    hash ^= bytes[i];
                    hash *= prime;
                }

                return hash;

            }

        };

        // ---------------------------------------------------------------------------------
        //      MurmurHash3 (x64 128-bit core, folded to 64 bits)
        // ---------------------------------------------------------------------------------
        // Austin Appleby's MurmurHash3, x64_128 variant. We run the full 128-bit body and
        // return the first lane, which already absorbs the second lane during finalization.
        // NOTE(Chris): Block reads assume a little-endian host; the dense map only hashes
        //              in-memory object representations, so the value is never serialized.

        template <typename type_t>
        requires std::is_trivially_copyable<type_t>::value && std::has_unique_object_representations<type_t>::value
        struct murmurhash3
        {

            static inline constexpr uint32_t seed = 0;

            static inline uint64_t
            rotl64(uint64_t x, int8_t r)
            {
                return (x << r) | (x >> (64 - r));
            }

            static inline uint64_t
            read64(const uint8_t *p)
            {
                uint64_t v;
                memcpy(&v, p, sizeof(v));
                return v;
            }

            static inline uint64_t
            fmix64(uint64_t k)
            {
                k ^= k >> 33;
                k *= 0xff51afd7ed558ccd;
                k ^= k >> 33;
                k *= 0xc4ceb9fe1a85ec53;
                k ^= k >> 33;
                return k;
            }

            static inline uint64_t
            hash(const type_t& value)
            {

                static constexpr uint64_t c1 = 0x87c37b91114253d5;
                static constexpr uint64_t c2 = 0x4cf5ad432745937f;

                const uint8_t *data = reinterpret_cast<const uint8_t*>(&value);
                const size_t   len    = sizeof(type_t);
                const size_t   blocks = len / 16;

                uint64_t h1 = seed;
                uint64_t h2 = seed;

                // Body: consume the input in 16-byte blocks, two 64-bit lanes at a time.
                for (size_t i = 0; i < blocks; ++i)
                {

                    uint64_t k1 = read64(data + i * 16 + 0);
                    uint64_t k2 = read64(data + i * 16 + 8);

                    k1 *= c1; k1 = rotl64(k1, 31); k1 *= c2; h1 ^= k1;
                    h1 = rotl64(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

                    k2 *= c2; k2 = rotl64(k2, 33); k2 *= c1; h2 ^= k2;
                    h2 = rotl64(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;

                }

                // Tail: fold the trailing 0..15 bytes that did not fill a whole block.
                const uint8_t *tail = data + blocks * 16;
                uint64_t k1 = 0;
                uint64_t k2 = 0;

                switch (len & 15)
                {
                    case 15: k2 ^= (uint64_t)tail[14] << 48;
                    case 14: k2 ^= (uint64_t)tail[13] << 40;
                    case 13: k2 ^= (uint64_t)tail[12] << 32;
                    case 12: k2 ^= (uint64_t)tail[11] << 24;
                    case 11: k2 ^= (uint64_t)tail[10] << 16;
                    case 10: k2 ^= (uint64_t)tail[ 9] << 8;
                    case  9: k2 ^= (uint64_t)tail[ 8] << 0;
                             k2 *= c2; k2 = rotl64(k2, 33); k2 *= c1; h2 ^= k2;
                    case  8: k1 ^= (uint64_t)tail[ 7] << 56;
                    case  7: k1 ^= (uint64_t)tail[ 6] << 48;
                    case  6: k1 ^= (uint64_t)tail[ 5] << 40;
                    case  5: k1 ^= (uint64_t)tail[ 4] << 32;
                    case  4: k1 ^= (uint64_t)tail[ 3] << 24;
                    case  3: k1 ^= (uint64_t)tail[ 2] << 16;
                    case  2: k1 ^= (uint64_t)tail[ 1] << 8;
                    case  1: k1 ^= (uint64_t)tail[ 0] << 0;
                             k1 *= c1; k1 = rotl64(k1, 31); k1 *= c2; h1 ^= k1;
                    default: break; // len is a multiple of 16; there are no tail bytes.
                }

                // Finalization: mix the length in and avalanche both lanes together.
                h1 ^= len; h2 ^= len;
                h1 += h2;  h2 += h1;
                h1 = fmix64(h1);
                h2 = fmix64(h2);
                h1 += h2;  h2 += h1;

                return h1;

            }

        };

        // ---------------------------------------------------------------------------------
        //      rapidhash
        // ---------------------------------------------------------------------------------
        // Nicolas De Carli's rapidhash, the successor to wyhash. Built around a 64x64->128
        // bit "multiply-fold" mixing step, it is the fastest of the three on larger keys.
        // NOTE(Chris): 64/32-bit reads assume a little-endian host, matching the in-memory
        //              representation the dense map hashes.

        template <typename type_t>
        requires std::is_trivially_copyable<type_t>::value && std::has_unique_object_representations<type_t>::value
        struct rapidhash
        {

            static inline constexpr uint64_t seed      = 0xbdd89aa982704029;
            static inline constexpr uint64_t secret[3] =
            {
                0x2d358dccaa6c78a5,
                0x8bb84b93962eacc9,
                0x4b33a62ed433d4a3,
            };

            static inline uint64_t
            read64(const uint8_t *p)
            {
                uint64_t v;
                memcpy(&v, p, sizeof(v));
                return v;
            }

            static inline uint64_t
            read32(const uint8_t *p)
            {
                uint32_t v;
                memcpy(&v, p, sizeof(v));
                return v;
            }

            // Assembles a 1..3 byte tail into a single word without reading out of bounds.
            static inline uint64_t
            read_small(const uint8_t *p, size_t k)
            {
                return ((uint64_t)p[0] << 56) | ((uint64_t)p[k >> 1] << 32) | p[k - 1];
            }

            // 64x64 -> 128 bit multiply, returning the low half and storing the high half.
            static inline uint64_t
            mul128(uint64_t a, uint64_t b, uint64_t *high)
            {
            #if defined(__SIZEOF_INT128__)

                __uint128_t product = (__uint128_t)a * (__uint128_t)b;
                *high = (uint64_t)(product >> 64);
                return (uint64_t)product;

            #elif defined(_MSC_VER) && defined(_M_X64)

                return _umul128(a, b, high);

            #else

                // Portable schoolbook fallback when no wide multiply is available.
                uint64_t a_low  = (uint32_t)a;
                uint64_t a_high = a >> 32;
                uint64_t b_low  = (uint32_t)b;
                uint64_t b_high = b >> 32;

                uint64_t low_low   = a_low  * b_low;
                uint64_t high_low  = a_high * b_low;
                uint64_t low_high  = a_low  * b_high;
                uint64_t high_high = a_high * b_high;

                uint64_t cross = (low_low >> 32) + (uint32_t)high_low + (uint32_t)low_high;
                *high = high_high + (high_low >> 32) + (low_high >> 32) + (cross >> 32);
                return (low_low & 0xffffffff) | (cross << 32);

            #endif
            }

            // Multiplies the two words and writes the 128-bit product back across them.
            static inline void
            mum(uint64_t *a, uint64_t *b)
            {
                uint64_t high;
                uint64_t low = mul128(*a, *b, &high);
                *a = low;
                *b = high;
            }

            // Folds a multiplied pair down into a single mixed word.
            static inline uint64_t
            mix(uint64_t a, uint64_t b)
            {
                mum(&a, &b);
                return a ^ b;
            }

            static inline uint64_t
            hash(const type_t& value)
            {

                const uint8_t *p   = reinterpret_cast<const uint8_t*>(&value);
                size_t         len = sizeof(type_t);

                uint64_t state = seed ^ mix(seed ^ secret[0], secret[1]) ^ len;
                uint64_t a = 0;
                uint64_t b = 0;

                if (len <= 16)
                {

                    if (len >= 4)
                    {
                        const uint8_t *plast = p + len - 4;
                        a = (read32(p) << 32) | read32(plast);
                        const uint64_t delta = (len & 24) >> (len >> 3);
                        b = (read32(p + delta) << 32) | read32(plast - delta);
                    }
                    else if (len > 0)
                    {
                        a = read_small(p, len);
                        b = 0;
                    }

                }

                else
                {

                    size_t i = len;

                    // Wide path: three independent accumulators consume 48 bytes per turn.
                    if (i > 48)
                    {

                        uint64_t state1 = state;
                        uint64_t state2 = state;

                        do
                        {
                            state  = mix(read64(p)      ^ secret[0], read64(p + 8)  ^ state);
                            state1 = mix(read64(p + 16) ^ secret[1], read64(p + 24) ^ state1);
                            state2 = mix(read64(p + 32) ^ secret[2], read64(p + 40) ^ state2);
                            p += 48;
                            i -= 48;
                        } while (i >= 48);

                        state ^= state1 ^ state2;

                    }

                    if (i > 16)
                    {
                        state = mix(read64(p) ^ secret[2], read64(p + 8) ^ state ^ secret[1]);
                        if (i > 32)
                        {
                            state = mix(read64(p + 16) ^ secret[2], read64(p + 24) ^ state);
                        }
                    }

                    // The last 16 bytes always overlap back into the consumed region.
                    a = read64(p + i - 16);
                    b = read64(p + i - 8);

                }

                a ^= secret[1];
                b ^= state;
                mum(&a, &b);
                return mix(a ^ secret[0] ^ len, b ^ secret[1]);

            }

        };

    }

}
