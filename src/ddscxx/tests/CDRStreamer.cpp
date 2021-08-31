/*
 * Copyright(c) 2006 to 2021 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include "dds/dds.hpp"
#include <gtest/gtest.h>
#include "CdrDataModels.hpp"
#include "CdrDataModels_pragma.hpp"

using namespace org::eclipse::cyclonedds::core::cdr;
using namespace CDR_testing;

typedef std::vector<unsigned char> bytes;

/**
 * Fixture for the DataWriter tests
 */
class CDRStreamer : public ::testing::Test
{
public:
    enum stream_type {
      basic,
      xcdr_v1,
      xcdr_v2
    };

    basic_cdr_stream b;
    xcdr_v1_stream x_1;
    xcdr_v2_stream x_2;

    bytes BS_basic_normal {
        0, 1, 226, 64 /*basicstruct.l*/,
        'g' /*basicstruct.c*/,
        0, 0, 0 /*padding bytes (3)*/,
        0, 0, 0, 7 /*basicstruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*basicstruct.str.c_str*/,
        0, 0, 0, 0, 0 /*padding bytes (5)*/,
        64, 132, 114, 145, 104, 114, 176, 33 /*basicstruct.d*/};
    bytes BS_basic_key {'g'/*basicstruct.c*/};
    /*xcdrv2 is max aligned to 4 bytes*/
    bytes BS_xcdrv2_normal {
        0, 1, 226, 64 /*basicstruct.l*/,
        'g' /*basicstruct.c*/,
        0, 0, 0 /*padding bytes (3)*/,
        0, 0, 0, 7 /*basicstruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*basicstruct.str.c_str*/,
        0 /*padding bytes (1)*/,
        64, 132, 114, 145, 104, 114, 176, 33 /*basicstruct.d*/};

    bytes AS_xcdr_v2_normal {
        0, 0, 0, 28/*dheader*/,
        0, 1, 226, 64 /*appendablestruct.l*/,
        'g' /*appendablestruct.c*/,
        0, 0, 0 /*padding bytes (3)*/,
        0, 0, 0, 7 /*appendablestruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*appendablestruct.str.c_str*/,
        0 /*padding bytes (1)*/,
        64, 132, 114, 145, 104, 114, 176, 33 /*appendablestruct.d*/};
    bytes AS_xcdr_v2_key {
        0, 0, 0, 1/*dheader*/,
        'g'/*appendablestruct.c*/};

    CDRStreamer() :
        b(endianness::big_endian),
        x_1(endianness::big_endian),
        x_2(endianness::big_endian)
    {
    }

    template<typename T>
    void VerifyWrite(const T& in, const bytes &out, stream_type stream, bool as_key);

    template<typename T>
    void VerifyRead(const bytes &in, const T& out, stream_type stream, bool as_key);

    template<typename T>
    void VerifyReadOneDeeper(const bytes &in, const T& out, stream_type stream, bool as_key);

    void SetUp() { }

    void TearDown() { }

};

template<typename T>
void CDRStreamer::VerifyWrite(const T& in, const bytes &out, stream_type stream, bool as_key)
{
  bytes buffer;
  switch (stream) {
    case basic:
      b.reset_position();
      move(b, in, as_key);
      buffer.resize(b.position());
      b.set_buffer(buffer.data());
      write(b, in, as_key);
      break;
    case xcdr_v1:
      x_1.reset_position();
      move(x_1, in, as_key);
      buffer.resize(x_1.position());
      x_1.set_buffer(buffer.data());
      write(x_1, in, as_key);
      break;
    case xcdr_v2:
      x_2.reset_position();
      move(x_2, in, as_key);
      buffer.resize(x_2.position());
      x_2.set_buffer(buffer.data());
      write(x_2, in, as_key);
      break;
  }

  ASSERT_EQ(buffer, out);
}

template<typename T>
void CDRStreamer::VerifyRead(const bytes &in, const T& out, stream_type stream, bool as_key)
{
  bytes incopy(in);
  T buffer;
  switch (stream) {
    case basic:
      b.set_buffer(incopy.data());
      read(b, buffer, as_key);
      break;
    case xcdr_v1:
      x_1.set_buffer(incopy.data());
      read(x_1, buffer, as_key);
      break;
    case xcdr_v2:
      x_2.set_buffer(incopy.data());
      read(x_2, buffer, as_key);
      break;
  }

  if (as_key)
    ASSERT_EQ(buffer.c(), out.c());
  else
    ASSERT_EQ(buffer, out);
}

