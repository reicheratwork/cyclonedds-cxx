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
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include "idlcxx/streamer_generator.h"
#include "idlcxx/backendCpp11Utils.h"
#include "idlcxx/token_replace.h"
#include "idl/tree.h"
#include "idl/scope.h"
#include "idl/string.h"
#include "idl/processor.h"

#define BASE_TYPE_MASK ((IDL_BASE_TYPE << 1) -1)

static void format_ostream_indented(size_t depth, idl_ostream_t* ostr, const char *fmt, ...)
{
  if (depth > 0) format_ostream(ostr, "%*c", depth, ' ');
  va_list args;
  va_start(args, fmt);
  format_ostream_va_args(ostr, fmt, args);
  va_end(args);
}

#define format_header_stream(indent,ctx,...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->str->head_stream, __VA_ARGS__);

#define format_impl_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->str->impl_stream, __VA_ARGS__);

#define format_key_size_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_size_stream, __VA_ARGS__);

#define format_key_max_size_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_max_size_stream, __VA_ARGS__);

#define format_key_max_size_intermediate_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_max_size_intermediate_stream, __VA_ARGS__);

#define format_key_write_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_write_stream, __VA_ARGS__);

#define format_key_read_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_read_stream, __VA_ARGS__);

#define format_max_size_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->max_size_stream, __VA_ARGS__); \
if (key) { format_key_max_size_stream(indent, ctx, __VA_ARGS__); }

#define format_max_size_intermediate_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->max_size_intermediate_stream, __VA_ARGS__); \
if (key) { format_key_max_size_intermediate_stream(indent, ctx, __VA_ARGS__); }

#define format_write_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->write_stream, __VA_ARGS__); \
if (key) {format_key_write_stream(indent,ctx, __VA_ARGS__);}

#define format_write_size_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->write_size_stream, __VA_ARGS__); \
if (key) {format_key_size_stream(indent,ctx, __VA_ARGS__);}

#define format_read_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->read_stream, __VA_ARGS__); \
if (key) {format_key_read_stream(indent,ctx, __VA_ARGS__);}

#define namespace_declaration "namespace %s\n"
#define namespace_closure "} //end " namespace_declaration "\n"
#define incr_comment "  //moving position indicator\n"
#define align_comment "  //alignment\n"
#define padding_comment "  //padding bytes\n"
#define bytes_for_member_comment "  //bytes for member: %s\n"
#define bytes_for_seq_entries_comment "  //bytes for sequence entries\n"
#define entries_of_sequence_comment "  //entries of sequence\n"
#define union_switch "  switch (_disc_temp)\n"
#define union_case "  case %s:\n"
#define default_case "  default:\n"
#define union_case_ending "  break;\n"
#define union_clear_func "  clear();\n"
#define open_block "{\n"
#define close_block "}\n"
#define close_function close_block "\n"
#define primitive_calc_alignment_modulo "(%d - (position%%%d))%%%d;"
#define primitive_calc_alignment_shift "(%d - (position&%#x))&%#x;"
#define seqentries "_se%d"
#define alignmentbytes "_al%d"
#define position_incr "  position += "
#define position_set "  position = "
#define position_return "  return position;\n"
#define position_incr_alignment position_incr alignmentbytes ";"
#define primitive_incr_pos position_incr "%d;"
#define max_size_check "  if (position != UINT_MAX) "
#define max_size_incr_checked max_size_check primitive_incr_pos
#define primitive_write_func_padding "  memset(static_cast<char*>(data)+position,0x0,%d);  //setting padding bytes to 0x0\n"
#define primitive_write_func_alignment "  memset(static_cast<char*>(data)+position,0x0,"alignmentbytes");  //setting alignment bytes to 0x0\n"
#define primitive_write_func_write "  *reinterpret_cast<%s*>(static_cast<char*>(data)+position) = %s;  //writing bytes for member: %s\n"
#define primitive_write_func_array "  memcpy(static_cast<char*>(data)+position,%s.data(),%d);  //writing bytes for member: %s\n"
#define primitive_write_func_seq seqentries" = static_cast<uint32_t>(%s.size()%s);  //number of entries in the sequence\n"
#define primitive_write_func_seq2 "  *reinterpret_cast<uint32_t*>(static_cast<char*>(data) + position) = "seqentries";  //writing entries for member: %s\n"
#define primitive_read_func_read "  %s = *reinterpret_cast<const %s*>(static_cast<const char*>(data)+position);  //reading bytes for member: %s\n"
#define primitive_read_func_array "  memcpy(%s.data(),static_cast<const char*>(data)+position,%d);  //reading bytes for member: %s\n"
#define primitive_read_func_seq seqentries" = *reinterpret_cast<const uint32_t*>(static_cast<const char*>(data)+position);  //number of entries in the sequence\n"
#define primitive_write_seq "memcpy(static_cast<char*>(data)+position,%s.data(),"seqentries"*%d); //writing bytes for member: %s\n"
#define primitive_write_seq_checked "  if (0 < %s.size()) " primitive_write_seq
#define primitive_read_seq "  %s.assign(reinterpret_cast<const %s*>(static_cast<const char*>(data)+position),reinterpret_cast<const %s*>(static_cast<const char*>(data)+position)+"seqentries"); //reading bytes for member: %s\n"
#define read_str "  %s.assign(static_cast<const char*>(data)+position,static_cast<const char*>(data)+position+"seqentries"-1); //reading bytes for member: %s\n"
#define string_write "  " primitive_write_seq
#define bool_write_seq "  for (size_t _b = 0; _b < "seqentries"; _b++) *(static_cast<char*>(data)+position++) = (%s[_b] ? 0x1 : 0x0); //writing bytes for member: %s\n"
#define bool_read_seq "  for (size_t _b = 0; _b < "seqentries"; _b++) %s[_b] = (*(static_cast<const char*>(data)+position++) ? 0x1 : 0x0); //reading bytes for member: %s\n"
#define sequence_iterate "  for (size_t _i%d = 0; _i%d < "seqentries"; _i%d++) {\n"
#define bound_iterate "  for (size_t _i%d = 0; _i%d < %"PRIu64"; _i%d++) {\n"
#define seq_read_resize "  %s.resize("seqentries");\n"
#define seq_length_exception "  if ("seqentries" > %"PRIu64") throw dds::core::InvalidArgumentError(\"attempt to assign entries to bounded member %s in excess of maximum length %"PRIu64"\");\n"
#define seq_read_past_bound_resize "  if ("seqentries" > %"PRIu64") %s.resize(%"PRIu64");\n"
#define seq_incr position_incr seqentries"*%d;"
#define seq_inc_1 position_incr seqentries ";"
#define max_size_incr_checked_multiple max_size_check primitive_incr_pos entries_of_sequence_comment
#define array_iterate "for (size_t _i%d = 0; _i%d < %"PRIu64"; _i%d++) {\n"
#define array_accessor "%s[_i%d]"
#define instance_write_func position_set "%s.write_struct(data, position);\n"
#define instance_key_write_func position_set "%s.key_write(data, position);\n"
#define instance_key_read_func position_set "%s.key_read(data, position);\n"
#define instance_size_func_calc position_set "%s.write_size(position);\n"
#define instance_key_size_func_calc position_set "%s.key_size(position);\n"
#define instance_key_max_size_func_calc max_size_check position_set "%s.key_max_size(position);\n"
#define instance_max_size_func max_size_check position_set "%s.max_size(position);\n"
#define instance_key_max_size_union_func_calc "case_max = %s.key_max_size(case_max);\n"
#define instance_max_size_func_union "case_max = %s.max_size(case_max);\n"
#define instance_read_func position_set "%s.read_struct(data, position);\n"
#define const_ref_cast "dynamic_cast<const %s%s&>(*this)"
#define ref_cast "dynamic_cast<%s%s&>(*this)"
#define member_access "%s()"
#define write_func_define "size_t %s::write_struct(void *data, size_t position) const"
#define write_size_func_define "size_t %s::write_size(size_t position) const"
#define max_size_define "size_t %s::max_size(size_t position) const"
#define read_func_define "size_t %s::read_struct(const void *data, size_t position)"
#define key_size_define "size_t %s::key_size(size_t position) const"
#define key_max_size_define "size_t %s::key_max_size(size_t position) const"
#define key_write_define "size_t %s::key_write(void *data, size_t position) const"
#define key_read_define "size_t %s::key_read(const void *data, size_t position)"
#define key_calc_define "bool %s::key(ddsi_keyhash_t &hash) const"
#define typedef_write_define "size_t typedef_write_%s(const %s &obj, void* data, size_t position)"
#define typedef_write_size_define "size_t typedef_write_size_%s(const %s &obj, size_t position)"
#define typedef_read_define "size_t typedef_read_%s(%s &obj, const void* data, size_t position)"
#define typedef_max_size_define "size_t typedef_max_size_%s(const %s &obj, size_t position)"
#define typedef_key_size_define "size_t typedef_key_size_%s(const %s &obj, size_t position)"
#define typedef_key_max_size_define "size_t typedef_key_max_size_%s(const %s &obj, size_t position)"
#define typedef_key_write_define "size_t typedef_key_write_%s(const %s &obj, void *data, size_t position)"
#define typedef_key_read_define "size_t typedef_key_read_%s(%s &obj, const void *data, size_t position)"
#define typedef_write_call position_set "%stypedef_write_%s(%s, data, position);\n"
#define typedef_write_size_call position_set "%stypedef_write_size_%s(%s, position);\n"
#define typedef_read_call position_set "%stypedef_read_%s(%s, data, position);\n"
#define typedef_max_size_call position_set "%stypedef_max_size_%s(%s, position);\n"
#define typedef_key_size_call position_set "%stypedef_key_size_%s(%s, position);\n"
#define typedef_key_max_size_call position_set "%stypedef_key_max_size_%s(%s, position);\n"
#define typedef_key_write_call position_set "%stypedef_key_write_%s(%s, data, position);\n"
#define typedef_key_read_call position_set "%stypedef_key_read_%s(%s, data, position);\n"
#define union_case_max_check "if (case_max != UINT_MAX) "
#define union_case_max_incr union_case_max_check "case_max += "
#define union_case_max_set "case_max = "
#define union_case_max_set_limit union_case_max_set "UINT_MAX;\n"

