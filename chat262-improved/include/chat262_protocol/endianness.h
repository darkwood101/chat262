#ifndef _ENDIANNESS_H_
#define _ENDIANNESS_H_

#include <cstdint>

// macOS apparently doesn't have an <endian.h> header.
// Joke's on them, we'll ship our own.

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

inline constexpr uint16_t e_htole16(uint16_t x) {
    return x;
}
inline constexpr uint32_t e_htole32(uint32_t x) {
    return x;
}
inline constexpr uint64_t e_htole64(uint64_t x) {
    return x;
}
inline constexpr uint16_t e_le16toh(uint16_t x) {
    return x;
}
inline constexpr uint32_t e_le32toh(uint32_t x) {
    return x;
}
inline constexpr uint64_t e_le64toh(uint64_t x) {
    return x;
}
inline constexpr uint16_t e_htobe16(uint16_t x) {
    return ((x >> 8) & 0x00FFu) | ((x << 8) & 0xFF00u);
}
inline constexpr uint32_t e_htobe32(uint32_t x) {
    return ((x >> 24) & 0x000000FFul) | ((x >> 8) & 0x0000FF00UL) |
           ((x << 8) & 0x00FF0000UL) | ((x << 24) & 0xFF000000ul);
}
inline constexpr uint64_t e_htobe64(uint64_t x) {
    return ((x >> 56) & 0x00000000000000FFull) |
           ((x >> 40) & 0x000000000000FF00ull) |
           ((x >> 24) & 0x0000000000FF0000ull) |
           ((x >> 8) & 0x00000000FF000000ull) |
           ((x << 8) & 0x000000FF00000000ull) |
           ((x << 24) & 0x0000FF0000000000ull) |
           ((x << 40) & 0x00FF000000000000ull) |
           ((x << 56) & 0xFF00000000000000ull);
}
inline constexpr uint16_t e_be16toh(uint16_t x) {
    return ((x >> 8) & 0x00FFu) | ((x << 8) & 0xFF00u);
}
inline constexpr uint32_t e_be32toh(uint32_t x) {
    return ((x >> 24) & 0x000000FFul) | ((x >> 8) & 0x0000FF00UL) |
           ((x << 8) & 0x00FF0000UL) | ((x << 24) & 0xFF000000ul);
}
inline constexpr uint64_t e_be64toh(uint64_t x) {
    return ((x >> 56) & 0x00000000000000FFull) |
           ((x >> 40) & 0x000000000000FF00ull) |
           ((x >> 24) & 0x0000000000FF0000ull) |
           ((x >> 8) & 0x00000000FF000000ull) |
           ((x << 8) & 0x000000FF00000000ull) |
           ((x << 24) & 0x0000FF0000000000ull) |
           ((x << 40) & 0x00FF000000000000ull) |
           ((x << 56) & 0xFF00000000000000ull);
}

static_assert(e_htole16(0xCDEFu) == 0xCDEFu);
static_assert(e_htole32(0x89ABCDEFul) == 0x89ABCDEFul);
static_assert(e_htole64(0x0123456789ABCDEFull) == 0x0123456789ABCDEFull);
static_assert(e_le16toh(0xCDEFu) == 0xCDEFu);
static_assert(e_le32toh(0x89ABCDEFul) == 0x89ABCDEFul);
static_assert(e_le64toh(0x0123456789ABCDEFull) == 0x0123456789ABCDEFull);
static_assert(e_htobe16(0xCDEFu) == 0xEFCDu);
static_assert(e_htobe32(0x89ABCDEFul) == 0xEFCDAB89ul);
static_assert(e_htobe64(0x0123456789ABCDEFull) == 0xEFCDAB8967452301ull);
static_assert(e_be16toh(0xCDEFu) == 0xEFCDu);
static_assert(e_be32toh(0x89ABCDEFul) == 0xEFCDAB89ul);
static_assert(e_be64toh(0x0123456789ABCDEFull) == 0xEFCDAB8967452301ull);

#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

inline constexpr uint16_t e_htobe16(uint16_t x) {
    return x;
}
inline constexpr uint32_t e_htobe32(uint32_t x) {
    return x;
}
inline constexpr uint64_t e_htobe64(uint64_t x) {
    return x;
}
inline constexpr uint16_t e_be16toh(uint16_t x) {
    return x;
}
inline constexpr uint32_t e_be32toh(uint32_t x) {
    return x;
}
inline constexpr uint64_t e_be64toh(uint64_t x) {
    return x;
}
inline constexpr uint16_t e_htobe16(uint16_t x) {
    return ((x >> 8) & 0x00FFu) | ((x << 8) & 0xFF00u);
}
inline constexpr uint32_t e_htobe32(uint32_t x) {
    return ((x >> 24) & 0x000000FFul) | ((x >> 8) & 0x0000FF00UL) |
           ((x << 8) & 0x00FF0000UL) | ((x << 24) & 0xFF000000ul);
}
inline constexpr uint64_t e_htole64(uint64_t x) {
    return ((x >> 56) & 0x00000000000000FFull) |
           ((x >> 40) & 0x000000000000FF00ull) |
           ((x >> 24) & 0x0000000000FF0000ull) |
           ((x >> 8) & 0x00000000FF000000ull) |
           ((x << 8) & 0x000000FF00000000ull) |
           ((x << 24) & 0x0000FF0000000000ull) |
           ((x << 40) & 0x00FF000000000000ull) |
           ((x << 56) & 0xFF00000000000000ull);
}
inline constexpr uint16_t e_le16toh(uint16_t x) {
    return ((x >> 8) & 0x00FFu) | ((x << 8) & 0xFF00u);
}
inline constexpr uint32_t e_le32toh(uint32_t x) {
    return ((x >> 24) & 0x000000FFul) | ((x >> 8) & 0x0000FF00UL) |
           ((x << 8) & 0x00FF0000UL) | ((x << 24) & 0xFF000000ul);
}
inline constexpr uint64_t e_le64toh(uint64_t x) {
    return ((x >> 56) & 0x00000000000000FFull) |
           ((x >> 40) & 0x000000000000FF00ull) |
           ((x >> 24) & 0x0000000000FF0000ull) |
           ((x >> 8) & 0x00000000FF000000ull) |
           ((x << 8) & 0x000000FF00000000ull) |
           ((x << 24) & 0x0000FF0000000000ull) |
           ((x << 40) & 0x00FF000000000000ull) |
           ((x << 56) & 0xFF00000000000000ull);
}

static_assert(e_htobe16(0xCDEFu) == 0xCDEFu);
static_assert(e_htobe32(0x89ABCDEFul) == 0x89ABCDEFul);
static_assert(e_htobe64(0x0123456789ABCDEFull) == 0x0123456789ABCDEFull);
static_assert(e_be16toh(0xCDEFu) == 0xCDEFu);
static_assert(e_be32toh(0x89ABCDEFul) == 0x89ABCDEFul);
static_assert(e_be64toh(0x0123456789ABCDEFull) == 0x0123456789ABCDEFull);
static_assert(e_htole16(0xCDEFu) == 0xEFCDu);
static_assert(e_htole32(0x89ABCDEFul) == 0xEFCDAB89ul);
static_assert(e_htole64(0x0123456789ABCDEFull) == 0xEFCDAB8967452301ull);
static_assert(e_le16toh(0xCDEFu) == 0xEFCDu);
static_assert(e_le32toh(0x89ABCDEFul) == 0xEFCDAB89ul);
static_assert(e_le64toh(0x0123456789ABCDEFull) == 0xEFCDAB8967452301ull);

#else

    #error "What the hell are you running on"

#endif

#endif
