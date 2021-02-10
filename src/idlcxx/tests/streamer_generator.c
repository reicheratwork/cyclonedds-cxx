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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern const unsigned char base_head_cpp_in[];
extern const unsigned char instance_head_cpp_in[];
extern const unsigned char string_head_cpp_in[];
extern const unsigned char sequence_head_cpp_in[];
extern const unsigned char union_head_cpp_in[];
extern const unsigned char enum_head_cpp_in[];
extern const unsigned char array_head_cpp_in[];
extern const unsigned char keys_base_head_cpp_in[];
extern const unsigned char keys_struct_implicit_head_cpp_in[];
extern const unsigned char keys_struct_explicit_head_cpp_in[];
extern const unsigned char keys_typedef_header_cpp_in[];
extern const unsigned char keys_union_implicit_head_cpp_in[];
extern const unsigned char struct_inheritance_head_cpp_in[];
extern const unsigned char typedef_resolution_header_cpp_in[];

#include "idlcxx/streamer_generator.h"
#include "idl/processor.h"

#include "CUnit/Theory.h"

void test_base_member()
{
  const char *buffer =
    "module N {\n"\
    "struct s {\n"\
    "double mem;\n"\
    "};\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(base_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_instance_member()
{
  const char *buffer =
    "module N {\n"\
    "struct I {\n"\
    "long l;\n"
    "};\n"
    "struct s {\n"\
    "I mem;\n"\
    "};\n"\
    "};\n";
  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(instance_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_string_member()
{
  const char *buffer =
    "module N {\n"\
    "struct s {\n"\
    "string str;\n"\
    "};\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(string_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_sequence_member()
{
  const char *buffer =
    "module N {\n"\
    "struct s {\n"\
    "sequence<long long> mem;\n"\
    "};\n"\
    "};\n";
  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(sequence_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_union()
{
  const char *buffer =
    "module N {\n"\
    "union s switch (long) {\n"\
    "case -12345:\n"\
    "case 0:\n"\
    "case 1: octet o;\n"\
    "case 2:\n"\
    "case 3: long l;\n"\
    "case 4:\n"\
    "case 5: string str;\n"\
    "default: float f;\n"\
    "};\n"\
    "};\n";
  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(union_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_enum()
{
  const char *buffer =
    "module N {\n"\
    "enum E {e_0, e_1, e_2, e_3};\n"\
    "struct s {\n"\
    "E mem;\n"\
    "};"\
    "};\n";
  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(enum_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_array()
{
  const char *buffer =
    "module N {\n"\
    "struct s {\n"\
    "float mem[3][2], mem2;\n"\
    "};\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, buffer);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(array_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_inheritance()
{
  const char* str =
    "module N {\n"\
    "struct I {\n"\
    "  long inherited_member; \n"\
    "};\n"\
    "struct s : I {\n"\
    "  long new_member; \n"\
    "};\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, IDL_FLAG_EXTENDED_DATA_TYPES, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(struct_inheritance_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_typedef_resolution()
{
  const char* str =
    "module M {\n"\
    "typedef long td_0;\n"\
    "typedef td_0 td_1;\n"\
    "typedef td_1 td_2;\n"\
    "typedef td_2 td_3;\n"\
    "typedef td_3 td_4;\n"\
    "typedef td_4 td_5;\n"\
    "typedef sequence<td_5> td_6;\n"\
    "};\n"\
    "module N {\n"\
    "struct s {\n"\
    "M::td_5 mem_simple;\n"\
    "sequence<M::td_6> mem;\n"\
    "};\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(typedef_resolution_header_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_keys_base()
{
  const char* str =
    "struct s {\n"\
    "octet _o;\n"\
    "@key long _l;\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, 0u, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(keys_base_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_keys_union_implicit()
{
  const char* str =
  "union u switch (long) {\n"\
    "case -12345:\n"\
    "case 0:\n"\
    "case 1: octet o;\n"\
    "case 2:\n"\
    "case 3: long l;\n"\
    "case 4:\n"\
    "case 5: string str;\n"\
    "default: float f;\n"\
    "};\n"\
    "struct ss {\n"\
    "octet _o;\n"\
    "@key u _u;\n"\
    "@key long _l;\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, IDL_FLAG_ANNOTATIONS, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(keys_union_implicit_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_keys_struct_explicit()
{
  const char* str =
    "struct s {\n"\
    "octet _o;\n"\
    "@key long _l;\n"\
    "};\n"\
    "struct ss {\n"\
    "octet _o;\n"\
    "@key s _s_;\n"\
    "@key long _l;\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, IDL_FLAG_ANNOTATIONS, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(keys_struct_explicit_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_keys_struct_implicit()
{
  const char* str =
    "struct s {\n"\
    "octet _o;\n"\
    "long _l;\n"\
    "};\n"\
    "struct ss {\n"\
    "octet _o;\n"\
    "@key s _s_;\n"\
    "@key long _l;\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, IDL_FLAG_ANNOTATIONS, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(keys_struct_implicit_head_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));

  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

void test_keys_typedef()
{
  const char* str =
    "typedef sequence<long> td_1;\n"\
    "struct s {\n"\
    "octet _o;\n"\
    "@key td_1 _t;\n"\
    "};\n";

  idl_pstate_t* tree = NULL;
  idl_builtin_annotation_t annots = { NULL,NULL,NULL };
  idl_create_pstate(IDL4, IDL_FLAG_ANNOTATIONS, &annots, &tree);

  idl_parse_string(tree, str);

  idl_streamer_output_t* generated = create_idl_streamer_output();
  idl_streamers_generate(tree, generated);

  CU_ASSERT_STRING_EQUAL("", get_ostream_buffer(get_idl_streamer_impl_buf(generated)));
  CU_ASSERT_STRING_EQUAL(keys_typedef_header_cpp_in, get_ostream_buffer(get_idl_streamer_head_buf(generated)));
  
  destruct_idl_streamer_output(generated);
  idl_delete_pstate(tree);
}

CU_Test(streamer_generator, base_types)
{
  test_base_member();
}

CU_Test(streamer_generator, instance_member)
{
  test_instance_member();
}

CU_Test(streamer_generator, string_member)
{
  test_string_member();
}

CU_Test(streamer_generator, sequence_member)
{
  test_sequence_member();
}

CU_Test(streamer_generator, union_case_long)
{
  test_union();
}

CU_Test(streamer_generator, enumerator)
{
  test_enum();
}

CU_Test(streamer_generator, array_)
{
  test_array();
}

CU_Test(streamer_generator, inheritance)
{
  test_inheritance();
}

CU_Test(streamer_generator, typedef_resolution)
{
  test_typedef_resolution();
}

CU_Test(streamer_generator, key_base)
{
  test_keys_base();
}

CU_Test(streamer_generator, key_struct_explicit)
{
  test_keys_struct_explicit();
}

CU_Test(streamer_generator, key_struct_implicit)
{
  test_keys_struct_implicit();
}

CU_Test(streamer_generator, key_union_implicit)
{
  test_keys_union_implicit();
}

CU_Test(streamer_generator, key_typedef)
{
  test_keys_typedef();
}