template<typename T>
void CDRStreamer::VerifyReadOneDeeper(const bytes &in, const T& out, stream_type stream, bool as_key)
{
  bytes incopy(in);
  T buffer;
  switch (stream) {
    case basic:
      b.set_buffer(incopy.data());
      read(b, buffer, as_key);
      break;
    case xcdr_v1:
      x_1.set_buffer(incopy.data());
      read(x_1, buffer, as_key);
      break;
    case xcdr_v2:
      x_2.set_buffer(incopy.data());
      read(x_2, buffer, as_key);
      break;
  }

  if (as_key) {
    ASSERT_EQ(buffer.c().size(), out.c().size());
    for (size_t i = 0; i < buffer.c().size() && i < out.c().size(); i++)
      ASSERT_EQ(buffer.c()[i].base_c(), out.c()[i].base_c());
  } else {
    ASSERT_EQ(buffer, out);
  }
}

#define read_test(test_struct, normal_bytes, key_bytes, stream_method)\
VerifyRead(normal_bytes, test_struct, stream_method, false);\
VerifyRead(key_bytes, test_struct, stream_method, true);

#define read_deeper_test(test_struct, normal_bytes, key_bytes, stream_method)\
VerifyRead(normal_bytes, test_struct, stream_method, false);\
VerifyReadOneDeeper(key_bytes, test_struct, stream_method, true);

#define write_test(test_struct, normal_bytes, key_bytes, stream_method)\
VerifyWrite(test_struct, normal_bytes, stream_method, false);\
VerifyWrite(test_struct, key_bytes, stream_method, true);

#define readwrite_test(test_struct, normal_bytes, key_bytes, stream_method)\
read_test(test_struct, normal_bytes, key_bytes, stream_method)\
write_test(test_struct, normal_bytes, key_bytes, stream_method)

#define readwrite_deeper_test(test_struct, normal_bytes, key_bytes, stream_method)\
read_deeper_test(test_struct, normal_bytes, key_bytes, stream_method)\
write_test(test_struct, normal_bytes, key_bytes, stream_method)

#define stream_test(test_struct, cdr_normal_bytes, cdr_key_bytes, xcdr_v1_normal_bytes, xcdr_v1_key_bytes, xcdr_v2_normal_bytes, xcdr_v2_key_bytes)\
readwrite_test(test_struct, cdr_normal_bytes, cdr_key_bytes, basic)\
readwrite_test(test_struct, xcdr_v1_normal_bytes, xcdr_v1_key_bytes, xcdr_v1)\
readwrite_test(test_struct, xcdr_v2_normal_bytes, xcdr_v2_key_bytes, xcdr_v2)

#define stream_deeper_test(test_struct, cdr_normal_bytes, cdr_key_bytes, xcdr_v1_normal_bytes, xcdr_v1_key_bytes, xcdr_v2_normal_bytes, xcdr_v2_key_bytes)\
readwrite_deeper_test(test_struct, cdr_normal_bytes, cdr_key_bytes, basic)\
readwrite_deeper_test(test_struct, xcdr_v1_normal_bytes, xcdr_v1_key_bytes, xcdr_v1)\
readwrite_deeper_test(test_struct, xcdr_v2_normal_bytes, xcdr_v2_key_bytes, xcdr_v2)

/*verifying reads/writes of a basic struct*/

TEST_F(CDRStreamer, cdr_basic)
{
  basicstruct BS(123456, 'g', "abcdef", 654.321);

  stream_test(BS, BS_basic_normal, BS_basic_key, BS_basic_normal, BS_basic_key, BS_xcdrv2_normal, BS_basic_key)
}

/*verifying reads/writes of an appendable struct*/

TEST_F(CDRStreamer, cdr_appendable)
{
  appendablestruct AS(123456, 'g', "abcdef", 654.321);

  stream_test(AS, BS_basic_normal, BS_basic_key, BS_basic_normal, BS_basic_key, AS_xcdr_v2_normal, AS_xcdr_v2_key)
}

/*verifying reads/writes of a mutable struct*/

