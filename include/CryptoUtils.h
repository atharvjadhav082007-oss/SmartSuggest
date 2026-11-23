#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>

/**
 * @class CryptoUtils
 * @brief Self-contained implementation of SHA-1, HMAC-SHA1, and Base64 encoding.
 * 
 * Demonstrates:
 * - Encapsulation: Hides raw bit manipulation, message padding, and SHA-1 compression rounds.
 * - Zero external dependencies (uses standard C++17 only).
 */
class CryptoUtils {
private:
    static inline uint32_t leftRotate(uint32_t value, size_t count) {
        return (value << count) | (value >> (32 - count));
    }

public:
    /// Computes raw 20-byte SHA-1 digest of input bytes.
    static std::vector<uint8_t> sha1(const uint8_t* data, size_t length) {
        uint32_t h0 = 0x67452301;
        uint32_t h1 = 0xEFCDAB89;
        uint32_t h2 = 0x98BADCFE;
        uint32_t h3 = 0x10325476;
        uint32_t h4 = 0xC3D2E1F0;

        uint64_t totalBits = static_cast<uint64_t>(length) * 8;
        size_t paddedLength = ((length + 8) / 64 + 1) * 64;
        std::vector<uint8_t> msg(paddedLength, 0);
        if (length > 0 && data != nullptr) {
            std::memcpy(msg.data(), data, length);
        }
        msg[length] = 0x80;

        for (int i = 0; i < 8; ++i) {
            msg[paddedLength - 1 - i] = static_cast<uint8_t>((totalBits >> (i * 8)) & 0xFF);
        }

        for (size_t chunk = 0; chunk < paddedLength; chunk += 64) {
            uint32_t w[80];
            for (size_t i = 0; i < 16; ++i) {
                w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(msg[chunk + i * 4 + 3]));
            }
            for (size_t i = 16; i < 80; ++i) {
                w[i] = leftRotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c = h2;
            uint32_t d = h3;
            uint32_t e = h4;

            for (size_t i = 0; i < 80; ++i) {
                uint32_t f = 0, k = 0;
                if (i < 20) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                } else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                } else if (i < 60) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                } else {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t temp = leftRotate(a, 5) + f + e + k + w[i];
                e = d;
                d = c;
                c = leftRotate(b, 30);
                b = a;
                a = temp;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
        }

        std::vector<uint8_t> digest(20);
        uint32_t state[5] = { h0, h1, h2, h3, h4 };
        for (int i = 0; i < 5; ++i) {
            digest[i * 4]     = static_cast<uint8_t>((state[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<uint8_t>(state[i] & 0xFF);
        }
        return digest;
    }

    /// Computes HMAC-SHA1 of data using key, returning 20-byte digest.
    static std::vector<uint8_t> hmacSha1(const std::string& key, const std::string& data) {
        const size_t blockSize = 64;
        std::vector<uint8_t> k(blockSize, 0);

        if (key.size() > blockSize) {
            auto hashed = sha1(reinterpret_cast<const uint8_t*>(key.data()), key.size());
            std::memcpy(k.data(), hashed.data(), hashed.size());
        } else {
            std::memcpy(k.data(), key.data(), key.size());
        }

        std::vector<uint8_t> oPad(blockSize, 0x5c);
        std::vector<uint8_t> iPad(blockSize, 0x36);
        for (size_t i = 0; i < blockSize; ++i) {
            oPad[i] ^= k[i];
            iPad[i] ^= k[i];
        }

        std::vector<uint8_t> innerMsg = iPad;
        innerMsg.insert(innerMsg.end(), data.begin(), data.end());
        auto innerHash = sha1(innerMsg.data(), innerMsg.size());

        std::vector<uint8_t> outerMsg = oPad;
        outerMsg.insert(outerMsg.end(), innerHash.begin(), innerHash.end());
        return sha1(outerMsg.data(), outerMsg.size());
    }

    /// Encodes arbitrary byte buffer into standard Base64 string.
    static std::string base64Encode(const uint8_t* data, size_t length) {
        static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string res;
        res.reserve(((length + 2) / 3) * 4);

        size_t i = 0;
        while (i < length) {
            size_t rem = length - i;
            uint32_t octet_a = data[i++];
            uint32_t octet_b = (rem > 1) ? data[i++] : 0;
            uint32_t octet_c = (rem > 2) ? data[i++] : 0;

            uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

            res.push_back(tbl[(triple >> 18) & 0x3F]);
            res.push_back(tbl[(triple >> 12) & 0x3F]);
            if (rem == 1) {
                res.push_back('=');
                res.push_back('=');
            } else if (rem == 2) {
                res.push_back(tbl[(triple >> 6) & 0x3F]);
                res.push_back('=');
            } else {
                res.push_back(tbl[(triple >> 6) & 0x3F]);
                res.push_back(tbl[triple & 0x3F]);
            }
        }
        return res;
    }

    /// Convenience wrapper for base64 encoding vector
    static std::string base64Encode(const std::vector<uint8_t>& data) {
        return base64Encode(data.data(), data.size());
    }
};

#endif // CRYPTOUTILS_H
