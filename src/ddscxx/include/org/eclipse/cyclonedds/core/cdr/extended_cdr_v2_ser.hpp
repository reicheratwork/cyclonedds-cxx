/*
 * Copyright(c) 2021 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#ifndef EXTENDED_CDR_SERIALIZATION_V2_HPP_
#define EXTENDED_CDR_SERIALIZATION_V2_HPP_

#include "basic_cdr_ser.hpp"
#include <org/eclipse/cyclonedds/core/type_helpers.hpp>
#include <dds/core/Exception.hpp>
#include <cassert>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

/**
 * @brief
 * Implementation of the extended cdr version1 stream.
 *
 * This type of cdr stream has a maximum alignment of 8 bytes.
 */
class xcdr_v2_stream : public cdr_stream {
public:
  /**
   * @brief
   * Constructor.
   *
   * Basically a pass through for the cdr_stream base class.
   *
   * @param[in] end The endianness to set for the data stream, default to the local system endianness.
   * @param[in] ignore_faults Bitmask for ignoring faults, can be composed of bit fields from the serialization_status enumerator.
   */
  xcdr_v2_stream(endianness end = native_endianness(), uint64_t ignore_faults = 0x0) : cdr_stream(end, 8, ignore_faults) { ; }

  bool structure_is_list(extensibility ext) const;

  entity_properties read_header();

  void push_entity(const entity_properties &props);

  void pop_entity();

  void move_header(const entity_properties &props);

  void write_d_header(const entity_properties &props);

  void write_em_header(const entity_properties &props);

  void finish_d_header();

  void finish_em_header();

private:

    static const uint32_t bytes_1         = uint32_t(0),
                          bytes_2         = uint32_t(1) << 28,
                          bytes_4         = uint32_t(2) << 28,
                          bytes_8         = uint32_t(3) << 28,
                          nextint         = uint32_t(4) << 28,
                          nextint_times_1 = uint32_t(5) << 28,
                          nextint_times_4 = uint32_t(6) << 28,
                          nextint_times_8 = uint32_t(7) << 28,
                          lc_mask         = uint32_t(7) << 28,
                          id_mask         = 0x0FFFFFFF,
                          must_understand = uint32_t(1) << 31;
};

/**
 * @brief
 * Primitive type stream manipulation functions.
 *
 * These are "endpoints" for write functions, since composit
 * (sequence/array/constructed type) functions will decay to these
 * calls.
 */

/**
 * @brief
 * Primitive type read function.
 *
 * Aligns the stream to the alignment of type T.
 * Reads the value from the current position of the stream str into
 * toread, will swap bytes if necessary.
 * Moves the cursor of the stream by the size of T.
 * This function is only enabled for arithmetic types and enums.
 *
 * @param[in, out] str The stream which is read from.
 * @param[out] toread The variable to read into.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value && !std::is_enum<T>::value, bool> = true >
void read(xcdr_v2_stream &str, T& toread, const entity_properties &props = entity_properties())
{
  (void) props;

  if (str.abort_status())
    return;

  char *cursor = str.get_cursor();
  str.align(sizeof(T), false);

  assert(cursor);
  transfer_and_swap(*(reinterpret_cast<const T*>(cursor)), toread, str.swap_endianness());
  str.incr_position(sizeof(T));
}

/**
 * @brief
 * Primitive type write function.
 *
 * Aligns str to the type to be written.
 * Writes towrite to str.
 * Swaps bytes written to str if the endiannesses do not match up.
 * Moves the cursor of str by the size of towrite.
 * This function is only enabled for arithmetic types.
 *
 * @param[in, out] str The stream which is written to.
 * @param[in] towrite The variable to write.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value && !std::is_enum<T>::value, bool> = true >
void write(xcdr_v2_stream& str, const T& towrite, const entity_properties &props = entity_properties())
{
  if (str.abort_status())
    return;

  if (str.structure_is_list(props.parent_extensibility))
    str.write_header_fixed(props, sizeof(T));

  char *cursor = str.get_cursor();
  str.align(sizeof(T), true);

  assert(cursor);
  transfer_and_swap(towrite, *(reinterpret_cast<T*>(cursor)), str.swap_endianness());
  str.incr_position(sizeof(T));
}

/**
 * @brief
 * Primitive type cursor move function.
 *
 * Used in determining the size of a type when written to the stream.
 * Aligns str to the size of toincr.
 * Moves the cursor of str by the size of toincr.
 * This function is only enabled for arithmetic types.
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] toincr The variable to move the cursor by, no contents of this variable are used, it is just used to determine the template.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value && !std::is_enum<T>::value, bool> = true >
void move(xcdr_v2_stream& str, const T& toincr, const entity_properties &props = entity_properties())
{
  (void)toincr;

  if (str.abort_status())
    return;

  if (str.structure_is_list(props.parent_extensibility))
    str.move_header(props);

  str.align(sizeof(T), false);

  str.incr_position(sizeof(T));
}

/**
 * @brief
 * Primitive type max stream move function.
 *
 * Used in determining the maximum stream size of a constructed type.
 * Moves the cursor to the maximum position it could occupy after
 * writing max_sz to the stream.
 * Is in essence the same as the primitive type cursor move function,
 * but additionally checks for whether the cursor it at the "end",
 * which may happen if unbounded members (strings/sequences/...)
 * are part of the constructed type.
 * This function is only enabled for arithmetic types.
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] max_sz The variable to move the cursor by, no contents of this variable are used, it is just used to determine the template.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value && !std::is_enum<T>::value, bool> = true >
void max(xcdr_v2_stream& str, const T& max_sz, const entity_properties &props = entity_properties())
{
  if (str.abort_status() ||
      str.position() == SIZE_MAX)
    return;

  move(str, max_sz, props);
}

/**
 * @brief
 * Enumerated type stream manipulation functions.
 * Since enumerated types are represented by a uint32_t in basic CDR streams
 * they just loop through to writing uint32_t versions of the enum.
 *
 * These are "endpoints" for write functions, since compound
 * (sequence/array/constructed type) functions will decay to these
 * calls.
 */