TEST_F(CDRStreamer, cdr_mutable)
{
  mutablestruct MS(123456, 'g', "abcdef", 654.321);

  bytes MS_xcdr_v1_normal {
      64, 7, 0, 4 /*mutablestruct.l.mheader*/,
      0, 1, 226, 64 /*mutablestruct.l*/,
      64, 5, 0, 1 /*mutablestruct.g.mheader*/,
      'g' /*mutablestruct.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 1, 0, 8 /*mutablestruct.str.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 3, 0, 0, 0, 11 /*mutablestruct.str.mheader (extended)*/,
      0, 0, 0, 7 /*mutablestruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*mutablestruct.str.c_str*/,
      0 /*padding bytes (1)*/,
      64, 1, 0, 12 /*mutablestruct.d.mheader*/,
      0, 0, 0, 0 /*padding bytes (4)*/,
      64, 132, 114, 145, 104, 114, 176, 33 /*mutablestruct.d*/,
      127, 2, 0, 0 /*mutablestruct list termination header*/
      };
  bytes MS_xcdr_v1_normal_reordered {
      64, 1, 0, 12 /*mutablestruct.d.mheader*/,
      0, 0, 0, 0 /*padding bytes (4)*/,
      64, 132, 114, 145, 104, 114, 176, 33 /*mutablestruct.d*/,
      127, 1, 0, 8 /*mutablestruct.str.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 3, 0, 0, 0, 11 /*mutablestruct.str.mheader (extended)*/,
      0, 0, 0, 7 /*mutablestruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*mutablestruct.str.c_str*/,
      0 /*padding bytes (1)*/,
      64, 5, 0, 1 /*mutablestruct.g.mheader*/,
      'g' /*mutablestruct.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 7, 0, 4 /*mutablestruct.l.mheader*/,
      0, 1, 226, 64 /*mutablestruct.l*/,
      127, 2, 0, 0 /*mutablestruct list termination header*/
      };
  bytes MS_xcdr_v1_key{
      64, 5, 0, 1 /*mutablestruct.g.mheader*/,
      'g' /*mutablestruct.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 2, 0, 0 /*mutablestruct list termination header*/
      };
  bytes MS_xcdr_v2_normal {
      0, 0, 0, 48 /*dheader*/,
      160, 0, 0, 7 /*mutablestruct.l.emheader*/,
      0, 1, 226, 64 /*mutablestruct.l*/,
      128, 0, 0, 5 /*mutablestruct.g.emheader*/,
      'g' /*mutablestruct.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      192, 0, 0, 3, 0, 0, 0, 11 /*mutablestruct.str.emheader*/,
      0, 0, 0, 7 /*mutablestruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*mutablestruct.str.c_str*/,
      0 /*padding bytes (1)*/,
      176, 0, 0, 1 /*mutablestruct.d.emheader*/,
      64, 132, 114, 145, 104, 114, 176, 33 /*mutablestruct.d*/};
  bytes MS_xcdr_v2_normal_reordered {
      0, 0, 0, 48 /*dheader*/,
      176, 0, 0, 1 /*mutablestruct.d.emheader*/,
      64, 132, 114, 145, 104, 114, 176, 33 /*mutablestruct.d*/,
      192, 0, 0, 3, 0, 0, 0, 11 /*mutablestruct.str.emheader*/,
      0, 0, 0, 7 /*mutablestruct.str.length*/, 'a', 'b', 'c', 'd', 'e', 'f', '\0' /*mutablestruct.str.c_str*/,
      0 /*padding bytes (1)*/,
      128, 0, 0, 5 /*mutablestruct.g.emheader*/,
      'g' /*mutablestruct.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      160, 0, 0, 7 /*mutablestruct.l.emheader*/,
      0, 1, 226, 64 /*mutablestruct.l*/};
  bytes MS_xcdr_v2_key {
      0, 0, 0, 5 /*dheader*/,
      128, 0, 0, 5 /*mutablestruct.g.emheader*/,
      'g' /*mutablestruct.c*/};

  stream_test(MS, BS_basic_normal, BS_basic_key, MS_xcdr_v1_normal, MS_xcdr_v1_key, MS_xcdr_v2_normal, MS_xcdr_v2_key)
  VerifyRead(MS_xcdr_v1_normal_reordered, MS, xcdr_v1, false);
  VerifyRead(MS_xcdr_v2_normal_reordered, MS, xcdr_v2, false);
}

/*verifying reads/writes of a nested struct*/

