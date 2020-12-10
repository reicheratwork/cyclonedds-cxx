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
#include "idl/tree.h"
#include "idl/scope.h"
#include "idl/string.h"

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

#define format_key_write_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_write_stream, __VA_ARGS__);

#define format_key_read_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_read_stream, __VA_ARGS__);

#define format_max_size_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->max_size_stream, __VA_ARGS__); \
if (key) { format_key_max_size_stream(indent, ctx, __VA_ARGS__); }

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
#define union_switch "switch (obj._d()) {\n"
#define union_case "  case %s:\n"
#define default_case "  default:\n"
#define union_case_ending "break;\n"
#define union_clear_func "obj.clear();\n"
#define open_block "{\n"
#define close_block "}\n"
#define close_function close_block "\n"
#define position_return "return stream.position() - pos;\n"
#define const_ref_cast "dynamic_cast<const %s%s&>(obj)"
#define ref_cast "dynamic_cast<%s%s&>(obj)"
#define member_access "obj.%s()"
#define ignore_obj "(void)obj;\n"
#define position_store "size_t pos = stream.position();\n"

//c++ class member function defines/calls
#define write_def "template<typename T> size_t write(const %s%s &obj, T &stream) {\n"
#define write_call "write(%s,stream);\n"
#define write_size_def "template<typename T> size_t write_size(const %s%s &obj, T &stream) {\n"
#define write_size_call "write_size(%s,stream);\n"
#define write_size_max_def "template<typename T> size_t write_size_max(const %s%s &obj, T &stream) {\n"
#define write_size_max_call "write_size_max(%s,stream);\n"
#define read_def "template<typename T> size_t read(%s%s &obj, T &stream) {\n"
#define read_call "read(%s,stream);\n"
#define key_write_def "template<typename T> size_t key_write(const %s%s &obj, T &stream) {\n"
#define key_write_call "key_write(%s,stream);\n"
#define key_write_size_def "template<typename T> size_t key_write_size(const %s%s &obj, T &stream) {\n"
#define key_write_size_call "key_write_size(%s,stream);\n"
#define key_write_size_max_def "template<typename T> size_t key_write_size_max(const %s%s &obj, T &stream) {\n"
#define key_write_size_max_call "key_write_size_max(%s,stream);\n"
#define key_read_def "template<typename T> size_t key_read(%s%s &obj, T &stream) {\n"
#define key_read_call "key_read(%s,stream);\n"
#define key_calc_define "template<typename T> bool key_generate(const %s%s &obj, T &stream, ddsi_keyhash_t &hash) {\n"

//cdr stream function calls
#define read_primitive_call "stream.read_primitive(%s);\n"
#define write_primitive_call "stream.write_primitive(%s);\n"
#define incr_primitive_call "stream.incr_primitive(%s);\n"
#define read_string_call "stream.read_string(%s,%"PRIu64");\n"
#define write_string_call "stream.write_string(%s,%"PRIu64");\n"
#define incr_string_call "stream.incr_string(%s,%"PRIu64");\n"
#define max_size_string_call "stream.max_size_string(%s,%"PRIu64");\n"
#define read_array_call "stream.read_array(%s);\n"
#define write_array_call "stream.write_array(%s);\n"
#define incr_array_call "stream.incr_array(%s);\n"
#define max_size_array_call "stream.max_size_array(%s);\n"
#define read_sequence_call "stream.read_sequence(%s,%"PRIu64");\n"
#define write_sequence_call "stream.write_sequence(%s,%"PRIu64");\n"
#define incr_sequence_call "stream.incr_sequence(%s,%"PRIu64");\n"
#define max_size_sequence_call "stream.max_size_sequence(%s,%"PRIu64");\n"
#define read_enum_call "stream.read_enum(%s);\n"
#define write_enum_call "stream.write_enum(%s);\n"
#define incr_enum_call "stream.incr_enum(%s);\n"
#define max_size_enum_call "stream.max_size_enum(%s);\n"

