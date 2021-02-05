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
#ifndef BASIC_CDR_SERIALIZATION_HPP_
#define BASIC_CDR_SERIALIZATION_HPP_

#include "cdr_stream.hpp"
#include <org/eclipse/cyclonedds/core/type_helpers.hpp>
#include <stdexcept>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

class basic_cdr_stream : public cdr_stream {
public:
  basic_cdr_stream(endianness end = native_endianness()) : cdr_stream(end, 8) { ; }

  //friend function decls
};

//primitive functions

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void read(basic_cdr_stream &str, T& toread)
{
  str.align(sizeof(T), false);

  transfer_and_swap(
    *(reinterpret_cast<const T*>(str.get_cursor())),
    toread,
    str.swap_endianness());

  str.incr_position(sizeof(T));
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void write(basic_cdr_stream& str, const T& towrite)
{
  str.align(sizeof(T), true);

  transfer_and_swap(
    towrite,
    *(reinterpret_cast<T*>(str.get_cursor())),
    str.swap_endianness());

  str.incr_position(sizeof(T));
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void incr(basic_cdr_stream& str, const T& toincr)
{
  (void)toincr;

  str.align(sizeof(T), false);

  str.incr_position(sizeof(T));
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void max_size(basic_cdr_stream& str, const T& max_sz)
{
  if (str.position() == SIZE_MAX)
    return;

  incr(str, max_sz);
}

//enum functions

template<typename T, typename = std::enable_if_t<std::is_enum<T>::value> >
void read(basic_cdr_stream& str, T& toread) {
  read(str, uint32_t(toread));
}

template<typename T, typename = std::enable_if_t<std::is_enum<T>::value> >
void write(basic_cdr_stream& str, const T& towrite) {
  write(str, uint32_t(towrite));
}

template<typename T, typename = std::enable_if_t<std::is_enum<T>::value> >
void incr(basic_cdr_stream& str, const T& toincr) {
  incr(str, uint32_t(toincr));
}

template<typename T, typename = std::enable_if_t<std::is_enum<T>::value> >
void max_size(basic_cdr_stream& str, const T& max_sz) {
  max_size(str, uint32_t(max_sz));
}

//length functions

void write_length(basic_cdr_stream& str, size_t length, size_t bound)
{
  //throw an exception if we attempt to write a length field in excess of the supplied bound
  if (bound &&
    length > bound)
    throw 1;  //replace throw

  write(str, uint32_t(length));
}

void read_length(basic_cdr_stream& str, uint32_t& length)
{
  read(str, length);
}

void incr_length(basic_cdr_stream& str, size_t length, size_t bound)
{
  //throw an exception if we attempt to move the cursor for a length field in excess of the supplied bound
  if (bound &&
    length > bound)
    throw 2;  //replace throw

  incr(str, uint32_t(length));
}

template<typename T, size_t N>
void read_vec_resize(basic_cdr_stream& str, idl_bounded_sequence<T, N>& toread, uint32_t& seq_length)
{
  //the length of the entries contained in the stream is read, but...
  read_length(str, seq_length);

  //the container is only enlarged upto its maximum size
  auto read_length = std::min<size_t>(seq_length, N ? N : SIZE_MAX);

  toread.resize(read_length);
}

//string functions

template<size_t N>
void read(basic_cdr_stream& str, idl_bounded_string<N>& toread)
{
  uint32_t string_length = 0;

  read_length(str, string_length);

  auto cursor = str.get_cursor();
  toread.assign(cursor, cursor + std::min<size_t>(string_length - 1, N ? N : SIZE_MAX));  //remove 1 for terminating NULL

  str.incr_position(string_length);
}

template<size_t N>
void write(basic_cdr_stream& str, const idl_bounded_string<N>& towrite)
{
  size_t string_length = towrite.length() + 1;  //add 1 extra for terminating NULL

  write_length(str, string_length, N);

  //no check on string length necessary after this since it is already checked in write_length

  memcpy(str.get_cursor(), towrite.c_str(), string_length);

  str.incr_position(string_length);
}

template<size_t N>
void incr(basic_cdr_stream& str, const idl_bounded_string<N>& toincr)
{
  size_t string_length = toincr.length() + 1;  //add 1 extra for terminating NULL

  incr_length(str, string_length, N);

  //no check on string length necessary after this since it is already checked in incr_length

  str.incr_position(string_length);
}

template<size_t N>
void max_size(basic_cdr_stream& str, const idl_bounded_string<N>& max_sz)
{
  (void)max_sz;

  if (str.position() == SIZE_MAX)
    return;

  if (N == 0)
  {
    //unbounded string, theoretical length unlimited
    str.position(SIZE_MAX);
  }
  else
  {
    //length field
    max_size(uint32_t(0));

    //bounded string, length maximum N+1 characters
    str.incr_position(N + 1);
  }
}

//array functions

//arrays of primitives

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void read(basic_cdr_stream& str, idl_array<T, N>& toread)
{
  str.align(sizeof(T), false);

  memcpy(toread.data(), str.get_cursor(), N*sizeof(T));

  if (sizeof(T) > 1 &&
    str.swap_endianness())
  {
    for (auto& e : toread)
      byte_swap(e);
  }

  str.incr_position(N * sizeof(T));
}

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void write(basic_cdr_stream& str, const idl_array<T, N>& towrite)
{
  str.align(sizeof(T), false);

  memcpy(str.get_cursor(), towrite.data(), N * sizeof(T));

  if (sizeof(T) > 1 &&
    str.swap_endianness())
  {
    for (size_t i = 0; i < N; i++)
    {
      byte_swap(*static_cast<T*>(str.get_cursor()));
      str.incr_position(sizeof(T));
    }
  }
  else
  {
    str.incr_position(N * sizeof(T));
  }
}

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void incr(basic_cdr_stream& str, const idl_array<T, N>& toincr)
{
  str.align(sizeof(T), false);

  str.incr_position(N * sizeof(T));
}

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void max_size(basic_cdr_stream& str, const idl_array<T, N>& max_sz)
{
  (void)max_sz;

  if (str.position() == SIZE_MAX)
    return;

  str.align(sizeof(T), false);

  str.incr_position(N * sizeof(T));
}

//arrays of complex types

template<typename T, size_t N>
void read(basic_cdr_stream& str, idl_array<T, N>& toread)
{
  for (auto& e : toread)
    read(str, e);
}

template<typename T, size_t N>
void write(basic_cdr_stream& str, const idl_array<T, N>& towrite)
{
  for (const auto& e : towrite)
    write(str, e);
}

template<typename T, size_t N>
void incr(basic_cdr_stream& str, const idl_array<T, N>& toincr)
{
  for (const auto& e : toincr)
    incr(str, e);
}

template<typename T, size_t N>
void max_size(basic_cdr_stream& str, const idl_array<T, N>& max_sz)
{
  if (str.position() == SIZE_MAX)
    return;

  for (const auto & e:max_sz)
    max_size(str, e);
}

//sequence functions

//sequences of primitives

//implement special read function for sequences of bools, since the std::vector<bool> is also specialized, and direct copy will not give a happy result

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void read(basic_cdr_stream& str, idl_bounded_sequence<T, N>& toread) {
  uint32_t seq_length = 0;  //this is the sequence length retrieved from the stream, not the number of entities to be written to the sequence object
  read_vec_resize(str, toread, seq_length);

  align(str, sizeof(T), true);

  memcpy(toread.data(), str.get_cursor(), toread.size() * sizeof(T));

  if (sizeof(T) > 1 &&
    str.swap_endianness())
  {
    for (auto & e:toread)
      byte_swap(e);
  }

  str.incr_position(seq_length * sizeof(T));
}

//implement special write function for sequences of bools, since the std::vector<bool> is also specialized, and direct copy will not give a happy result

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void write(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& towrite) {
  write_length(str, towrite.size(), N);

  //no check on length necessary after this point, it is done in the write_length function

  str.align(sizeof(T), true);

  memcpy(str.get_cursor(), towrite.data(), towrite.size() * sizeof(T));

  if (sizeof(T) > 1 &&
    str.swap_endianness())
  {
    for (size_t i = 0; i < towrite.size(); i++)
    {
      byte_swap(*static_cast<T*>(str.get_cursor()));
      str.incr_position(sizeof(T));
    }
  }
  else
  {
    str.incr_position(towrite.size() * sizeof(T));
  }
}

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void incr(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& toincr) {
  incr_length(str, toincr.size(), N);

  //no check on length necessary after this point, it is done in the incr_length function

  str.align(sizeof(T), false);

  str.incr_position(toincr.size() * sizeof(T));
}

template<typename T, size_t N, typename = std::enable_if_t<std::is_arithmetic<T>::value> >
void max_size(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& max_sz) {
  if (m_position == SIZE_MAX)
    return;

  if (N == 0)
  {
    str.position(SIZE_MAX);
  }
  else
  {
    max_size(str, uint32_t(0));

    str.align(sizeof(T), false);

    str.incr_position(N * sizeof(T));
  }
}

//sequences of complex types

template<typename T, size_t N>
void read(basic_cdr_stream& str, idl_bounded_sequence<T, N>& toread) {
  uint32_t seq_length = 0;  //this is the sequence length of entities in the stream, not the number of entities to be read to the sequence
  read_vec_resize(str, toread, seq_length);

  for (auto& e : toread)
    read(str, e);

  //dummy reads
  for (size_t i = N; i < static_cast<size_t>(seq_length); i++)
  {
    T dummy;
    read(str, dummy);
  }
}

template<typename T, size_t N>
void write(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& towrite) {
  write_length(str, towrite.size(), N);

  //no check on length necessary after this point, it is done in the write_length function

  for (const auto& e : towrite)
    write(str, e);
}

template<typename T, size_t N>
void incr(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& toincr) {
  incr_length(str, toincr.size(), N);

  //no check on length necessary after this point, it is done in the incr_length function

  for (const auto& e : toincr)
    incr(str, e);
}

template<typename T, size_t N>
void max_size(basic_cdr_stream& str, const idl_bounded_sequence<T, N>& max_sz)
{
  (void)max_sz;

  if (m_position == SIZE_MAX)
    return;

  if (N == 0)
  {
    str.position(SIZE_MAX);
  }
  else
  {
    max_size(str, uint32_t(0));

    T dummy;
    for (size_t i = 0; i < N; i++)
      max_size(str, T);
  }
}

#endif