TEST_F(CDRStreamer, cdr_nested)
{
  outer NS(inner('a',123), inner('b', 456), inner('c', 789));

  bytes NS_basic_normal {
      'a' /*outer.a.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 123 /*outer.a.l_inner*/,
      'b' /*outer.b.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 1, 200 /*outer.b.l_inner*/,
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 3, 21 /*outer.c.l_inner*/};
  bytes NS_basic_key {
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 3, 21 /*outer.c.l_inner*/};
  bytes NS_xcdr_v1_normal {
      127, 1, 0, 8 /*outer.a.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 0, 0, 0, 0, 20 /*outer.a.mheader (extended)*/,
      64, 0, 0, 1 /*outer.a.c_inner.mheader*/,
      'a' /*outer.a.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 1, 0, 4 /*outer.a.l_inner.mheader*/,
      0, 0, 0, 123 /*outer.a.l_inner*/,
      127, 2, 0, 0 /*inner list termination header*/,
      127, 1, 0, 8 /*outer.b.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 1, 0, 0, 0, 20 /*outer.b.mheader (extended)*/,
      64, 0, 0, 1 /*outer.b.c_inner.mheader*/,
      'b' /*outer.b.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 1, 0, 4 /*outer.b.l_inner.mheader*/,
      0, 0, 1, 200 /*outer.b.l_inner*/,
      127, 2, 0, 0 /*inner list termination header*/,
      127, 1, 0, 8 /*outer.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 2, 0, 0, 0, 20 /*outer.c.mheader (extended)*/,
      64, 0, 0, 1 /*outer.c.c_inner.mheader*/,
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 1, 0, 4 /*outer.c.l_inner.mheader*/,
      0, 0, 3, 21 /*outer.c.l_inner*/,
      127, 2, 0, 0 /*inner list termination header*/,
      127, 2, 0, 0 /*outer list termination header*/};
  bytes NS_xcdr_v1_key {
      127, 1, 0, 8 /*outer.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 2, 0, 0, 0, 20 /*outer.c.mheader (extended)*/,
      64, 0, 0, 1 /*outer.c.c_inner.mheader*/,
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 1, 0, 4 /*outer.c.l_inner.mheader*/,
      0, 0, 3, 21 /*outer.c.l_inner*/,
      127, 2, 0, 0 /*inner list termination header*/,
      127, 2, 0, 0 /*outer list termination header*/};
  bytes NS_xcdr_v2_normal {
      0, 0, 0, 84 /*outer.dheader*/,
      192, 0, 0, 0 /*outer.a.emheader*/,
      0, 0, 0, 20 /*outer.a.emheader.nextint*/,
      0, 0, 0, 16 /*outer.a.dheader*/,
      128, 0, 0, 0 /*outer.a.c_inner.emheader*/,
      'a' /*outer.a.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      160, 0, 0, 1 /*outer.a.l_inner.emheader*/,
      0, 0, 0, 123 /*outer.a.l_inner*/,
      192, 0, 0, 1 /*outer.b.emheader*/,
      0, 0, 0, 20 /*outer.b.emheader.nextint*/,
      0, 0, 0, 16 /*outer.b.dheader*/,
      128, 0, 0, 0 /*outer.b.c_inner.emheader*/,
      'b' /*outer.b.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      160, 0, 0, 1 /*outer.b.l_inner.emheader*/,
      0, 0, 1, 200 /*outer.b.l_inner*/,
      192, 0, 0, 2 /*outer.c.emheader*/,
      0, 0, 0, 20 /*outer.c.emheader.nextint*/,
      0, 0, 0, 16 /*outer.c.dheader*/,
      128, 0, 0, 0 /*outer.c.c_inner.emheader*/,
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      160, 0, 0, 1 /*outer.c.l_inner.emheader*/,
      0, 0, 3, 21 /*outer.c.l_inner*/};
  bytes NS_xcdr_v2_key {
      0, 0, 0, 28 /*outer.dheader*/,
      192, 0, 0, 2 /*outer.c.emheader*/,
      0, 0, 0, 20 /*outer.c.emheader.nextint*/,
      0, 0, 0, 16 /*outer.c.dheader*/,
      128, 0, 0, 0 /*outer.c.c_inner.emheader*/,
      'c' /*outer.c.c_inner*/,
      0, 0, 0 /*padding bytes (3)*/,
      160, 0, 0, 1 /*outer.c.l_inner.emheader*/,
      0, 0, 3, 21 /*outer.c.l_inner*/};

  stream_test(NS, NS_basic_normal, NS_basic_key, NS_xcdr_v1_normal, NS_xcdr_v1_key, NS_xcdr_v2_normal, NS_xcdr_v2_key)
}

/*verifying reads/writes of a struct containing inheritance*/

TEST_F(CDRStreamer, cdr_inherited)
{
  derived DS("gfedcb", 'a');
  DS.base_str("hjklmn");
  DS.base_c('o');

  bytes DS_basic_normal {
      0, 0, 0, 7 /*derived::base.base_str.length*/, 'h', 'j', 'k', 'l', 'm', 'n', '\0' /*derived::base.base_str.c_str*/,
      'o'/*derived::base.base_c*/,
      0, 0, 0, 7 /*derived.str.length*/, 'g', 'f', 'e', 'd', 'c', 'b', '\0'/*derived.str.c_str*/,
      'a'/*derived.c*/
      };
  bytes DS_basic_key {
      'o'/*derived::base.base_c*/,
      'a'/*derived.c*/
      };
  bytes DS_xcdr_v1_normal {
      0, 0, 0, 7 /*derived::base.base_str.length*/, 'h', 'j', 'k', 'l', 'm', 'n', '\0' /*derived::base.base_str.c_str*/,
      'o'/*derived::base.base_c*/,
      127, 1, 0, 8 /*derived.str.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 123, 0, 0, 0, 11 /*derived.str.mheader (extended)*/,
      0, 0, 0, 7 /*derived.str.length*/, 'g', 'f', 'e', 'd', 'c', 'b', '\0'/*derived.str.c_str*/,
      0 /*padding bytes (1)*/,
      64, 234, 0, 1 /*derived.c.mheader*/,
      'a'/*derived.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 2, 0, 0 /*inner list termination header*/
      };
  bytes DS_xcdr_v1_key {
      'o'/*derived::base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      64, 234, 0, 1 /*derived.c.mheader*/,
      'a'/*derived.c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 2, 0, 0 /*inner list termination header*/
      };
  bytes DS_xcdr_v2_normal {
      0, 0, 0, 37/*derived.dheader*/,
      0, 0, 0, 7 /*derived::base.base_str.length*/, 'h', 'j', 'k', 'l', 'm', 'n', '\0' /*derived::base.base_str.c_str*/,
      'o'/*derived::base.base_c*/,
      192, 0, 0, 123 /*derived.str.emheader*/,
      0, 0, 0, 11 /*derived.str.emheader.nextint*/,
      0, 0, 0, 7 /*derived.str.length*/, 'g', 'f', 'e', 'd', 'c', 'b', '\0'/*derived.str.c_str*/,
      0 /*padding bytes (1)*/,
      128, 0, 0, 234 /*derived.c.emheader*/,
      'a' /*derived.c*/
      };
  bytes DS_xcdr_v2_key {
      0, 0, 0, 9/*derived.dheader*/,
      'o'/*derived::base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      128, 0, 0, 234 /*derived.c.emheader*/,
      'a' /*derived.c*/
      };

  stream_test(DS, DS_basic_normal, DS_basic_key, DS_xcdr_v1_normal, DS_xcdr_v1_key, DS_xcdr_v2_normal, DS_xcdr_v2_key)
}