#define char_cast "char"
#define bool_cast "bool"
#define int8_cast "int8_t"
#define uint8_cast "uint8_t"
#define int16_cast "int16_t"
#define uint16_cast "uint16_t"
#define int32_cast "int32_t"
#define uint32_cast "uint32_t"
#define int64_cast "int64_t"
#define uint64_cast "uint64_t"
#define float_cast "float"
#define double_cast "double"

struct idl_streamer_output
{
  size_t indent;
  idl_ostream_t* impl_stream;
  idl_ostream_t* head_stream;
};

typedef struct context context_t;

struct functioncontents {
  int currentalignment;
  int accumulatedalignment;
  bool alignmentpresent;
  bool sequenceentriespresent;
};

typedef struct functioncontents functioncontents_t;

static void reset_function_contents(functioncontents_t* funcs)
{
  assert(funcs);
  funcs->currentalignment = -1;
  funcs->accumulatedalignment = 0;
  funcs->alignmentpresent = false;
  funcs->sequenceentriespresent = false;
}

struct context
{
  idl_streamer_output_t* str;
  char* context;
  idl_ostream_t* write_size_stream;
  idl_ostream_t* write_stream;
  idl_ostream_t* read_stream;
  idl_ostream_t* key_size_stream;
  idl_ostream_t* max_size_stream;
  idl_ostream_t* max_size_intermediate_stream;
  idl_ostream_t* key_max_size_stream;
  idl_ostream_t* key_max_size_intermediate_stream;
  idl_ostream_t* key_write_stream;
  idl_ostream_t* key_read_stream;
  size_t depth;
  functioncontents_t streamer_funcs;
  functioncontents_t key_funcs;
  context_t* parent;
  bool in_union;
  bool key_max_size_unlimited;
  bool max_size_unlimited;
};