struct idl_streamer_output
{
  size_t indent;
  idl_ostream_t* impl_stream;
  idl_ostream_t* head_stream;
};

typedef struct context context_t;

struct context
{
  idl_streamer_output_t* str;
  char* context;
  idl_ostream_t* write_size_stream;
  idl_ostream_t* write_stream;
  idl_ostream_t* read_stream;
  idl_ostream_t* key_size_stream;
  idl_ostream_t* max_size_stream;
  idl_ostream_t* key_max_size_stream;
  idl_ostream_t* key_write_stream;
  idl_ostream_t* key_read_stream;
  size_t depth;
  context_t* parent;
  const char* parsed_file;
};

static uint64_t array_entries(idl_declarator_t* decl);
static idl_retcode_t add_default_case(context_t* ctx);
static idl_retcode_t process_node(context_t* ctx, idl_node_t* node);
static idl_retcode_t process_instance(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_constructed_type_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_constructed_type_impl(context_t* ctx, const char* accessor, bool is_key, bool key_is_all_members);
static idl_retcode_t process_base_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_base_impl(context_t* ctx, const char *accessor, uint64_t entries, bool is_key);
static idl_retcode_t process_enum_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key);
static idl_retcode_t process_enum_impl(context_t* ctx, const char *accessor, uint64_t entries, bool is_key);
static idl_retcode_t process_string_decl(context_t* ctx, idl_declarator_t* decl, idl_string_t* spec, bool is_key);
static idl_retcode_t process_string_impl(context_t* ctx, const char *accessor, idl_string_t* spec, bool is_key);
static idl_retcode_t process_sequence_decl(context_t* ctx, idl_declarator_t* decl, idl_sequence_t* spec, bool is_key);
static idl_retcode_t process_sequence_impl(context_t* ctx, const char* accessor, idl_sequence_t* spec, bool is_key);
static idl_type_spec_t* resolve_typedef(idl_type_spec_t* def);
static idl_retcode_t process_member(context_t* ctx, idl_member_t* mem);
static idl_retcode_t process_module(context_t* ctx, idl_module_t* module);
static idl_retcode_t process_constructed(context_t* ctx, idl_node_t* node);
static idl_retcode_t process_case(context_t* ctx, idl_case_t* _case);
static idl_retcode_t process_case_label(context_t* ctx, idl_case_label_t* label);
static idl_retcode_t write_instance_funcs(context_t* ctx, const char* write_accessor, const char* read_accessor, bool is_key, bool key_is_all_members);
static context_t* create_context(const char* name);
static context_t* child_context(context_t* ctx, const char* name);
static void close_context(context_t* ctx, idl_streamer_output_t* str);
static void resolve_namespace(idl_node_t* node, char** up);
static char* generate_accessor(idl_declarator_t* decl);
static bool has_keys(idl_type_spec_t* spec);

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
  if (NULL != ptr)
  {
    ptr->str = create_idl_streamer_output();
    assert(ptr->str);
    ptr->context = idl_strdup(name);
    ptr->write_size_stream = create_idl_ostream(NULL);
    ptr->write_stream = create_idl_ostream(NULL);
    ptr->read_stream = create_idl_ostream(NULL);
    ptr->key_size_stream = create_idl_ostream(NULL);
    ptr->max_size_stream = create_idl_ostream(NULL);
    ptr->key_max_size_stream = create_idl_ostream(NULL);
    ptr->key_write_stream = create_idl_ostream(NULL);
    ptr->key_read_stream = create_idl_ostream(NULL);
  }
  return ptr;
}

context_t* child_context(context_t* ctx, const char* name)
{
  context_t* ptr = create_context(name);
  assert(ptr);

  ptr->parent = ctx;
  ptr->depth = ctx->depth;
  ptr->parsed_file = ctx->parsed_file;

  return ptr;
}