/*verifying reads/writes of a struct containing sequences*/

TEST_F(CDRStreamer, cdr_sequence)
{
  sequence_struct SS({'z','y','x'}, {4,3,2,1});

  bytes SS_basic_normal {
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/,
      0 /*padding bytes (1)*/,
      0, 0, 0, 4/*sequence_struct.l.length*/, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1/*sequence_struct.l.data*/
      };
  bytes SS_basic_key {
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/
      };
  bytes SS_xcdr_v1_normal {
      127, 1, 0, 8 /*sequence_struct.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 0, 0, 0, 0, 7 /*sequence_struct.c.mheader (extended)*/,
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/,
      0 /*padding bytes (1)*/,
      127, 1, 0, 8 /*sequence_struct.l.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 1, 0, 0, 0, 20 /*sequence_struct.l.mheader (extended)*/,
      0, 0, 0, 4/*sequence_struct.l.length*/, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1/*sequence_struct.l.data*/,
      127, 2, 0, 0 /*inner list termination header*/
      };
  bytes SS_xcdr_v1_key {
      127, 1, 0, 8 /*sequence_struct.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 0, 0, 0, 0, 7 /*sequence_struct.c.mheader (extended)*/,
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/,
      0 /*padding bytes (1)*/,
      127, 2, 0, 0 /*inner list termination header*/
      };
  bytes SS_xcdr_v2_normal {
      0, 0, 0, 44/*sequence_struct.dheader*/,
      192, 0, 0, 0 /*derived.c.emheader*/,
      0, 0, 0, 7 /*derived.c.emheader.nextint*/,
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/,
      0 /*padding bytes (1)*/,
      192, 0, 0, 1 /*derived.l.emheader*/,
      0, 0, 0, 20 /*derived.c.emheader.nextint*/,
      0, 0, 0, 4/*sequence_struct.l.length*/, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1/*sequence_struct.l.data*/
      };
  bytes SS_xcdr_v2_key {
      0, 0, 0, 15/*sequence_struct.dheader*/,
      192, 0, 0, 0 /*derived.c.emheader*/,
      0, 0, 0, 7 /*derived.c.emheader.nextint*/,
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/
      };
  /*different length code, overlapping nextint with the length of the sequence
    our streamer implementation does not write this way, but it must be able to
    read it*/
  bytes SS_xcdr_v2_normal_lc_not_4 {
      0, 0, 0, 36/*sequence_struct.dheader*/,
      208, 0, 0, 0 /*derived.c.emheader*/, /*lc = 5: length = sequence_struct.c.length*1*/
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/,
      0 /*padding bytes (1)*/,
      224, 0, 0, 1 /*derived.l.emheader*/, /*lc = 6: length = sequence_struct.c.length*4*/
      0, 0, 0, 4/*sequence_struct.l.length*/, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1/*sequence_struct.l.data*/
      };
  bytes SS_xcdr_v2_key_lc_not_4 {
      0, 0, 0, 11/*sequence_struct.dheader*/,
      208, 0, 0, 0 /*derived.c.emheader*/, /*lc = 5: length = sequence_struct.c.length*1*/
      0, 0, 0, 3/*sequence_struct.c.length*/, 'z', 'y', 'x'/*sequence_struct.c.data*/
      };

  stream_test(SS, SS_basic_normal, SS_basic_key, SS_xcdr_v1_normal, SS_xcdr_v1_key, SS_xcdr_v2_normal, SS_xcdr_v2_key)

  read_test(SS, SS_xcdr_v2_normal_lc_not_4, SS_xcdr_v2_key_lc_not_4, xcdr_v2)
}

/*verifying reads/writes of a struct containing arrays*/