/**
 * @brief
 * Reads the value of the enum from the stream.
 *
 * @param[in, out] str The stream which is read from.
 * @param[out] toread The variable to read into.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_enum<T>::value && !std::is_arithmetic<T>::value, bool> = true >
void read(xcdr_v2_stream& str, T& toread, const entity_properties &props = entity_properties()) {
  switch (props.member_bit_bound)
  {
    case bb_8_bits:
      read(str, *reinterpret_cast<uint8_t*>(&toread), props);
      break;
    case bb_16_bits:
      read(str, *reinterpret_cast<uint16_t*>(&toread), props);
      break;
    case bb_32_bits:
      read(str, *reinterpret_cast<uint32_t*>(&toread), props);
      break;
  }
}

/**
 * @brief
 * Writes the value of the enum to the stream.
 *
 * @param [in, out] str The stream which is written to.
 * @param [in] towrite The variable to write.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_enum<T>::value && !std::is_arithmetic<T>::value, bool> = true >
void write(xcdr_v2_stream& str, const T& towrite, const entity_properties &props = entity_properties()) {
  switch (props.member_bit_bound)
  {
    case bb_8_bits:
      write(str, static_cast<int8_t>(towrite), props);
      break;
    case bb_16_bits:
      write(str, static_cast<int16_t>(towrite), props);
      break;
    case bb_32_bits:
      write(str, static_cast<int32_t>(towrite), props);
      break;
  }
}

/**
 * @brief
 * Moves the cursor of the stream by the size the enum would take up.
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] toincr The variable to move the cursor by, no contents of this variable are used, it is just used to determine the template.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_enum<T>::value && !std::is_arithmetic<T>::value, bool> = true >
void move(xcdr_v2_stream& str, const T& toincr, const entity_properties &props = entity_properties()) {
  switch (props.member_bit_bound)
  {
    case bb_8_bits:
      move(str, static_cast<int8_t>(toincr), props);
      break;
    case bb_16_bits:
      move(str, static_cast<int16_t>(toincr), props);
      break;
    case bb_32_bits:
      move(str, static_cast<int32_t>(toincr), props);
      break;
  }
}

/**
 * @brief
 * Moves the cursor of the stream by the size the enum would take up (maximum size version).
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] max_sz The variable to move the cursor by, no contents of this variable are used, it is just used to determine the template.
 * @param[in] props The properties of the parameter.
 */
template<typename T, std::enable_if_t<std::is_enum<T>::value && !std::is_arithmetic<T>::value, bool> = true >
void max(xcdr_v2_stream& str, const T& max_sz, const entity_properties &props = entity_properties()) {
  switch (props.member_bit_bound)
  {
    case bb_8_bits:
      max(str,static_cast<int8_t>(max_sz), props);
      break;
    case bb_16_bits:
      max(str, static_cast<int16_t>(max_sz), props);
      break;
    case bb_32_bits:
      max(str, static_cast<int32_t>(max_sz), props);
      break;
  }
}

 /**
 * @brief
 * String type stream manipulation functions
 *
 * These are "endpoints" for write functions, since compound
 * (sequence/array/constructed type) functions will decay to these
 * calls.
 */