void close_context(context_t* ctx, idl_streamer_output_t* str)
{
  assert(ctx);

  //bundle contents
  transfer_ostream_buffer(ctx->write_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->write_size_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->max_size_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->key_size_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->key_max_size_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->key_write_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->key_read_stream, ctx->str->head_stream);
  transfer_ostream_buffer(ctx->read_stream, ctx->str->head_stream);

  if (get_ostream_buffer_position(ctx->str->head_stream) != 0)
  {
    if (ctx->parent)
    {
      //there is a parent context (so all statements are made inside a namespace)
      //move the contents to the parent
      format_impl_stream(1, ctx->parent, namespace_declaration, ctx->context);
      format_impl_stream(1, ctx->parent, open_block "\n");
      transfer_ostream_buffer(ctx->str->head_stream, ctx->parent->str->head_stream);
      format_impl_stream(1, ctx->parent, namespace_closure, ctx->context);
    }
    else if (str)
    {
      //no parent context (so this is the root namespace)
      //move the contents to the "exit" stream
      transfer_ostream_buffer(ctx->str->head_stream, str->head_stream);
    }
  }

  //cleanup old context streams
  destruct_idl_ostream(ctx->write_stream);
  destruct_idl_ostream(ctx->write_size_stream);
  destruct_idl_ostream(ctx->key_size_stream);
  destruct_idl_ostream(ctx->max_size_stream);
  destruct_idl_ostream(ctx->key_max_size_stream);
  destruct_idl_ostream(ctx->key_write_stream);
  destruct_idl_ostream(ctx->key_read_stream);
  destruct_idl_ostream(ctx->read_stream);

  destruct_idl_streamer_output(ctx->str);
  free(ctx->context);
  free(ctx);
}

void resolve_namespace(idl_node_t* node, char** up)
{
  if (!node)
    return;

  if (!*up)
    *up = idl_strdup("");

  if (idl_is_module(node))
  {
    idl_module_t* mod = (idl_module_t*)node;
    char* cppname = get_cpp11_name(idl_identifier(mod));
    assert(cppname);
    char *temp = NULL;
    idl_asprintf(&temp, "%s::%s", cppname, *up);
    free(*up);
    *up = temp;
    free(cppname);
  }

  resolve_namespace(node->parent, up);
}

char* generate_accessor(idl_declarator_t* decl)
{
  assert(decl);
  char* accessor = NULL;
  char* cpp11name = get_cpp11_name(idl_identifier(decl));
  idl_asprintf(&accessor, member_access, cpp11name);
  free(cpp11name);
  assert(accessor);
  return accessor;
}