TEST_F(CDRStreamer, cdr_array)
{
  array_struct ARS({'e','d','c','b','a'},{123,234,345,456,567});

  bytes ARS_normal {
      'e', 'd', 'c', 'b', 'a'/*array_struct.c*/,
      0, 0, 0 /*padding bytes*/,
      0, 0, 0, 123,
      0, 0, 0, 234,
      0, 0, 1, 89,
      0, 0, 1, 200,
      0, 0, 2, 55 /*array_struct.l*/,
      };
  bytes ARS_key {
      'e', 'd', 'c', 'b', 'a'/*array_struct.c*/
      };

  stream_test(ARS, ARS_normal, ARS_key, ARS_normal, ARS_key, ARS_normal, ARS_key)
}

/*verifying reads/writes of a struct containing typedefs*/

TEST_F(CDRStreamer, cdr_typedef)
{
  typedef_struct TDS({base("qwe",'a'),base("wer",'b'),base("ert",'c'),base("rty",'d')},{base("tyu",'e'),base("yui",'f'),base("uio",'g')});

  bytes TDS_basic_normal {
      0, 0, 0, 4/*typedef_struct.c.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 'q', 'w', 'e', '\0' /*base.base_str.c_str*/,
      'a'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'w', 'e', 'r', '\0' /*base.base_str.c_str*/,
      'b'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'e', 'r', 't', '\0' /*base.base_str.c_str*/,
      'c'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'r', 't', 'y', '\0' /*base.base_str.c_str*/,
      'd'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 3/*typedef_struct.l.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 't', 'y', 'u', '\0' /*base.base_str.c_str*/,
      'e'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'y', 'u', 'i', '\0' /*base.base_str.c_str*/,
      'f'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'u', 'i', 'o', '\0' /*base.base_str.c_str*/,
      'g'/*base.base_c*/
      };
  bytes TDS_basic_key {
      0, 0, 0, 4/*typedef_struct.c.length*/,
      'a'/*base.base_c*/,
      'b'/*base.base_c*/,
      'c'/*base.base_c*/,
      'd'/*base.base_c*/
      };
  bytes TDS_xcdr_v1_normal {
      127, 1, 0, 8 /*typedef_struct.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 0, 0, 0, 0, 49 /*typedef_struct.c.mheader (extended)*/,
      0, 0, 0, 4/*typedef_struct.c.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 'q', 'w', 'e', '\0' /*base.base_str.c_str*/,
      'a'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'w', 'e', 'r', '\0' /*base.base_str.c_str*/,
      'b'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'e', 'r', 't', '\0' /*base.base_str.c_str*/,
      'c'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'r', 't', 'y', '\0' /*base.base_str.c_str*/,
      'd'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 1, 0, 8 /*typedef_struct.l.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 1, 0, 0, 0, 37 /*typedef_struct.l.mheader (extended)*/,
      0, 0, 0, 3/*typedef_struct.l.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 't', 'y', 'u', '\0' /*base.base_str.c_str*/,
      'e'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'y', 'u', 'i', '\0' /*base.base_str.c_str*/,
      'f'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'u', 'i', 'o', '\0' /*base.base_str.c_str*/,
      'g'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      127, 2, 0, 0 /*list termination header*/
      };
  bytes TDS_xcdr_v1_key {
      127, 1, 0, 8 /*typedef_struct.c.mheader (pid_list_extended + must_understand + length = 8)*/,
      64, 0, 0, 0, 0, 0, 0, 8 /*typedef_struct.c.mheader (extended)*/,
      0, 0, 0, 4/*typedef_struct.c.length*/,
      'a'/*base.base_c*/,
      'b'/*base.base_c*/,
      'c'/*base.base_c*/,
      'd'/*base.base_c*/,
      127, 2, 0, 0 /*list termination header*/
      };
  bytes TDS_xcdr_v2_normal {
      0, 0, 0, 105/*typedef_struct.dheader*/,
      192, 0, 0, 0 /*typedef_struct.c.emheader*/,
      0, 0, 0, 49 /*typedef_struct.c.emheader.nextint*/,
      0, 0, 0, 4/*typedef_struct.c.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 'q', 'w', 'e', '\0' /*base.base_str.c_str*/,
      'a'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'w', 'e', 'r', '\0' /*base.base_str.c_str*/,
      'b'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'e', 'r', 't', '\0' /*base.base_str.c_str*/,
      'c'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'r', 't', 'y', '\0' /*base.base_str.c_str*/,
      'd'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      192, 0, 0, 1 /*typedef_struct.l.emheader*/,
      0, 0, 0, 37 /*typedef_struct.l.emheader.nextint*/,
      0, 0, 0, 3/*typedef_struct.l.length*/,
      0, 0, 0, 4/*base.base_str.length*/, 't', 'y', 'u', '\0' /*base.base_str.c_str*/,
      'e'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'y', 'u', 'i', '\0' /*base.base_str.c_str*/,
      'f'/*base.base_c*/,
      0, 0, 0 /*padding bytes (3)*/,
      0, 0, 0, 4/*base.base_str.length*/, 'u', 'i', 'o', '\0' /*base.base_str.c_str*/,
      'g'/*base.base_c*/
      };
  bytes TDS_xcdr_v2_key {
      0, 0, 0, 16/*typedef_struct.dheader*/,
      192, 0, 0, 0 /*typedef_struct.c.emheader*/,
      0, 0, 0, 8 /*typedef_struct.c.emheader.nextint*/,
      0, 0, 0, 4/*typedef_struct.c.length*/,
      'a'/*base.base_c*/,
      'b'/*base.base_c*/,
      'c'/*base.base_c*/,
      'd'/*base.base_c*/
      };

  stream_deeper_test(TDS, TDS_basic_normal, TDS_basic_key, TDS_xcdr_v1_normal, TDS_xcdr_v1_key, TDS_xcdr_v2_normal, TDS_xcdr_v2_key)
}