static void reset_alignment(context_t* ctx, bool is_key);
static idl_retcode_t add_default_case(context_t* ctx);
static idl_retcode_t process_instance(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_constructed_type_impl(context_t* ctx, const char* accessor, bool is_key, bool key_is_all_members);
static idl_retcode_t process_struct_definition(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_union_definition(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_string_impl(context_t* ctx, const char *accessor, idl_string_t* spec, bool is_key);
static idl_retcode_t process_sequence_impl(context_t* ctx, const char* accessor, idl_sequence_t* spec, bool is_key);
static idl_type_spec_t* resolve_typedef(idl_type_spec_t* def);
static idl_retcode_t process_typedef_definition(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_typedef_instance_impl(context_t* ctx, const char* accessor, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_known_width(context_t* ctx, const char* accessor, idl_node_t* typespec, bool is_key);
static idl_retcode_t process_sequence_entries(context_t* ctx, const char* accessor, bool plusone, bool is_key);
static int determine_byte_width(idl_node_t* typespec);
static char* determine_cast(idl_node_t* typespec);
static idl_retcode_t check_alignment(context_t* ctx, int bytewidth, bool is_key);
static idl_retcode_t add_null(context_t* ctx, int nbytes, bool stream, bool is_key);
static idl_retcode_t process_member(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_module_definition(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_case(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t process_case_label(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data);
static idl_retcode_t write_instance_funcs(context_t* ctx, const char* write_accessor, const char* read_accessor, bool is_key, bool key_is_all_members);
static context_t* create_context(const char* name);
static context_t* child_context(context_t* ctx, const char* name);
static void close_context(context_t* ctx, idl_streamer_output_t* str);
static char* generate_accessor(idl_declarator_t* decl);
static idl_retcode_t check_start_array(context_t* ctx, char** accessor, idl_declarator_t* decl, bool is_key);
static idl_retcode_t check_stop_array(context_t* ctx, idl_declarator_t* decl, bool is_key);
static void store_locals(context_t* ctx, bool* storage);
static void load_locals(context_t* ctx, bool* storage);
static bool has_keys(idl_type_spec_t* spec);

static char* generatealignment(int alignto)
{
  char* returnval = NULL;
  if (alignto < 2)
  {
    returnval = idl_strdup("0;");
  }
  else if (alignto == 2)
  {
    returnval = idl_strdup("position&0x1;");
  }
  else
  {
    int mask = 0xFFFFFF;
    while (mask != 0)
    {
      if (alignto == mask + 1)
      {
        (void)idl_asprintf(&returnval, primitive_calc_alignment_shift, alignto, mask, mask);
        return returnval;
      }
      mask >>= 1;
    }

    (void)idl_asprintf(&returnval, primitive_calc_alignment_modulo, alignto, alignto, alignto);
  }
  return returnval;
}

void reset_alignment(context_t* ctx, bool is_key)
{
  ctx->streamer_funcs.currentalignment = -1;
  ctx->streamer_funcs.accumulatedalignment = 0;
  if (is_key)
  {
    ctx->key_funcs.currentalignment = -1;
    ctx->key_funcs.accumulatedalignment = 0;
  }
}

int determine_byte_width(idl_node_t* typespec)
{
  if (NULL == typespec)
    return -1;

  if (idl_is_enum(typespec))
    return 4;

  switch (idl_mask(typespec) & BASE_TYPE_MASK)
  {
  case IDL_INT8:
  case IDL_UINT8:
  case IDL_CHAR:
  case IDL_BOOL:
  case IDL_OCTET:
    return 1;
  case IDL_SHORT:
  case IDL_USHORT:
  case IDL_INT16:
  case IDL_UINT16:
    return 2;
  case IDL_LONG:
  case IDL_ULONG:
  case IDL_INT32:
  case IDL_UINT32:
  case IDL_FLOAT:
    return 4;
  case IDL_LLONG:
  case IDL_ULLONG:
  case IDL_INT64:
  case IDL_UINT64:
  case IDL_DOUBLE:
    return 8;
  }

  return -1;
}

idl_streamer_output_t* create_idl_streamer_output()
{
  idl_streamer_output_t* ptr = calloc(sizeof(idl_streamer_output_t),1);
  if (NULL != ptr)
  {
    ptr->impl_stream = create_idl_ostream(NULL);
    ptr->head_stream = create_idl_ostream(NULL);
  }
  return ptr;
}

void destruct_idl_streamer_output(idl_streamer_output_t* str)
{
  if (NULL == str)
    return;

  if (str->impl_stream != NULL)
  {
    destruct_idl_ostream(str->impl_stream);
    destruct_idl_ostream(str->head_stream);
  }
  free(str);
}

idl_ostream_t* get_idl_streamer_impl_buf(const idl_streamer_output_t* str)
{
  return str->impl_stream;
}

idl_ostream_t* get_idl_streamer_head_buf(const idl_streamer_output_t* str)
{
  return str->head_stream;
}

context_t* create_context(const char* name)
{
  context_t* ptr = calloc(sizeof(context_t),1);
  if (NULL == ptr)
    goto fail0;

  if (NULL == (ptr->str = create_idl_streamer_output()))
    goto fail1;
  if (NULL == (ptr->context = idl_strdup(name)))
    goto fail2;
  if (NULL == (ptr->write_size_stream = create_idl_ostream(NULL)))
    goto fail3;
  if (NULL == (ptr->write_stream = create_idl_ostream(NULL)))
    goto fail4;
  if (NULL == (ptr->read_stream = create_idl_ostream(NULL)))
    goto fail5;
  if (NULL == (ptr->key_size_stream = create_idl_ostream(NULL)))
    goto fail6;
  if (NULL == (ptr->max_size_stream = create_idl_ostream(NULL)))
    goto fail7;
  if (NULL == (ptr->max_size_intermediate_stream = create_idl_ostream(NULL)))
    goto fail8;
  if (NULL == (ptr->key_max_size_stream = create_idl_ostream(NULL)))
    goto fail9;
  if (NULL == (ptr->key_max_size_intermediate_stream = create_idl_ostream(NULL)))
    goto fail10;
  if (NULL == (ptr->key_write_stream = create_idl_ostream(NULL)))
    goto fail11;
  if (NULL == (ptr->key_read_stream = create_idl_ostream(NULL)))
    goto fail12;
  goto success;

fail12:
  destruct_idl_ostream(ptr->key_write_stream);
fail11:
  destruct_idl_ostream(ptr->key_max_size_intermediate_stream);
fail10:
  destruct_idl_ostream(ptr->key_max_size_stream);
fail9:
  destruct_idl_ostream(ptr->max_size_intermediate_stream);
fail8:
  destruct_idl_ostream(ptr->max_size_stream);
fail7:
  destruct_idl_ostream(ptr->key_size_stream);
fail6:
  destruct_idl_ostream(ptr->read_stream);
fail5:
  destruct_idl_ostream(ptr->write_stream);
fail4:
  destruct_idl_ostream(ptr->write_size_stream);
fail3:
  free(ptr->context);
fail2:
  destruct_idl_streamer_output(ptr->str);
fail1:
  free(ptr);
success:
  reset_function_contents(&ptr->streamer_funcs);
  reset_function_contents(&ptr->key_funcs);
fail0:
  return ptr;
}

context_t* child_context(context_t* ctx, const char* name)
{
  context_t* ptr = create_context(name);
  if (!ptr)
    return NULL;

  ptr->parent = ctx;
  ptr->depth = ctx->depth + 1;

  return ptr;
}

void close_context(context_t* ctx, idl_streamer_output_t* str)
{
  if (get_ostream_buffer_position(ctx->str->head_stream) != 0)
  {
    if (ctx->parent)
    {
      //there is a parent context (so all statements are made inside a namespace)
      //move the contents to the parent
      format_header_stream(1, ctx->parent, namespace_declaration, ctx->context);
      format_header_stream(1, ctx->parent, open_block "\n");
      transfer_ostream_buffer(ctx->str->head_stream, ctx->parent->str->head_stream);
      format_header_stream(1, ctx->parent, namespace_closure, ctx->context);
    }
    else if (str)
    {
      //no parent context (so this is the root namespace)
      //move the contents to the "exit" stream
      format_ostream(str->head_stream, "%s", get_ostream_buffer(ctx->str->head_stream));
    }
  }

  //bundle impl contents
  transfer_ostream_buffer(ctx->write_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->write_size_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->max_size_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->key_size_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->key_max_size_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->key_write_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->key_read_stream, ctx->str->impl_stream);
  transfer_ostream_buffer(ctx->read_stream, ctx->str->impl_stream);

  if (get_ostream_buffer_position(ctx->str->impl_stream) != 0)
  {
    if (ctx->parent)
    {
      //there is a parent context (so all statements are made inside a namespace)
      //move the contents to the parent
      format_impl_stream(1, ctx->parent, namespace_declaration, ctx->context);
      format_impl_stream(1, ctx->parent, open_block "\n");
      transfer_ostream_buffer(ctx->str->impl_stream, ctx->parent->str->impl_stream);
      format_impl_stream(1, ctx->parent, namespace_closure, ctx->context);
    }
    else if (str)
    {
      //no parent context (so this is the root namespace)
      //move the contents to the "exit" stream
      format_ostream(str->impl_stream, "%s", get_ostream_buffer(ctx->str->impl_stream));
    }
  }

  destruct_idl_ostream(ctx->write_stream);
  destruct_idl_ostream(ctx->write_size_stream);
  destruct_idl_ostream(ctx->key_size_stream);
  destruct_idl_ostream(ctx->max_size_stream);
  destruct_idl_ostream(ctx->max_size_intermediate_stream);
  destruct_idl_ostream(ctx->key_max_size_stream);
  destruct_idl_ostream(ctx->key_max_size_intermediate_stream);
  destruct_idl_ostream(ctx->key_write_stream);
  destruct_idl_ostream(ctx->key_read_stream);
  destruct_idl_ostream(ctx->read_stream);

  destruct_idl_streamer_output(ctx->str);
  free(ctx->context);
  free(ctx);
}

char* generate_accessor(idl_declarator_t* decl)
{
  char* accessor = NULL;
  if (decl)
  {
    char* cpp11name = get_cpp11_name(idl_identifier(decl));
    if (!cpp11name)
      return NULL;

    idl_asprintf(&accessor, member_access, cpp11name);
    free(cpp11name);
  }
  else
  {
    accessor = idl_strdup("obj");
  }
  return accessor;
}

idl_retcode_t check_start_array(context_t* ctx, char **accessor, idl_declarator_t* decl, bool is_key)
{
  idl_retcode_t ret = IDL_RETCODE_OK;
  idl_const_expr_t* ce = decl ? decl->const_expr : NULL;
  while (ce)
  {
    if (idl_mask(ce) & IDL_CONST)
    {
      uint64_t entries = array_entries(ce);
      if (entries)
      {
        ctx->depth++;
        format_write_size_stream(1, ctx, is_key, array_iterate, ctx->depth-1, ctx->depth-1, entries, ctx->depth-1);
        format_write_stream(1, ctx, is_key, array_iterate, ctx->depth-1, ctx->depth-1, entries, ctx->depth-1);
        format_read_stream(1, ctx, is_key, array_iterate, ctx->depth-1, ctx->depth-1, entries, ctx->depth-1);
        format_max_size_intermediate_stream(1, ctx, is_key, array_iterate, ctx->depth-1, ctx->depth-1, entries, ctx->depth-1);

        //modify accessor
        char* temp = NULL;
        if (0 > idl_replace_indices_with_values(&temp, array_accessor, *accessor, ctx->depth-1))
        {
          ret = IDL_RETCODE_NO_MEMORY;
          goto fail;
        }

        free(*accessor);
        *accessor = temp;
      }
    }
    ce = ((idl_node_t*)ce)->next;
  }

fail:
  return ret;
}

idl_retcode_t check_stop_array(context_t* ctx, idl_declarator_t* decl, bool is_key)
{
  idl_retcode_t ret = IDL_RETCODE_OK;
  idl_const_expr_t* ce = decl ? decl->const_expr : NULL;
  while (ce)
  {
    if (idl_mask(ce) & IDL_CONST)
    {
      if (array_entries(ce))
      {
        format_write_size_stream(1, ctx, is_key, close_block);
        format_write_stream(1, ctx, is_key, close_block);
        format_read_stream(1, ctx, is_key, close_block);
        format_max_size_intermediate_stream(1, ctx, is_key, close_block);

        ctx->depth--;
      }
    }
    ce = ((idl_node_t*)ce)->next;
  }

  return ret;
}

void store_locals(context_t* ctx, bool* storage)
{
  storage[0] = ctx->streamer_funcs.alignmentpresent;
  storage[1] = ctx->streamer_funcs.sequenceentriespresent;
  storage[2] = ctx->key_funcs.alignmentpresent;
  storage[3] = ctx->key_funcs.sequenceentriespresent;

  ctx->streamer_funcs.alignmentpresent = false;
  ctx->streamer_funcs.sequenceentriespresent = false;
  ctx->key_funcs.alignmentpresent = false;
  ctx->key_funcs.sequenceentriespresent = false;
}

void load_locals(context_t* ctx, bool* storage)
{
  ctx->streamer_funcs.alignmentpresent = storage[0];
  ctx->streamer_funcs.sequenceentriespresent = storage[1];
  ctx->key_funcs.alignmentpresent = storage[2];
  ctx->key_funcs.sequenceentriespresent = storage[3];
}

bool has_keys(idl_type_spec_t* spec)
{
  //is struct
  if (idl_is_struct(spec))
  {
    idl_member_t* mem = ((idl_struct_t*)spec)->members;
    while (mem)
    {
      if (mem->key) return true;
      mem = (idl_member_t*)(mem->node.next);
    }
  }
  //is union
  else if (idl_is_union(spec))
  {
    if (idl_is_masked(((idl_union_t*)spec)->switch_type_spec, IDL_KEY)) return true;
  }
  //is typedef
  else if (idl_is_typedef(spec))
  {
    idl_type_spec_t* newspec = resolve_typedef(spec);
    return has_keys(newspec);
  }
  return false;
}

idl_retcode_t process_member(const idl_pstate_t* pstate, idl_visit_t* visit, const void* node, void* user_data)
{
  (void)pstate;
  (void)visit;

  context_t* ctx = *(context_t**)user_data;
  idl_member_t* mem = (idl_member_t*)node;
  return process_instance(ctx, mem->declarators, mem->type_spec, mem->key);
}

idl_retcode_t process_instance(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key)
{
  idl_retcode_t returnval = IDL_RETCODE_OK;
  char* accessor = generate_accessor(decl);
  if (!accessor)
  {
    returnval = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }

  if ((returnval = check_alignment(ctx, determine_byte_width(spec), is_key)))
    goto fail2;

  if ((returnval = check_start_array(ctx, &accessor, decl, is_key)))
    goto fail2;

  if (idl_is_base_type(spec) || idl_is_enum(spec))
  {
    if ((returnval = process_known_width(ctx, accessor, spec, is_key)))
      goto fail2;
  }
  else if (idl_is_struct(spec) || idl_is_union(spec))
  {
    if ((returnval = process_constructed_type_impl(ctx, accessor, is_key, !has_keys(spec))))
      goto fail2;
  }
  else if (idl_is_string(spec))
  {
    if ((returnval = process_string_impl(ctx, accessor, spec, is_key)))
      goto fail2;
  }
  else if (idl_is_sequence(spec))
  {
    if ((returnval = process_sequence_impl(ctx, accessor, spec, is_key)))
      goto fail2;
  } else {
    assert(idl_is_typedef(spec));

    idl_type_spec_t* ispec = resolve_typedef(spec);

    if (idl_is_base_type(ispec))
    {
      if ((returnval = process_known_width(ctx, accessor, ispec, is_key)))
        goto fail2;
    }
    else if ((returnval = process_typedef_instance_impl(ctx, accessor, spec, is_key)))
    {
      goto fail2;
    }
  }

  if ((returnval = check_stop_array(ctx, decl, is_key)))
    goto fail2;

  idl_declarator_t* nextdecl = decl ? (idl_declarator_t*)(decl->node.next) : NULL;
  if (nextdecl)
    returnval = process_instance(ctx, nextdecl, spec, is_key);

fail2:
  free(accessor);
fail1:
  return returnval;
}

idl_retcode_t process_constructed_type_impl(context_t* ctx, const char* accessor, bool is_key, bool key_is_all_members)
{
  return write_instance_funcs(ctx, accessor, accessor, is_key, key_is_all_members);
}

idl_retcode_t write_instance_funcs(context_t* ctx, const char* write_accessor, const char* read_accessor, bool is_key, bool key_is_all_members)
{

  format_write_stream(1, ctx, false , instance_write_func, write_accessor);
  format_write_size_stream(1, ctx, false, instance_size_func_calc, write_accessor);
  format_read_stream(1, ctx, false, instance_read_func, read_accessor);

  if (ctx->in_union)
  {
    format_max_size_intermediate_stream(1, ctx, false, instance_max_size_func_union, write_accessor);
  }
  else
  {
    format_max_size_intermediate_stream(1, ctx, false, instance_max_size_func, write_accessor);
  }

  if (is_key)
  {
    if (!key_is_all_members)
    {
      format_key_write_stream(1, ctx, instance_key_write_func, write_accessor);
      format_key_read_stream(1, ctx, instance_key_read_func, read_accessor);
      format_key_size_stream(1, ctx, instance_key_size_func_calc, write_accessor);
      if (ctx->in_union)
      {
        format_key_max_size_intermediate_stream(1, ctx, instance_key_max_size_union_func_calc, write_accessor);
      }
      else
      {
        format_key_max_size_intermediate_stream(1, ctx, instance_key_max_size_func_calc, write_accessor);
      }
    }
    else
    {
      format_key_write_stream(1, ctx, instance_write_func, write_accessor);
      format_key_read_stream(1, ctx, instance_read_func, read_accessor);
      format_key_size_stream(1, ctx, instance_size_func_calc, write_accessor);
      if (ctx->in_union)
      {
        format_key_max_size_intermediate_stream(1, ctx, instance_max_size_func_union, write_accessor);
      }
      else
      {
        format_key_max_size_intermediate_stream(1, ctx, instance_max_size_func, write_accessor);
      }
    }
  }

  reset_alignment(ctx, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t check_alignment(context_t* ctx, int bytewidth, bool is_key)
{
  if (bytewidth < 1)
    return IDL_RETCODE_OK;

  if (ctx->streamer_funcs.currentalignment == bytewidth)
    ctx->streamer_funcs.accumulatedalignment += bytewidth;
  if (is_key && ctx->key_funcs.currentalignment == bytewidth)
    ctx->key_funcs.accumulatedalignment += bytewidth;

  if (ctx->streamer_funcs.currentalignment == bytewidth && ctx->key_funcs.currentalignment == bytewidth)
    return IDL_RETCODE_OK;

  char* buffer = generatealignment(bytewidth);
  if (!buffer)
    return IDL_RETCODE_NO_MEMORY;

  idl_retcode_t returnval = IDL_RETCODE_OK;
  if ((0 > ctx->streamer_funcs.currentalignment || bytewidth > ctx->streamer_funcs.currentalignment) && bytewidth != 1)
  {
    format_write_stream(1, ctx, false, "  %s" alignmentbytes " = %s" align_comment, ctx->streamer_funcs.alignmentpresent ? "" : "size_t ", ctx->depth, buffer);
    ctx->streamer_funcs.alignmentpresent = true;
    format_write_stream(1, ctx, false, primitive_write_func_alignment, ctx->depth);
    format_write_stream(1, ctx, false, position_incr_alignment incr_comment, ctx->depth);

    format_write_size_stream(1, ctx, false, position_incr "%s" align_comment, buffer);

    if (ctx->in_union)
    {
      format_max_size_intermediate_stream(1, ctx, false, union_case_max_incr "%s" align_comment, buffer);
    }
    else
    {
      format_max_size_intermediate_stream(1, ctx, false, max_size_check position_incr "%s" align_comment, buffer);
    }

    format_read_stream(1, ctx, false, position_incr "%s" align_comment, buffer);

    ctx->streamer_funcs.accumulatedalignment = 0;
    ctx->streamer_funcs.currentalignment = bytewidth;
  }
  else
  {
    int missingbytes = (bytewidth - (ctx->streamer_funcs.accumulatedalignment % bytewidth)) % bytewidth;
    if (0 != missingbytes)
    {
      if ((returnval = add_null(ctx, missingbytes, true, false)))
        goto fail;
      ctx->streamer_funcs.accumulatedalignment = 0;
    }
  }

  if (is_key)
  {
    if ((0 > ctx->key_funcs.currentalignment || bytewidth > ctx->key_funcs.currentalignment) && bytewidth != 1)
    {
      format_key_write_stream(1, ctx, "  %s" alignmentbytes " = %s" align_comment, ctx->key_funcs.alignmentpresent ? "" : "size_t ", ctx->depth, buffer);
      ctx->key_funcs.alignmentpresent = true;
      format_key_write_stream(1, ctx, primitive_write_func_alignment, ctx->depth);
      format_key_write_stream(1, ctx, position_incr_alignment incr_comment, ctx->depth);

      format_key_size_stream(1, ctx, position_incr "%s" align_comment, buffer);

      if (ctx->in_union)
      {
        format_key_max_size_intermediate_stream(1, ctx, max_size_check union_case_max_incr "%s" align_comment, buffer);
      }
      else
      {
        format_key_max_size_intermediate_stream(1, ctx, max_size_check position_incr "%s" align_comment, buffer);
      }

      format_key_read_stream(1, ctx, position_incr "%s" align_comment, buffer);

      ctx->key_funcs.accumulatedalignment = 0;
      ctx->key_funcs.currentalignment = bytewidth;
    }
    else
    {
      int missingbytes = (bytewidth - (ctx->key_funcs.accumulatedalignment % bytewidth)) % bytewidth;
      if (0 != missingbytes)
      {
        if ((returnval = add_null(ctx, missingbytes, false, true)))
          goto fail;
        ctx->key_funcs.accumulatedalignment = 0;
      }
    }
  }

fail:
  free(buffer);

  return returnval;
}

idl_retcode_t add_null(context_t* ctx, int nbytes, bool stream, bool is_key)
{
  if (stream)
  {
    format_write_stream(1, ctx, false, primitive_write_func_padding, nbytes);
    format_write_stream(1, ctx, false, primitive_incr_pos incr_comment, nbytes);
    format_write_size_stream(1, ctx, false, primitive_incr_pos padding_comment, nbytes);
    format_read_stream(1, ctx, false, primitive_incr_pos padding_comment, nbytes);
    if (ctx->in_union)
    {
      format_max_size_intermediate_stream(1, ctx, false, union_case_max_incr " %d;" padding_comment, nbytes);
    }
    else
    {
      format_max_size_intermediate_stream(1, ctx, false, primitive_incr_pos padding_comment, nbytes);
    }
  }

  if (is_key)
  {
    format_key_write_stream(1, ctx, primitive_write_func_padding, nbytes);
    format_key_write_stream(1, ctx, primitive_incr_pos incr_comment, nbytes);
    format_key_size_stream(1, ctx, primitive_incr_pos padding_comment, nbytes);
    format_key_read_stream(1, ctx, primitive_incr_pos padding_comment, nbytes);
    if (ctx->in_union)
    {
      format_key_max_size_intermediate_stream(1, ctx, union_case_max_incr " %d;" padding_comment, nbytes);
    }
    else
    {
      format_key_max_size_intermediate_stream(1, ctx, primitive_incr_pos padding_comment, nbytes);
    }
  }

  return IDL_RETCODE_OK;
}

char* determine_cast(idl_node_t* typespec)
{
  if (idl_is_enum(typespec))
  {
    char* ns = NULL, *returnval = NULL;
    resolve_namespace(typespec, &ns);

    if (!ns)
      return NULL;

    idl_asprintf(&returnval, "%s%s", ns, ((idl_enum_t*)typespec)->name->identifier);
    free(ns);
    return returnval;
  }
  else if (idl_is_string(typespec))
  {
    return idl_strdup(char_cast);
  }

  switch (idl_mask(typespec) & BASE_TYPE_MASK)
  {
  case IDL_CHAR:
    return idl_strdup(char_cast);
    break;
  case IDL_BOOL:
    return idl_strdup(bool_cast);
    break;
  case IDL_INT8:
    return idl_strdup(int8_cast);
    break;
  case IDL_UINT8:
  case IDL_OCTET:
    return idl_strdup(uint8_cast);
    break;
  case IDL_INT16:
  case IDL_SHORT:
    return idl_strdup(int16_cast);
    break;
  case IDL_UINT16:
  case IDL_USHORT:
    return idl_strdup(uint16_cast);
    break;
  case IDL_INT32:
  case IDL_LONG:
    return idl_strdup(int32_cast);
    break;
  case IDL_UINT32:
  case IDL_ULONG:
    return idl_strdup(uint32_cast);
    break;
  case IDL_INT64:
  case IDL_LLONG:
    return idl_strdup(int64_cast);
    break;
  case IDL_UINT64:
  case IDL_ULLONG:
    return idl_strdup(uint64_cast);
    break;
  case IDL_FLOAT:
    return idl_strdup(float_cast);
    break;
  case IDL_DOUBLE:
    return idl_strdup(double_cast);
    break;
  }
  return NULL;
}

idl_retcode_t process_known_width(context_t* ctx, const char* accessor, idl_node_t* typespec, bool is_key)
{
  idl_retcode_t returnval = IDL_RETCODE_OK;
  int bytewidth = determine_byte_width(typespec);

  char* cast = determine_cast(typespec);
  if (!cast)
  {
    returnval = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }

  if ((returnval = check_alignment(ctx, bytewidth, is_key)))
    goto fail2;

  format_write_stream(1, ctx, is_key, primitive_write_func_write, cast, accessor, accessor);
  format_write_stream(1, ctx, is_key, primitive_incr_pos incr_comment, bytewidth);

  format_write_size_stream(1, ctx, is_key, primitive_incr_pos bytes_for_member_comment, bytewidth, accessor);

  if (ctx->in_union)
  {
    format_max_size_intermediate_stream(1, ctx, is_key, union_case_max_incr " %d;\n", bytewidth);
  }
  else
  {
    format_max_size_intermediate_stream(1, ctx, is_key, max_size_incr_checked bytes_for_member_comment, bytewidth, accessor);
  }

  format_read_stream(1, ctx, is_key, primitive_read_func_read, accessor, cast, accessor);
  format_read_stream(1, ctx, is_key, primitive_incr_pos incr_comment, bytewidth);

fail2:
  free(cast);
fail1:
  return returnval;
}

idl_retcode_t process_sequence_entries(context_t* ctx, const char* accessor, bool plusone, bool is_key)
{
  idl_retcode_t returnval = IDL_RETCODE_OK;

  if ((returnval = check_alignment(ctx, 4, is_key)))
    goto fail;

  format_read_stream(1, ctx, is_key, "  ");
  format_write_stream(1, ctx, is_key, "  ");
  format_write_size_stream(1, ctx, is_key, "  ");
  if (!ctx->streamer_funcs.sequenceentriespresent)
  {
    format_read_stream(0, ctx, false, "uint32_t ");
    format_write_stream(0, ctx, false, "uint32_t ");
    format_write_size_stream(0, ctx, false, "uint32_t ");
    ctx->streamer_funcs.sequenceentriespresent = true;
  }

  if (is_key && !ctx->key_funcs.sequenceentriespresent)
  {
    format_key_read_stream(0, ctx, "uint32_t ");
    format_key_write_stream(0, ctx, "uint32_t ");
    format_key_size_stream(0, ctx, "uint32_t ");
    ctx->key_funcs.sequenceentriespresent = true;
  }

  format_read_stream(0, ctx, is_key, primitive_read_func_seq, ctx->depth);
  format_read_stream(1, ctx, is_key, primitive_incr_pos incr_comment, (int)4);

  format_write_stream(0, ctx, is_key, primitive_write_func_seq, ctx->depth, accessor, plusone ? "+1" : "");
  format_write_stream(1, ctx, is_key, primitive_write_func_seq2, ctx->depth, accessor);
  format_write_stream(1, ctx, is_key, primitive_incr_pos incr_comment, (int)4);

  format_write_size_stream(0, ctx, is_key, primitive_write_func_seq, ctx->depth, accessor, plusone ? "+1" : "");
  format_write_size_stream(1, ctx, is_key, primitive_incr_pos bytes_for_seq_entries_comment, (int)4);

  if (ctx->in_union)
  {
    format_max_size_intermediate_stream(1, ctx, is_key, union_case_max_incr "4;\n");
  }
  else
  {
    format_max_size_intermediate_stream(1, ctx, is_key, max_size_incr_checked bytes_for_seq_entries_comment, (int)4);
  }

fail:
  return returnval;
}

idl_retcode_t process_module_definition(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  context_t* ctx = *((context_t**)user_data);
  const idl_module_t* module = (const idl_module_t*)node;
  idl_retcode_t ret = IDL_RETCODE_NO_MEMORY;
  char* cpp11name = get_cpp11_name(idl_identifier(module));
  if (!cpp11name)
    goto fail;

  assert(ctx);

  ret = IDL_RETCODE_OK;
  if (module->definitions)
  {
    if (visit->type == IDL_EXIT)
    {
      context_t* parentctx = ctx->parent;
      close_context(ctx, NULL);
      *((context_t**)user_data) = parentctx;
    }
    else
    {
      context_t* newctx = child_context(ctx, cpp11name);
      *((context_t**)user_data) = newctx;
      ret = IDL_VISIT_REVISIT;
    }
  }

  free(cpp11name);
fail:
  return ret;
}

static void print_constructed_type_close(context_t* ctx, char* cpp11name)
{
  format_write_size_stream(1, ctx, true, position_return);
  format_write_size_stream(1, ctx, true, close_function);
  format_write_stream(1, ctx, true, position_return);
  format_write_stream(1, ctx, true, close_function);
  format_read_stream(1, ctx, true, position_return);
  format_read_stream(1, ctx, true, close_function);

  if (ctx->key_max_size_unlimited)
  {
    flush_ostream(ctx->key_max_size_intermediate_stream);
    format_key_max_size_stream(1, ctx, "  (void)position;\n");
    format_key_max_size_stream(1, ctx, "  return UINT_MAX;\n");
    ctx->key_max_size_unlimited = false;
  }
  else
  {
    transfer_ostream_buffer(ctx->key_max_size_intermediate_stream, ctx->key_max_size_stream);
    format_key_max_size_stream(1, ctx, position_return);
  }

  if (ctx->max_size_unlimited)
  {
    flush_ostream(ctx->max_size_intermediate_stream);
    format_max_size_stream(1, ctx, false, "  (void)position;\n");
    format_max_size_stream(1, ctx, false, "  return UINT_MAX;\n");
    ctx->max_size_unlimited = false;
  }
  else
  {
    transfer_ostream_buffer(ctx->max_size_intermediate_stream, ctx->max_size_stream);
    format_max_size_stream(1, ctx, false, position_return);
  }
  format_max_size_stream(1, ctx, true, close_function);

  format_key_write_stream(1, ctx, key_calc_define "\n", cpp11name);
  format_key_write_stream(1, ctx, "{\n");
  format_key_write_stream(1, ctx, "  size_t sz = key_size(0);\n");
  format_key_write_stream(1, ctx, "  size_t padding = 16 - sz%%16;\n");
  format_key_write_stream(1, ctx, "  if (sz != 0 && padding == 16) padding = 0;\n");
  format_key_write_stream(1, ctx, "  std::vector<unsigned char> buffer(sz+padding);\n");
  format_key_write_stream(1, ctx, "  memset(buffer.data()+sz,0x0,padding);\n");
  format_key_write_stream(1, ctx, "  key_write(buffer.data(),0);\n");
  format_key_write_stream(1, ctx, "  static bool (*fptr)(const std::vector<unsigned char>&, ddsi_keyhash_t &) = NULL;\n");
  format_key_write_stream(1, ctx, "  if (fptr == NULL)\n");
  format_key_write_stream(1, ctx, "  {\n");
  format_key_write_stream(1, ctx, "    if (key_max_size(0) <= 16)\n");
  format_key_write_stream(1, ctx, "    {\n");
  format_key_write_stream(1, ctx, "      //bind to unmodified function which just copies buffer into the keyhash\n");
  format_key_write_stream(1, ctx, "      fptr = &org::eclipse::cyclonedds::topic::simple_key;\n");
  format_key_write_stream(1, ctx, "    }\n");
  format_key_write_stream(1, ctx, "    else\n");
  format_key_write_stream(1, ctx, "    {\n");
  format_key_write_stream(1, ctx, "      //bind to MD5 hash function\n");
  format_key_write_stream(1, ctx, "      fptr = &org::eclipse::cyclonedds::topic::complex_key;\n");
  format_key_write_stream(1, ctx, "    }\n");
  format_key_write_stream(1, ctx, "  }\n");
  format_key_write_stream(1, ctx, "  return (*fptr)(buffer,hash);\n");
  format_key_write_stream(1, ctx, close_function);

  reset_function_contents(&ctx->streamer_funcs);
  reset_function_contents(&ctx->key_funcs);
}

static void print_constructed_type_open(context_t* ctx, char* cpp11name)
{
  format_write_stream(1, ctx, false, write_func_define "\n", cpp11name);
  format_write_stream(1, ctx, false, open_block);

  format_write_size_stream(1, ctx, false, write_size_func_define "\n", cpp11name);
  format_write_size_stream(1, ctx, false, open_block);

  format_max_size_stream(1, ctx, false, max_size_define "\n", cpp11name);
  format_max_size_stream(1, ctx, false, open_block);

  format_key_write_stream(1, ctx, key_write_define "\n", cpp11name);
  format_key_write_stream(1, ctx, open_block);
  format_key_write_stream(1, ctx, "  (void)data;\n");

  format_key_size_stream(1, ctx, key_size_define "\n", cpp11name);
  format_key_size_stream(1, ctx, open_block);

  format_key_max_size_stream(1, ctx, key_max_size_define "\n", cpp11name);
  format_key_max_size_stream(1, ctx, open_block);

  format_read_stream(1, ctx, false, read_func_define "\n", cpp11name);
  format_read_stream(1, ctx, false, open_block);

  format_key_read_stream(1, ctx, key_read_define "\n", cpp11name);
  format_key_read_stream(1, ctx, open_block);
  format_key_read_stream(1, ctx, "  (void)data;\n");

  ctx->streamer_funcs.currentalignment = -1;
  ctx->streamer_funcs.alignmentpresent = false;
  ctx->streamer_funcs.sequenceentriespresent = false;
  ctx->streamer_funcs.accumulatedalignment = 0;
}

idl_retcode_t process_case(const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  context_t* ctx = *((context_t**)user_data);
  const idl_case_t* _case = (const idl_case_t*)node;
  idl_retcode_t ret = IDL_RETCODE_OK;

  if (visit->type == IDL_EXIT)
  {
    //store the offsets and alignments for this case
    functioncontents_t sfuncs = ctx->streamer_funcs;
    functioncontents_t kfuncs = ctx->key_funcs;
    reset_function_contents(&ctx->streamer_funcs);
    reset_function_contents(&ctx->key_funcs);

    format_write_stream(1, ctx, false, "  {\n");
    format_write_size_stream(1, ctx, false, "  {\n");
    format_read_stream(1, ctx, false, "  {\n");
    format_max_size_intermediate_stream(1, ctx, false, "{  //cases\n");
    format_max_size_intermediate_stream(1, ctx, false, "  size_t case_max = position;\n");
    ctx->depth++;

    char *cpp11name = get_cpp11_name(idl_identifier(_case->declarator));
    if (!cpp11name)
    {
      ret = IDL_RETCODE_NO_MEMORY;
      goto fail1;
    }
    char *cpp11type = get_cpp11_type(_case->type_spec);
    if (!cpp11type)
    {
      ret = IDL_RETCODE_NO_MEMORY;
      goto fail2;
    }

    format_write_stream(1, ctx, false, "  auto& obj = %s();\n", cpp11name);
    format_write_size_stream(1, ctx, false, "  auto& obj = %s();\n", cpp11name);
    format_write_size_stream(1, ctx, false, "  (void)obj;\n", cpp11name);
    format_read_stream(1, ctx, false, "  %s obj;\n", cpp11type);

    ret = process_instance(ctx, NULL, _case->type_spec, false);
    if (ret)
      goto fail3;

    format_read_stream(1, ctx, false, "  %s(obj,_disc_temp);\n", cpp11name);

    ctx->depth--;
    format_max_size_intermediate_stream(1, ctx, false, "  union_max = std::max(case_max,union_max);\n");
    format_max_size_intermediate_stream(1, ctx, false, "}\n");
    format_write_stream(1, ctx, false, "  }\n");
    format_write_stream(1, ctx, false, union_case_ending);
    format_write_size_stream(1, ctx, false, "  }\n");
    format_write_size_stream(1, ctx, false, union_case_ending);
    format_read_stream(1, ctx, false, "  }\n");
    format_read_stream(1, ctx, false, union_case_ending);

    ctx->streamer_funcs = sfuncs;
    ctx->key_funcs = kfuncs;
  fail3:
    free(cpp11type);
  fail2:
    free(cpp11name);
  }
  else
  {
    //first process all the case labels
    ret = IDL_VISIT_REVISIT;
  }
fail1:
  return ret;
}

idl_type_spec_t* resolve_typedef(idl_type_spec_t* spec)
{
  while (NULL != spec)
  {
    if (idl_is_declarator(spec))
      spec = ((idl_node_t*)spec)->parent;
    else if (idl_is_typedef(spec))
      spec = ((idl_typedef_t*)spec)->type_spec;
    else
      break;
  }
  return spec;
}

idl_retcode_t print_inherit_spec(context_t* ctx, idl_inherit_spec_t* ispec)
{
  idl_retcode_t returnval = IDL_RETCODE_OUT_OF_MEMORY;

  char* base_cpp11name = get_cpp11_name(idl_identifier(ispec->base));
  if (!base_cpp11name)
    goto fail1;
  char* ns = NULL;
  resolve_namespace((idl_node_t*)ispec->base, &ns);
  if (!ns)
    goto fail2;

  char* write_accessor = NULL, * read_accessor = NULL;
  if (idl_asprintf(&write_accessor, const_ref_cast, ns, base_cpp11name) == -1)
    goto fail3;
  if (idl_asprintf(&read_accessor, ref_cast, ns, base_cpp11name) == -1)
    goto fail4;

  returnval = write_instance_funcs(ctx, write_accessor, read_accessor, true, false);

  free(read_accessor);
fail4:
  free(write_accessor);
fail3:
  free(ns);
fail2:
  free(base_cpp11name);
fail1:
  return returnval;
}

idl_retcode_t process_struct_definition(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;

  idl_retcode_t returnval = IDL_RETCODE_OUT_OF_MEMORY;
  context_t* ctx = *(context_t**)user_data;
  char* cpp11name = get_cpp11_name(idl_identifier(node));
  if (!cpp11name)
    goto fail;

  if (visit->type == IDL_EXIT)
  {
    print_constructed_type_close(ctx, cpp11name);
    returnval = IDL_RETCODE_OK;
  }
  else
  {
    print_constructed_type_open(ctx, cpp11name);
    idl_struct_t* _struct = (idl_struct_t*)node;

    if (_struct->inherit_spec &&
        (returnval = print_inherit_spec(ctx, _struct->inherit_spec)))
        goto fail;

    returnval = IDL_VISIT_REVISIT;
  }

  free(cpp11name);
fail:
  return returnval;
}

static idl_retcode_t union_switch_start(context_t* ctx, idl_switch_type_spec_t* st)
{
  bool disc_is_key = idl_is_masked(st, IDL_KEY);
  idl_retcode_t returnval = IDL_RETCODE_OK;

  /*create temporary discriminator holders since
  the discriminator cannot be set through reference*/
  char* discriminator_cast = determine_cast(st->type_spec);
  if (!discriminator_cast)
  {
    returnval = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }

  format_write_stream(1, ctx, false, "  %s _disc_temp = _d();\n", discriminator_cast);
  format_write_size_stream(1, ctx, false, "  %s _disc_temp = _d();\n", discriminator_cast);
  format_read_stream(1, ctx, false, "  %s _disc_temp;\n", discriminator_cast);
  if ((returnval = process_known_width(ctx, "_disc_temp", st->type_spec, disc_is_key)))
    goto fail2;

  format_write_size_stream(1, ctx, false, union_switch);
  format_write_size_stream(1, ctx, false, "  {\n");
  format_write_stream(1, ctx, false, union_switch);
  format_write_stream(1, ctx, false, "  {\n");
  format_read_stream(1, ctx, false, union_switch);
  format_read_stream(1, ctx, false, "  {\n");

  ctx->in_union = true;
  format_max_size_intermediate_stream(1, ctx, false, "  size_t union_max = position;\n");
  ctx->depth++;

fail2:
  free(discriminator_cast);
fail1:
  return returnval;
}

static idl_retcode_t union_switch_stop(context_t* ctx)
{
  ctx->depth--;

  ctx->streamer_funcs.currentalignment = -1;
  ctx->streamer_funcs.accumulatedalignment = 0;
  ctx->key_funcs.currentalignment = -1;
  ctx->key_funcs.accumulatedalignment = 0;
  format_max_size_intermediate_stream(1, ctx, false, "  position = union_max;\n");
  ctx->in_union = false;

  format_write_stream(1, ctx, false, "  }\n");
  format_write_size_stream(1, ctx, false, "  }\n");
  format_read_stream(1, ctx, false, "  }\n");

  return IDL_RETCODE_OK;
}

idl_retcode_t process_union_definition(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;

  idl_retcode_t returnval = IDL_RETCODE_OUT_OF_MEMORY;
  context_t* ctx = *(context_t**)user_data;
  char* cpp11name = get_cpp11_name(idl_identifier(node));
  if (!cpp11name)
    goto fail1;

  if (visit->type == IDL_EXIT)
  {
    if ((returnval = union_switch_stop(ctx)))
      goto fail2;

    print_constructed_type_close(ctx, cpp11name);

    returnval = IDL_RETCODE_OK;
  }
  else
  {
    print_constructed_type_open(ctx, cpp11name);

    idl_union_t* _union = (idl_union_t*)node;
    if ((returnval = union_switch_start(ctx, _union->switch_type_spec)))
      goto fail2;

    returnval = IDL_VISIT_REVISIT;
  }

fail2:
  free(cpp11name);
fail1:
  return returnval;
}

idl_retcode_t process_typedef_definition(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  context_t* ctx = *((context_t**)user_data);
  const idl_typedef_t* typedef_node = (const idl_typedef_t*)node;
  idl_type_spec_t* spec = resolve_typedef(typedef_node->type_spec);
  idl_retcode_t returnval = IDL_RETCODE_OK;

  if (!idl_is_base_type(spec) &&
      !idl_is_enum(spec))
  {
    const char* tsname = idl_identifier(typedef_node->declarators);

    format_write_stream(1, ctx, false, typedef_write_define "\n", tsname, tsname);
    format_write_stream(1, ctx, false, open_block);
    format_write_size_stream(1, ctx, false, typedef_write_size_define "\n", tsname, tsname);
    format_write_size_stream(1, ctx, false, open_block);
    format_max_size_stream(1, ctx, false, typedef_max_size_define "\n", tsname, tsname);
    format_max_size_stream(1, ctx, false, open_block);
    format_max_size_stream(1, ctx, false, "  (void)obj;\n");
    format_read_stream(1, ctx, false, typedef_read_define "\n", tsname, tsname);
    format_read_stream(1, ctx, false, open_block);
    format_key_write_stream(1, ctx, typedef_key_write_define "\n", tsname, tsname);
    format_key_write_stream(1, ctx, open_block);
    format_key_read_stream(1, ctx, typedef_key_read_define "\n", tsname, tsname);
    format_key_read_stream(1, ctx, open_block);
    format_key_size_stream(1, ctx, typedef_key_size_define "\n", tsname, tsname);
    format_key_size_stream(1, ctx, open_block);
    format_key_max_size_stream(1, ctx, typedef_key_max_size_define "\n", tsname, tsname);
    format_key_max_size_stream(1, ctx, open_block);
    format_key_max_size_stream(1, ctx, "  (void)obj;\n");

    format_header_stream(1, ctx, typedef_write_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_write_size_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_max_size_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_read_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_key_write_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_key_read_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_key_size_define ";\n\n", tsname, tsname);
    format_header_stream(1, ctx, typedef_key_max_size_define ";\n\n", tsname, tsname);

    /*a typedef declaration generates write functions, but the object they write is called "obj"
    since this is not an instance inside a constructed type*/
    if ((returnval = process_instance(ctx, NULL, spec, true)))
      return returnval;

    if (ctx->max_size_unlimited)
    {
      flush_ostream(ctx->max_size_intermediate_stream);
      format_max_size_stream(1, ctx, false, "  (void)position;\n");
      format_max_size_stream(1, ctx, false, "  return UINT_MAX;\n");
      ctx->max_size_unlimited = false;
    }
    else
    {
      transfer_ostream_buffer(ctx->max_size_intermediate_stream, ctx->max_size_stream);
      format_max_size_stream(1, ctx, false, position_return);
    }

    if (ctx->key_max_size_unlimited)
    {
      flush_ostream(ctx->key_max_size_intermediate_stream);
      format_key_max_size_stream(1, ctx, "  (void)position;\n");
      format_key_max_size_stream(1, ctx, "  return UINT_MAX;\n");
      ctx->key_max_size_unlimited = false;
    }
    else
    {
      transfer_ostream_buffer(ctx->key_max_size_intermediate_stream, ctx->key_max_size_stream);
      format_key_max_size_stream(1, ctx, position_return);
    }

    format_write_stream(1, ctx, true, position_return);
    format_write_stream(1, ctx, true,close_function);
    format_write_size_stream(1, ctx, true, position_return);
    format_write_size_stream(1, ctx, true, close_function);
    format_read_stream(1, ctx, true, position_return);
    format_read_stream(1, ctx, true, close_function);
    format_max_size_stream(1, ctx, true, close_function);

    reset_function_contents(&ctx->streamer_funcs);
    reset_function_contents(&ctx->key_funcs);
  }
  return returnval;
}

idl_retcode_t process_typedef_instance_impl(context_t* ctx, const char* accessor, idl_type_spec_t* spec, bool is_key)
{
  char* ns = NULL;  //namespace in which the typedef is declared
  idl_retcode_t ret = IDL_RETCODE_OK;
  resolve_namespace(spec, &ns);
  if (!ns)
  {
    ret = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }
  const char* tdname = idl_identifier((idl_declarator_t*)spec);  //name of the typedef
  if (!tdname)
  {
    ret = IDL_RETCODE_SEMANTIC_ERROR;
    goto fail2;
  }

  format_write_stream(1, ctx, false, typedef_write_call, ns, tdname, accessor);
  format_write_size_stream(1, ctx, false, typedef_write_size_call, ns, tdname, accessor);
  format_read_stream(1, ctx, false, typedef_read_call, ns, tdname, accessor);
  if (ctx->in_union)
  {
    format_max_size_intermediate_stream(1, ctx, false, union_case_max_set "%stypedef_max_size_%s(%s, case_max);\n", ns, tdname, accessor);
  }
  else
  {
    format_max_size_intermediate_stream(1, ctx, false, typedef_max_size_call, ns, tdname, accessor);
  }

  if (is_key)
  {
    format_key_write_stream(1, ctx, typedef_key_write_call, ns, tdname, accessor);
    format_key_read_stream(1, ctx, typedef_key_read_call, ns, tdname, accessor);
    format_key_size_stream(1, ctx, typedef_key_size_call, ns, tdname, accessor);
    if (ctx->in_union)
    {
      format_key_max_size_intermediate_stream(1, ctx, union_case_max_set "%stypedef_key_max_size_%s(%s, case_max);\n", ns, tdname, accessor);
    }
    else
    {
      format_key_max_size_intermediate_stream(1, ctx, typedef_key_max_size_call, ns, tdname, accessor);
    }
  }

  reset_alignment(ctx, is_key);

fail2:
  free(ns);
fail1:
  return ret;
}

idl_retcode_t process_case_label(const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  context_t* ctx = *((context_t**)user_data);
  idl_const_expr_t* ce = ((idl_case_label_t*)node)->const_expr;
  if (ce)
  {
    char* buffer = NULL;
    idl_constval_t* cv = (idl_constval_t*)ce;
    switch (idl_mask(ce) % (IDL_BASE_TYPE * 2))
    {
    case IDL_INT8:
      if (idl_asprintf(&buffer, "%" PRId8, cv->value.int8) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_OCTET:
    case IDL_UINT8:
      if (idl_asprintf(&buffer, "%" PRIu8, cv->value.uint8) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_SHORT:
    case IDL_INT16:
      if (idl_asprintf(&buffer, "%" PRId16, cv->value.int16) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_USHORT:
    case IDL_UINT16:
      if (idl_asprintf(&buffer, "%" PRIu16, cv->value.uint16) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_LONG:
    case IDL_INT32:
      if (idl_asprintf(&buffer, "%" PRId32, cv->value.int32) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_ULONG:
    case IDL_UINT32:
      if (idl_asprintf(&buffer, "%" PRIu32, cv->value.uint32) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_LLONG:
    case IDL_INT64:
      if (idl_asprintf(&buffer, "%" PRId64, cv->value.int64) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_ULLONG:
    case IDL_UINT64:
      if (idl_asprintf(&buffer, "%" PRIu64, cv->value.uint64) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_BOOL:
      if ((buffer = idl_strdup(cv->value.bln ? "true" : "false")) == NULL)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_CHAR:
      if (idl_asprintf(&buffer, "\'%c\'", cv->value.chr) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    default:
      assert(0);
    }

    if (buffer)
    {
      format_write_stream(1, ctx, false, union_case, buffer);
      format_write_size_stream(1, ctx, false, union_case, buffer);
      format_read_stream(1, ctx, false, union_case, buffer);
      free(buffer);
    }

    return IDL_RETCODE_OK;
  }
  else
  {
    return add_default_case(ctx);
  }
}

idl_retcode_t add_default_case(context_t* ctx)
{
  format_write_stream(1, ctx, false, default_case);
  format_write_size_stream(1, ctx, false, default_case);
  format_read_stream(1, ctx, false, default_case);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_string_impl(context_t* ctx, const char* accessor, idl_string_t* spec, bool is_key)
{
  //sequence entries
  process_sequence_entries(ctx, accessor, true, is_key);
  uint64_t bound = spec->maximum;
  if (bound)
  {
    //add boundary checking function
    format_write_stream(1, ctx, is_key, seq_length_exception, ctx->depth, bound+1, accessor, bound);
    //adding one to the length check here, since the sequence_entries CDR field for a string includes the terminating NULL
  }
  else
  {
    ctx->max_size_unlimited = true;
    if (is_key)
      ctx->key_max_size_unlimited = true;
  }

  //data
  format_write_stream(1, ctx, is_key, string_write, accessor, ctx->depth, 1, accessor);
  format_write_stream(1, ctx, is_key, seq_inc_1 entries_of_sequence_comment, ctx->depth);
  format_write_size_stream(1, ctx, is_key, seq_inc_1 entries_of_sequence_comment, ctx->depth);

  format_read_stream(1, ctx, is_key, read_str, accessor, ctx->depth, accessor);
  format_read_stream(1, ctx, is_key, seq_inc_1 entries_of_sequence_comment, ctx->depth);

  if (bound)
  {
    if (ctx->in_union)
    {
      format_max_size_intermediate_stream(1, ctx, is_key, union_case_max_incr "%d;\n", bound + 1);
    }
    else
    {
      format_max_size_intermediate_stream(1, ctx, is_key, max_size_incr_checked entries_of_sequence_comment, bound + 1);
    }
  }

  reset_alignment(ctx, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_sequence_impl(context_t* ctx, const char* accessor, idl_sequence_t* spec, bool is_key)
{
  idl_type_spec_t* ispec = spec->type_spec;
  if (idl_is_typedef(ispec))
  {
    idl_type_spec_t* temp = resolve_typedef(ispec);
    if (idl_is_base_type(temp))
      ispec = temp;
  }

  //sequence entries
  process_sequence_entries(ctx, accessor, false, is_key);
  uint64_t bound = spec->maximum;
  if (bound)
  {
    //add boundary checking function
    format_write_stream(1, ctx, is_key, seq_length_exception, ctx->depth, bound, accessor, bound);
  }
  else
  {
    ctx->max_size_unlimited = true;
    if (is_key)
      ctx->key_max_size_unlimited = true;
  }

  if (idl_is_base_type(ispec))
  {
    //base types are treated differently from more complex template types
    int bytewidth = determine_byte_width(ispec);
    assert(bytewidth > 0);
    if (bytewidth > 4)
      check_alignment(ctx, bytewidth, is_key);
    char* cast = determine_cast(ispec);
    assert(cast);

    if (0 == strcmp(cast,bool_cast))  //necessary because IDL_BOOL has overlap with IDL_ULONG
    {
      format_write_stream(1, ctx, is_key, bool_write_seq, ctx->depth, accessor, accessor);
      format_read_stream(1, ctx, is_key, seq_read_resize, accessor, ctx->depth);
      format_read_stream(1, ctx, is_key, bool_read_seq, ctx->depth, accessor, accessor);
    }
    else
    {
      format_write_stream(1, ctx, is_key, primitive_write_seq_checked, accessor, accessor, ctx->depth, bytewidth, accessor);
      format_write_stream(1, ctx, is_key, seq_incr entries_of_sequence_comment, ctx->depth, bytewidth);
      format_read_stream(1, ctx, is_key, primitive_read_seq, accessor, cast, cast, ctx->depth, accessor);
      format_read_stream(1, ctx, is_key, seq_incr entries_of_sequence_comment, ctx->depth, bytewidth);
    }
    format_write_size_stream(1, ctx, is_key, seq_incr entries_of_sequence_comment, ctx->depth, bytewidth);
    if (bound)
    {
      format_max_size_intermediate_stream(1, ctx, is_key, max_size_incr_checked_multiple, (int)bound * bytewidth);
    }

    free(cast);
  }
  else
  {
    format_read_stream(1, ctx, is_key, seq_read_resize, accessor, ctx->depth);

    //loop over
    format_write_stream(1, ctx, is_key, sequence_iterate, ctx->depth + 1, ctx->depth + 1, ctx->depth, ctx->depth + 1);
    format_write_size_stream(1, ctx, is_key, sequence_iterate, ctx->depth + 1, ctx->depth + 1, ctx->depth, ctx->depth + 1);
    format_read_stream(1, ctx, is_key, sequence_iterate, ctx->depth + 1, ctx->depth + 1, ctx->depth, ctx->depth + 1);
    format_max_size_intermediate_stream(1, ctx, is_key, bound_iterate, ctx->depth + 1, ctx->depth + 1, bound, ctx->depth + 1);
    ctx->depth++;

    bool locals[4];
    store_locals(ctx, locals);

    char* entryaccess = NULL;
    idl_asprintf(&entryaccess, array_accessor, accessor, ctx->depth);
    assert(entryaccess);

    if (idl_is_string(ispec))
    {
      process_string_impl(ctx, entryaccess, (idl_string_t*)ispec, is_key);
    }
    else if (idl_is_sequence(ispec))
    {
      process_sequence_impl(ctx, entryaccess, (idl_sequence_t*)ispec, is_key);
    }
    else if (idl_is_struct(ispec) || idl_is_union(ispec))
    {
      process_constructed_type_impl(ctx, entryaccess, is_key, !has_keys((idl_type_spec_t*)spec));
    }
    else if (idl_is_typedef(ispec))
    {
      process_typedef_instance_impl(ctx, entryaccess, ispec, is_key);
    }
    free(entryaccess);

    //close loop
    format_write_size_stream(1, ctx, is_key, close_block);
    format_write_stream(1, ctx, is_key, close_block);
    format_read_stream(1, ctx, is_key, close_block);
    format_max_size_intermediate_stream(1, ctx, is_key, close_block);

    ctx->depth--;
    load_locals(ctx, locals);
  }

  if (bound)
  {
    format_read_stream(1, ctx, is_key, seq_read_past_bound_resize, ctx->depth, bound, accessor, bound);
  }

  reset_alignment(ctx, is_key);

  return IDL_RETCODE_OK;
}

void idl_streamers_generate(const idl_pstate_t* tree, idl_streamer_output_t* str)
{
  context_t* ctx = create_context("");

  format_impl_stream(0, ctx, "#include \"org/eclipse/cyclonedds/topic/hash.hpp\"\n\n");

  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_MODULE | IDL_TYPEDEF | IDL_STRUCT | IDL_UNION | IDL_MEMBER | IDL_CASE | IDL_CASE_LABEL;
  visitor.accept[IDL_ACCEPT_TYPEDEF] = &process_typedef_definition;
  visitor.accept[IDL_ACCEPT_STRUCT] = &process_struct_definition;
  visitor.accept[IDL_ACCEPT_UNION] = &process_union_definition;
  visitor.accept[IDL_ACCEPT_MODULE] = &process_module_definition;
  visitor.accept[IDL_ACCEPT_MEMBER] = &process_member;
  visitor.accept[IDL_ACCEPT_CASE] = &process_case;
  visitor.accept[IDL_ACCEPT_CASE_LABEL] = &process_case_label;
  if (tree->sources)
    visitor.glob = tree->sources->path->name;
  idl_visit(tree, tree->root, &visitor, &ctx);

  close_context(ctx, str);
}