bool has_keys(idl_type_spec_t* spec)
{
  //is struct
  if (idl_is_struct(spec))
  {
    idl_member_t* mem = ((idl_struct_t*)spec)->members;
    while (mem)
    {
      if (idl_is_masked(mem, IDL_KEY)) return true;
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

uint64_t array_entries(idl_declarator_t* decl)
{
  if (NULL == decl)
    return 0;

  idl_const_expr_t* ce = decl->const_expr;
  uint64_t entries = 0;
  while (ce)
  {
    if ((ce->mask & IDL_CONST) == IDL_CONST)
    {
      idl_constval_t* var = (idl_constval_t*)ce;
      idl_mask_t mask = var->node.mask;
      if ((mask & IDL_UINT8) == IDL_UINT8)
      {
        if (entries)
          entries *= var->value.oct;
        else
          entries = var->value.oct;
      }
      else if ((mask & IDL_UINT32) == IDL_UINT32)
      {
        if (entries)
          entries *= var->value.ulng;
        else
          entries = var->value.ulng;
      }
      else if ((mask & IDL_UINT64) == IDL_UINT64)
      {
        if (entries)
          entries *= var->value.ullng;
        else
          entries = var->value.ullng;
      }
    }

    ce = ce->next;
  }
  return entries;
}

idl_type_spec_t* resolve_typedef(idl_type_spec_t* spec)
{
  while (NULL != spec &&
    idl_is_typedef(spec))
    spec = ((idl_typedef_t*)spec)->type_spec;

  return spec;
}

idl_retcode_t process_node(context_t* ctx, idl_node_t* node)
{

  if (idl_is_module(node))
  {
    process_module(ctx, (idl_module_t*)node);  //module entries are not filtered on which file they are defined in, since their first occurance may be in another (previous) file
  }
  else if (ctx->parsed_file == NULL ||
           0 == strcmp(node->location.first.file, ctx->parsed_file))
  {
    if (idl_is_struct(node) || idl_is_union(node))
      process_constructed(ctx, node);
  }

  if (node->next)
    process_node(ctx, node->next);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_member(context_t* ctx, idl_member_t* mem)
{
  assert(ctx);
  assert(mem);

  bool is_key = idl_is_masked(mem, IDL_KEY);
  process_instance(ctx, mem->declarators, mem->type_spec, is_key);

  if (mem->node.next)
    process_member(ctx, (idl_member_t*)(mem->node.next));

  return IDL_RETCODE_OK;
}

idl_retcode_t process_instance(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key)
{
  if (idl_is_base_type(spec))
  {
    return process_base_decl(ctx, decl, spec, is_key);
  }
  else if (idl_is_enum(spec))
  {
    return process_enum_decl(ctx, decl, spec, is_key);
  }
  else if (idl_is_struct(spec) || idl_is_union(spec))
  {
    return process_constructed_type_decl(ctx, decl, spec, is_key);
  }
  else if (idl_is_string(spec))
  {
    return process_string_decl(ctx, decl, (idl_string_t*)spec, is_key);
  }
  else if (idl_is_sequence(spec))
  {
    return process_sequence_decl(ctx, decl, (idl_sequence_t*)spec, is_key);
  } else {
    assert(idl_is_typedef(spec));

    idl_type_spec_t* temp = resolve_typedef(spec);

    return process_instance(ctx, decl, temp, is_key);
  }
}

idl_retcode_t process_constructed_type_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* spec, bool is_key)
{
  assert(ctx);

  uint64_t entries = array_entries(decl);
  char* accessor = generate_accessor(decl);

  if (entries)
  {
    //array of constructed types
    format_write_stream(1, ctx, is_key, write_array_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_array_call, accessor);
    format_read_stream(1, ctx, is_key, read_array_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_array_call, accessor);
  }
  else
  {
    //simple constructed type
    process_constructed_type_impl(ctx, accessor, is_key, !has_keys(spec));
  }

  free(accessor);

  if (NULL != decl &&
      ((idl_node_t*)decl)->next)
    process_constructed_type_decl(ctx, (idl_declarator_t*)((idl_node_t*)decl)->next, spec, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_constructed_type_impl(context_t* ctx, const char* accessor, bool is_key, bool key_is_all_members)
{
  return write_instance_funcs(ctx, accessor, accessor, is_key, key_is_all_members);
}

idl_retcode_t write_instance_funcs(context_t* ctx, const char* write_accessor, const char* read_accessor, bool is_key, bool key_is_all_members)
{

  format_write_stream(1, ctx, false , write_call, write_accessor);
  format_write_size_stream(1, ctx, false, write_size_call, write_accessor);
  format_read_stream(1, ctx, false, read_call, read_accessor);
  format_max_size_stream(1, ctx, false, write_size_max_call, write_accessor);

  if (is_key)
  {
    if (!key_is_all_members)
    {
      format_key_write_stream(1, ctx, key_write_call, write_accessor);
      format_key_read_stream(1, ctx, key_read_call, read_accessor);
      format_key_size_stream(1, ctx, key_write_size_call, write_accessor);
      format_key_max_size_stream(1, ctx, key_write_size_max_call, write_accessor);
    }
    else
    {
      format_key_write_stream(1, ctx, write_call, write_accessor);
      format_key_read_stream(1, ctx, read_call, read_accessor);
      format_key_size_stream(1, ctx, write_size_call, write_accessor);
      format_key_max_size_stream(1, ctx, key_write_size_call, write_accessor);
    }
  }

  return IDL_RETCODE_OK;
}

idl_retcode_t process_module(context_t* ctx, idl_module_t* module)
{
  assert(ctx);
  assert(module);

  if (module->definitions)
  {
    char* cpp11name = get_cpp11_name(idl_identifier(module));
    assert(cpp11name);

    context_t* newctx = child_context(ctx, cpp11name);

    process_node(newctx, (idl_node_t*)module->definitions);

    close_context(newctx, NULL);

    free(cpp11name);
  }

  return IDL_RETCODE_OK;
}

idl_retcode_t process_constructed(context_t* ctx, idl_node_t* node)
{
  assert(ctx);
  assert(node);

  char* cpp11name = NULL;
  char* ns = NULL;
  idl_retcode_t returnval = IDL_RETCODE_OK;

  if (idl_is_struct(node) ||
      idl_is_union(node))
  {
    if (idl_is_struct(node))
      cpp11name = get_cpp11_name(idl_identifier((idl_struct_t*)node));
    else if (idl_is_union(node))
      cpp11name = get_cpp11_name(idl_identifier((idl_union_t*)node));
    assert(cpp11name);
    resolve_namespace(node, &ns);
    assert(ns);

    format_write_stream(1, ctx, false, write_def, ns, cpp11name);
    format_write_size_stream(1, ctx, false, write_size_def, ns, cpp11name);
    format_max_size_stream(1, ctx, false, write_size_max_def, ns, cpp11name);
    format_read_stream(1, ctx, false, read_def, ns, cpp11name);
    format_key_write_stream(1, ctx, key_write_def, ns, cpp11name);
    format_key_size_stream(1, ctx, key_write_size_def, ns, cpp11name);
    format_key_max_size_stream(1, ctx, key_write_size_max_def, ns, cpp11name);
    format_key_read_stream(1, ctx, key_read_def, ns, cpp11name);

    ctx->depth++;

    format_write_stream(1, ctx, true, ignore_obj);
    format_write_size_stream(1, ctx, true, ignore_obj);
    format_max_size_stream(1, ctx, true, ignore_obj);
    format_read_stream(1, ctx, true, ignore_obj);

    format_write_stream(1, ctx, true, position_store);
    format_write_size_stream(1, ctx, true, position_store);
    format_max_size_stream(1, ctx, false, position_store);
    if (!idl_is_union(node))
    {
      format_key_max_size_stream(1, ctx, position_store);
    }
    format_read_stream(1, ctx, true, position_store);

    if (idl_is_struct(node))
    {
      idl_struct_t* _struct = (idl_struct_t*)node;
      if (_struct->base_type)
      {
        char* base_cpp11name = get_cpp11_name(idl_identifier(_struct->base_type));
        char* base_ns = NULL;
        assert(base_cpp11name);
        resolve_namespace((idl_node_t*)_struct->base_type, &base_ns);
        assert(base_ns);
        char* write_accessor = NULL, *read_accessor = NULL;
        if (idl_asprintf(&write_accessor, const_ref_cast, base_ns, base_cpp11name) != -1 &&
            idl_asprintf(&read_accessor, ref_cast, base_ns, base_cpp11name) != -1)
          returnval = write_instance_funcs(ctx, write_accessor, read_accessor, true, false);
        else
          returnval = IDL_RETCODE_NO_MEMORY;

        if (write_accessor)
          free(write_accessor);
        if (read_accessor)
          free(read_accessor);

        free(base_cpp11name);
        free(base_ns);
      }

      if (_struct->members &&
          returnval == IDL_RETCODE_OK)
        returnval = process_member(ctx, _struct->members);

      //max size done here since its treatment is different for structs than for unions
      format_max_size_stream(1, ctx, true, position_return);
    }
    else if (idl_is_union(node))
    {
      idl_union_t* _union = (idl_union_t*)node;
      idl_switch_type_spec_t* st = _union->switch_type_spec;
      bool disc_is_key = idl_is_masked(st, IDL_KEY);

      format_read_stream(1, ctx, true, union_clear_func);
      process_base_impl(ctx, "_d()", 0, disc_is_key);
      format_write_size_stream(1, ctx, false, union_switch);
      format_write_stream(1, ctx, false, union_switch);
      format_read_stream(1, ctx, false, union_switch);

      format_max_size_stream(1, ctx, false, "size_t union_max = stream.position();\n");
      format_max_size_stream(1, ctx, false, "T stream_copy(stream);\n");  //store a copy of the streamer so that for each switch case can make its own copy with the same name

      if (_union->cases)
        process_case(ctx, _union->cases);

      format_write_stream(1, ctx, false, close_block);
      format_write_size_stream(1, ctx, false, close_block);
      format_read_stream(1, ctx, false, close_block);

      format_max_size_stream(1, ctx, false, "return union_max;\n");
    }

    format_write_stream(1, ctx, true, position_return);
    format_write_size_stream(1, ctx, true, position_return);
    format_read_stream(1, ctx, true, position_return);

    ctx->depth--;

    format_write_size_stream(1, ctx, true, close_function);
    format_write_stream(1, ctx, true, close_function);
    format_read_stream(1, ctx, true, close_function);
    format_max_size_stream(1, ctx, true, close_function);

    format_key_write_stream(1, ctx, key_calc_define, ns, cpp11name);
    format_key_write_stream(1, ctx, "  " ignore_obj);
    format_key_write_stream(1, ctx, "  T stream_copy(stream);\n");
    format_key_write_stream(1, ctx, "  stream_copy.set_buffer(NULL);\n");
    format_key_write_stream(1, ctx, "  size_t sz = key_write_size(obj,stream_copy);\n");
    format_key_write_stream(1, ctx, "  size_t padding = 16 - sz%%16;\n");
    format_key_write_stream(1, ctx, "  if (sz != 0 && padding == 16) padding = 0;\n");
    format_key_write_stream(1, ctx, "  std::vector<unsigned char> buffer(sz+padding);\n");
    format_key_write_stream(1, ctx, "  std::memset(buffer.data()+sz,0x0,padding);\n");
    format_key_write_stream(1, ctx, "  stream_copy.set_buffer(const_cast<unsigned char*>(buffer.data()));\n");
    format_key_write_stream(1, ctx, "  key_write(obj,stream_copy);\n");
    format_key_write_stream(1, ctx, "  static bool (*fptr)(const std::vector<unsigned char>&, ddsi_keyhash_t &) = NULL;\n");
    format_key_write_stream(1, ctx, "  if (fptr == NULL)\n");
    format_key_write_stream(1, ctx, "  {\n");
    format_key_write_stream(1, ctx, "    if (key_write_size_max(obj,stream_copy) <= 16)\n");
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
  }

  if (cpp11name)
    free(cpp11name);
  if (ns)
    free(ns);
  return returnval;
}

idl_retcode_t process_case(context_t* ctx, idl_case_t* _case)
{
  if (_case->case_labels)
    process_case_label(ctx, _case->case_labels);

  format_max_size_stream(1, ctx, false, open_block);
  ctx->depth++;

  format_write_stream(1, ctx, false, open_block);
  format_write_size_stream(1, ctx, false, open_block);
  format_read_stream(1, ctx, false, open_block);

  process_instance(ctx, _case->declarator, _case->type_spec, false);

  format_max_size_stream(1, ctx, false, "union_max = std::max(stream.position(),union_max);\n");  //take the largest of the case maximum and the stored maximum
  format_max_size_stream(1, ctx, false, "stream = stream_copy;\n");  //reset stream to go back to position/alignment before processing the current group of cases

  format_write_stream(1, ctx, false, close_block);
  format_write_stream(1, ctx, false, union_case_ending);
  format_write_size_stream(1, ctx, false, close_block);
  format_write_size_stream(1, ctx, false, union_case_ending);
  format_read_stream(1, ctx, false, close_block);
  format_read_stream(1, ctx, false, union_case_ending);

  ctx->depth--;
  format_max_size_stream(1, ctx, false, close_block);

  //go to next case
  if (_case->node.next)
    process_case(ctx, (idl_case_t*)_case->node.next);
  return IDL_RETCODE_OK;
}

idl_retcode_t process_case_label(context_t* ctx, idl_case_label_t* label)
{
  idl_const_expr_t* ce = label->const_expr;
  if (ce)
  {
    char* buffer = NULL;
    idl_constval_t* cv = (idl_constval_t*)ce;

    switch (ce->mask % (IDL_BASE_TYPE * 2))
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
    case IDL_INT16:
      if (idl_asprintf(&buffer, "%" PRId16, cv->value.int16) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_UINT16:
      if (idl_asprintf(&buffer, "%" PRIu16, cv->value.uint16) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_INT32:
      if (idl_asprintf(&buffer, "%" PRId32, cv->value.int32) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_UINT32:
      if (idl_asprintf(&buffer, "%" PRIu32, cv->value.uint32) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
    case IDL_INT64:
      if (idl_asprintf(&buffer, "%" PRId64, cv->value.int64) == -1)
        return IDL_RETCODE_NO_MEMORY;
      break;
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
  }
  else
  {
    add_default_case(ctx);
  }

  if (label->node.next)
    process_case_label(ctx, (idl_case_label_t*)label->node.next);

  return IDL_RETCODE_OK;
}

idl_retcode_t add_default_case(context_t* ctx)
{
  format_write_stream(1, ctx, false, default_case);
  format_write_size_stream(1, ctx, false, default_case);
  format_read_stream(1, ctx, false, default_case);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_base_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* tspec, bool is_key)
{
  assert(ctx);
  assert(tspec);

  char* accessor = generate_accessor(decl);
  process_base_impl(ctx, accessor, array_entries(decl), is_key);
  free(accessor);

  if (NULL != decl &&
      ((idl_node_t*)decl)->next)
    process_base_decl(ctx, (idl_declarator_t*)((idl_node_t*)decl)->next, tspec, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_base_impl(context_t* ctx, const char* accessor, uint64_t entries, bool is_key)
{
  if (entries) {
    //this is an array of primitives
    format_write_stream(1, ctx, is_key, write_array_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_array_call, accessor);
    format_read_stream(1, ctx, is_key, read_array_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_array_call, accessor);
  }
  else {
    //this is just a single primitive
    format_write_stream(1, ctx, is_key, write_primitive_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_primitive_call, accessor);
    format_read_stream(1, ctx, is_key, read_primitive_call, accessor);
    format_max_size_stream(1, ctx, is_key, incr_primitive_call, accessor);
  }

  return IDL_RETCODE_OK;
}

idl_retcode_t process_enum_decl(context_t* ctx, idl_declarator_t* decl, idl_type_spec_t* tspec, bool is_key)
{
  assert(ctx);
  assert(tspec);

  char* accessor = generate_accessor(decl);
  process_enum_impl(ctx, accessor, array_entries(decl), is_key);
  free(accessor);

  if (NULL != decl &&
      ((idl_node_t*)decl)->next)
    process_base_decl(ctx, (idl_declarator_t*)((idl_node_t*)decl)->next, tspec, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_enum_impl(context_t* ctx, const char* accessor, uint64_t entries, bool is_key)
{
  if (entries) {
    //this is an array of primitives
    format_write_stream(1, ctx, is_key, write_array_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_array_call, accessor);
    format_read_stream(1, ctx, is_key, read_array_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_array_call, accessor);
  }
  else {
    //this is just a single primitive
    format_write_stream(1, ctx, is_key, write_enum_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_enum_call, accessor);
    format_read_stream(1, ctx, is_key, read_enum_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_enum_call, accessor);
  }

  return IDL_RETCODE_OK;
}

idl_retcode_t process_string_decl(context_t* ctx, idl_declarator_t* decl, idl_string_t* spec, bool is_key)
{
  assert(ctx);
  assert(spec);

  uint64_t entries = array_entries(decl);
  char* accessor = generate_accessor(decl);

  if (entries)
  {
    //this is an array of strings
    format_write_stream(1, ctx, is_key, write_array_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_array_call, accessor);
    format_read_stream(1, ctx, is_key, read_array_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_array_call, accessor);
  }
  else
  {
    //this is just a string
    process_string_impl(ctx, accessor, spec, is_key);
  }

  free(accessor);

  if (NULL != decl &&
    ((idl_node_t*)decl)->next)
    process_string_decl(ctx, (idl_declarator_t*)((idl_node_t*)decl)->next, spec, is_key);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_string_impl(context_t* ctx, const char* accessor, idl_string_t* spec, bool is_key)
{
  uint64_t bound = spec->maximum;

  format_write_stream(1, ctx, is_key, write_string_call, accessor, bound);
  format_write_size_stream(1, ctx, is_key, incr_string_call, accessor, bound);
  format_read_stream(1, ctx, is_key, read_string_call, accessor, bound);
  format_max_size_stream(1, ctx, is_key, max_size_string_call, accessor, bound);

  return IDL_RETCODE_OK;
}

idl_retcode_t process_sequence_decl(context_t* ctx, idl_declarator_t* decl, idl_sequence_t* spec, bool is_key)
{
  assert(ctx);
  assert(spec);

  uint64_t entries = array_entries(decl);
  char* accessor = generate_accessor(decl);

  if (entries)
  {
    //this is an array of sequences
    format_write_stream(1, ctx, is_key, write_array_call, accessor);
    format_write_size_stream(1, ctx, is_key, incr_array_call, accessor);
    format_read_stream(1, ctx, is_key, read_array_call, accessor);
    format_max_size_stream(1, ctx, is_key, max_size_array_call, accessor);
  }
  else
  {
    //this just a sequence
    process_sequence_impl(ctx, accessor, spec, is_key);
  }

  free(accessor);


  if (NULL != decl &&
    ((idl_node_t*)decl)->next)
    process_sequence_decl(ctx, (idl_declarator_t*)((idl_node_t*)decl)->next, spec, is_key);

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

  //max sequence entries
  uint64_t bound = spec->maximum;

  format_write_stream(1, ctx, is_key, write_sequence_call, accessor, bound);
  format_write_size_stream(1, ctx, is_key, incr_sequence_call, accessor, bound);
  format_read_stream(1, ctx, is_key, read_sequence_call, accessor, bound);
  format_max_size_stream(1, ctx, is_key, max_size_sequence_call, accessor, bound);

  return IDL_RETCODE_OK;
}

void idl_streamers_generate(const idl_tree_t* tree, idl_streamer_output_t* str)
{
  context_t* ctx = create_context("");

  format_header_stream(0, ctx, "#include <algorithm>\n");
  format_header_stream(0, ctx, "#include <vector>\n");
  format_header_stream(0, ctx, "#include <cstring>\n");
  format_header_stream(0, ctx, "#include \"org/eclipse/cyclonedds/topic/hash.hpp\"\n\n");
  if (tree->files)
    ctx->parsed_file = tree->files->name;

  process_node(ctx, tree->root);

  close_context(ctx, str);
}