/*verifying reads/writes of a struct containing unions*/

TEST_F(CDRStreamer, cdr_union)
{
  un _a, _b, _c;
  _a.c('b','a');
  _b.s(234,'c');
  _c.l(123,'e');

  union_struct US(_a, _b, _c);

  bytes US_basic_normal {
      'a'/*union_struct.a.switch*/,
      'b'/*union_struct.a.c*/,
      'c'/*union_struct.b.switch*/,
      0/*padding bytes (1)*/,
      0, 234/*union_struct.b.s*/,
      'e'/*union_struct.c.switch*/,
      0/*padding bytes (1)*/,
      0, 0, 0, 123/*union_struct.c.l*/
      };
  bytes US_basic_key {
      'e'/*union_struct.c.switch*/,
      0, 0, 0/*padding bytes (3)*/,
      0, 0, 0, 123/*union_struct.c.l*/
      };

  stream_test(US, US_basic_normal, US_basic_key, US_basic_normal, US_basic_key, US_basic_normal, US_basic_key)
}

/*verifying reads/writes of structs using pragma keylist*/

TEST_F(CDRStreamer, cdr_pragma)
{
  pragma_keys PS(sub_2(sub_1(123,234),sub_1(345,456)),sub_2(sub_1(567,678),sub_1(789,890)));
  pragma_keys PS_key_test(sub_2(sub_1(0,0),sub_1(345,456)),sub_2(sub_1(0,678),sub_1(0,0)));

  bytes PS_basic_normal {
      0, 0, 0, 123/*pragma_keys.c.s_1.l_1*/,
      0, 0, 0, 234/*pragma_keys.c.s_1.l_2*/,
      0, 0, 1,  89/*pragma_keys.c.s_2.l_1*/,
      0, 0, 1, 200/*pragma_keys.c.s_2.l_2*/,
      0, 0, 2,  55/*pragma_keys.d.s_1.l_1*/,
      0, 0, 2, 166/*pragma_keys.d.s_1.l_2*/,
      0, 0, 3,  21/*pragma_keys.d.s_2.l_1*/,
      0, 0, 3, 122/*pragma_keys.d.s_2.l_2*/
      };
  bytes PS_basic_key {
      0, 0, 1, 200/*pragma_keys.c.s_2.l_2*/,
      0, 0, 2, 166/*pragma_keys.d.s_1.l_2*/,
      0, 0, 1,  89/*pragma_keys.c.s_2.l_1*/,
      };

  VerifyRead(PS_basic_normal, PS, basic, false);
  VerifyRead(PS_basic_normal, PS, xcdr_v1, false);
  VerifyRead(PS_basic_normal, PS, xcdr_v2, false);

  VerifyRead(PS_basic_key, PS_key_test, basic, true);
  VerifyRead(PS_basic_key, PS_key_test, xcdr_v1, true);
  VerifyRead(PS_basic_key, PS_key_test, xcdr_v2, true);

  write_test(PS, PS_basic_normal, PS_basic_key, basic)
  write_test(PS, PS_basic_normal, PS_basic_key, xcdr_v1)
  write_test(PS, PS_basic_normal, PS_basic_key, xcdr_v2)
}

/*verifying reads/writes of a struct containing enums*/