/**
 * @brief
 * Bounded string read function.
 *
 * Reads the length from str, but then initializes toread with at most N characters from it.
 * It does move the cursor by length read, since that is the number of characters in the stream.
 * If N is 0, then the string is taken to be unbounded.
 *
 * @param[in, out] str The stream to read from.
 * @param[out] toread The string to read to.
 * @param[in] N The maximum number of characters to read from the stream.
 * @param[in] props The properties of the parameter.
 */
template<typename T>
void read_string(xcdr_v2_stream& str, T& toread, size_t N, const entity_properties &props = entity_properties())
{
  (void) props;

  if (str.abort_status())
    return;

  uint32_t string_length = 0;

  read(str, string_length);

  if (N &&
      string_length > N &&
      str.status(serialization_status::read_bound_exceeded))
      return;

  auto cursor = str.get_cursor();
  toread.assign(cursor, cursor + std::min<size_t>(string_length - 1, N ? N : SIZE_MAX));  //remove 1 for terminating NULL

  str.incr_position(string_length);

  //aligned to chars
  str.alignment(1);
}

/**
 * @brief
 * Bounded string write function.
 *
 * Attempts to write the length of towrite to str, where the bound is checked.
 * Then writes the contents of towrite to str.
 * If N is 0, then the string is taken to be unbounded.
 *
 * @param[in, out] str The stream to write to.
 * @param[in] towrite The string to write.
 * @param[in] N The maximum number of characters to write to the stream.
 * @param[in] props The properties of the parameter.
 */
template<typename T>
void write_string(xcdr_v2_stream& str, const T& towrite, size_t N, const entity_properties &props = entity_properties())
{
  if (str.abort_status())
    return;

  if (str.structure_is_list(props.parent_extensibility))
    str.push_entity(props);

  size_t string_length = towrite.length() + 1;  //add 1 extra for terminating NULL

  if (N &&
      string_length > N) {
    if (str.status(serialization_status::write_bound_exceeded))
      return;
  }

  write(str, uint32_t(string_length));

  memcpy(str.get_cursor(), towrite.c_str(), string_length);

  str.incr_position(string_length);

  //aligned to chars
  str.alignment(1);

  if (str.structure_is_list(props.parent_extensibility))
    str.pop_entity();
}

/**
 * @brief
 * Bounded string cursor move function.
 *
 * Attempts to move the cursor for the length field, where the bound is checked.
 * Then moves the cursor for the length of the string.
 * If N is 0, then the string is taken to be unbounded.
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] toincr The string used to move the cursor.
 * @param[in] N The maximum number of characters in the string which the stream is moved by.
 * @param[in] props The properties of the parameter.
 */
template<typename T>
void move_string(xcdr_v2_stream& str, const T& toincr, size_t N, const entity_properties &props = entity_properties())
{
  if (str.abort_status())
    return;

  size_t string_length = toincr.length() + 1;  //add 1 extra for terminating NULL

  if (N &&
      string_length > N) {
    if (str.status(serialization_status::move_bound_exceeded))
      return;
  }

  if (str.structure_is_list(props.parent_extensibility))
    str.move_header(props);

  move(str, uint32_t(string_length));

  str.incr_position(string_length);

  //aligned to chars
  str.alignment(1);
}

/**
 * @brief
 * Bounded string cursor max move function.
 *
 * Similar to the string move function, with the additional checks that no move
 * is done if the cursor is already at its maximum position, and that the cursor
 * is set to its maximum position if the bound is equal to 0 (unbounded).
 *
 * @param[in, out] str The stream whose cursor is moved.
 * @param[in] max_sz The string used to move the cursor.
 * @param[in] N The maximum number of characters in the string which the stream is moved by.
 * @param[in] props The properties of the parameter.
 */
template<typename T>
void max_string(xcdr_v2_stream& str, const T& max_sz, size_t N, const entity_properties &props = entity_properties())
{
  (void)max_sz;

  if (str.abort_status() ||
      str.position() == SIZE_MAX)
    return;


  if (N == 0)
  {
    //unbounded string, theoretical length unlimited
    str.position(SIZE_MAX);
  }
  else
  {
    //add header field length if in a parameter list
    if (str.structure_is_list(props.parent_extensibility))
      str.move_header(props);

    //length field
    max(str, uint32_t(0));

    //bounded string, length maximum N+1 characters
    str.incr_position(N + 1);

    //aligned to chars
    str.alignment(1);
  }
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */
#endif