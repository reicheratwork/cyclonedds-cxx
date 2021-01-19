/*
 * Copyright(c) 2020 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef CDR_STREAM_HPP_
#define CDR_STREAM_HPP_

#include "cdr_dummys.hpp"
#include "dds/ddsrt/endian.h"
#include <org/eclipse/cyclonedds/core/type_helpers.hpp>
#include <stdint.h>
#include <stdexcept>
#include <dds/core/macros.hpp>

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void byte_swap(T& toswap) {
    union { T a; uint16_t u2; uint32_t u4; uint64_t u8; } u;
    u.a = toswap;
    switch (sizeof(T)) {
    case 1:
        break;
    case 2:
        u.u2 = static_cast<uint16_t>((u.u2 & 0xFF00) >> 8) | static_cast<uint16_t>((u.u2 & 0x00FF) << 8);
        break;
    case 4:
        u.u4 = static_cast<uint32_t>((u.u4 & 0xFFFF0000) >> 16) | static_cast<uint32_t>((u.u4 & 0x0000FFFF) << 16);
        u.u4 = static_cast<uint32_t>((u.u4 & 0xFF00FF00) >> 8) | static_cast<uint32_t>((u.u4 & 0x00FF00FF) << 8);
        break;
    case 8:
        u.u8 = static_cast<uint64_t>((u.u8 & 0xFFFFFFFF00000000) >> 32) | static_cast<uint64_t>((u.u8 & 0x00000000FFFFFFFF) << 32);
        u.u8 = static_cast<uint64_t>((u.u8 & 0xFFFF0000FFFF0000) >> 16) | static_cast<uint64_t>((u.u8 & 0x0000FFFF0000FFFF) << 16);
        u.u8 = static_cast<uint64_t>((u.u8 & 0xFF00FF00FF00FF00) >> 8) | static_cast<uint64_t>((u.u8 & 0x00FF00FF00FF00FF) << 8);
        break;
    default:
        throw std::invalid_argument(std::string("attempted byteswap on variable of invalid size: ") + std::to_string(sizeof(T)));
    }
    toswap = u.a;
}

enum class endianness {
    little_endian = DDSRT_LITTLE_ENDIAN,
    big_endian = DDSRT_BIG_ENDIAN
};

constexpr endianness native_endianness() { return endianness(DDSRT_ENDIAN); }

class OMG_DDS_API cdr_stream {
public:
    cdr_stream(endianness end, size_t max_align) : m_stream_endianness(end), m_max_alignment(max_align) { ; }

    size_t position() const { return m_position; }

    void reset_position() { m_position = 0; m_current_alignment = 0; }

    void set_buffer(void* toset);

    void set_stream_endianness(endianness toset) { m_stream_endianness = toset; }

    void align(size_t newalignment, bool add_zeroes);

protected:

    template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
    void transfer_and_swap(const T& from, T& to, bool sw) {

        to = from;

        if (sw)
            byte_swap(to);

        m_position += sizeof(T);
    }

    endianness m_stream_endianness, //the endianness of the stream
        m_local_endianness = native_endianness();  //the local endianness
    size_t m_position = 0,  //the current offset position in the stream
        m_max_alignment,  //the maximum bytes that can be aligned to
        m_current_alignment = 1;  //the current alignment
    char* m_buffer = NULL;  //the current buffer in use
};

#endif