TEST_F(CDRStreamer, cdr_enum)
{
  enum_struct ES(enum_8::second_8, enum_16::third_16, enum_32::fourth_32);

  /*basic cdr treats all enums as 32 bit integers*/
  bytes ES_basic_normal {
      0, 0, 0 ,1 /*enum_struct.c*/,
      0, 0, 0, 2 /*enum_struct.b*/,
      0, 0, 0, 3 /*enum_struct.a*/
      };
  bytes ES_basic_key {
      0, 0, 0, 1 /*enum_struct.c*/
      };
  /*xcdr_v1 and xcdr_v2 treat bitbounded enums in the same manner*/
  bytes ES_xcdr_v1_normal {
      1 /*enum_struct.c*/,
      0 /*padding bytes (1)*/,
      0, 2 /*enum_struct.b*/,
      0, 0, 0, 3 /*enum_struct.a*/
      };
  bytes ES_xcdr_v1_key {
      1 /*enum_struct.c*/
      };

  //currently the bit_bound field is not being parsed, so enums are written as 32-bit integers
  (void) ES_xcdr_v1_normal;
  (void) ES_xcdr_v1_key;

  stream_test(ES, ES_basic_normal, ES_basic_key, ES_basic_normal, ES_basic_key, ES_basic_normal, ES_basic_key)
}

/*verifying reads/writes of structs containing optional fields*/

TEST_F(CDRStreamer, cdr_optional)
{
  optional_final_struct OFS('a', 'b', std::nullopt);
  optional_appendable_struct OAS('a', 'b', std::nullopt);
  optional_mutable_struct OMS('a', 'b', std::nullopt);

  /*no basic cdr, since it does not support optional fields*/
  bytes OFS_xcdr_v1_normal {
    64, 0, 0, 1 /*optional_final_struct.a.mheader*/,
    'a'/*optional_final_struct.a*/,
    'b'/*optional_final_struct.b*/,
    0, 0/*padding bytes (2)*/,
    64, 2, 0, 0 /*optional_final_struct.c.mheader*/
    };
  bytes OFS_xcdr_v1_key {
    64, 2, 0, 0 /*optional_final_struct.c.mheader*/
    };
  bytes OMS_xcdr_v1_normal {
    64, 0, 0, 1 /*optional_mutable_struct.a.mheader*/,
    'a'/*optional_mutable_struct.a*/,
    0, 0, 0/*padding bytes (3)*/,
    64, 1, 0, 1 /*optional_mutable_struct.b.mheader*/,
    'b'/*optional_mutable_struct.b*/,
    0, 0, 0/*padding bytes (3)*/,
    127, 2, 0, 0 /*optional_mutable_struct list termination header*/
    };
  bytes OMS_xcdr_v1_key {
    127, 2, 0, 0 /*optional_mutable_struct list termination header*/
    };
  bytes OFS_xcdr_v2_normal {
    1/*optional_final_struct.a.is_present*/,
    'a'/*optional_final_struct.a*/,
    'b'/*optional_final_struct.b*/,
    0/*optional_final_struct.c.is_present*/
    };
  bytes OFS_xcdr_v2_key {
    0/*optional_final_struct.c.is_present*/
    };
  bytes OAS_xcdr_v2_normal {
    0, 0, 0, 4/*dheader*/,
    1/*optional_final_struct.a.is_present*/,
    'a'/*optional_final_struct.a*/,
    'b'/*optional_final_struct.b*/,
    0/*optional_final_struct.c.is_present*/
    };
  bytes OAS_xcdr_v2_key {
    0, 0, 0, 1/*dheader*/,
    0/*optional_final_struct.c.is_present*/
    };
  bytes OMS_xcdr_v2_normal {
    0, 0, 0, 13/*dheader*/,
    128, 0, 0, 0 /*derived.c.emheader*/,
    'a'/*optional_final_struct.a*/,
    0, 0, 0/*padding bytes (3)*/,
    128, 0, 0, 1 /*derived.c.emheader*/,
    'b'/*optional_final_struct.b*/
    };
  bytes OMS_xcdr_v2_key {
    0, 0, 0, 0/*dheader*/
    };

  /* basic cdr does not support optional fields,
     therefore the streamer should enter error status
     when the streamer is asked to write them */
  bytes in_bytes {'a', 'b', 'c'};
  optional_final_struct out_struct;
  basic_cdr_stream b(endianness::big_endian);
  b.set_buffer(in_bytes.data());
  read(b, out_struct, false);

  ASSERT_EQ(b.status(), uint64_t(serialization_status::unsupported_property));

  bytes out_bytes(3, 0);
  b.set_buffer(out_bytes.data());
  write(b, OFS, false);

  ASSERT_EQ(b.status(), uint64_t(serialization_status::unsupported_property));

  readwrite_test(OFS, OFS_xcdr_v1_normal, OFS_xcdr_v1_key, xcdr_v1)
  readwrite_test(OAS, OFS_xcdr_v1_normal, OFS_xcdr_v1_key, xcdr_v1)
  readwrite_test(OMS, OMS_xcdr_v1_normal, OMS_xcdr_v1_key, xcdr_v1)

  readwrite_test(OFS, OFS_xcdr_v2_normal, OFS_xcdr_v2_key, xcdr_v2)
  readwrite_test(OAS, OAS_xcdr_v2_normal, OAS_xcdr_v2_key, xcdr_v2)
  readwrite_test(OMS, OMS_xcdr_v2_normal, OMS_xcdr_v2_key, xcdr_v2)
}
