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

static void format_ostream_indented(size_t depth, idl_ostream_t* ostr, const char *fmt, ...)
{
  if (depth > 0) format_ostream(ostr, "%*c", depth, ' ');
  va_list args;
  va_start(args, fmt);
  format_ostream_va_args(ostr, fmt, args);
  va_end(args);
}

#define format_key_move_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_move_stream, __VA_ARGS__);

#define format_key_max_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_max_stream, __VA_ARGS__);

#define format_key_write_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_write_stream, __VA_ARGS__);

#define format_key_read_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_read_stream, __VA_ARGS__);

#define format_key_stream(indent,ctx, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->key_stream, __VA_ARGS__);

#define format_max_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->max_stream, __VA_ARGS__); \
if (key) { format_key_max_stream(indent, ctx, __VA_ARGS__); }

#define format_write_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->write_stream, __VA_ARGS__); \
if (key) {format_key_write_stream(indent,ctx, __VA_ARGS__);}

#define format_move_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->move_stream, __VA_ARGS__); \
if (key) {format_key_move_stream(indent,ctx, __VA_ARGS__);}

#define format_read_stream(indent,ctx,key, ...) \
format_ostream_indented(indent ? ctx->depth*2 : 0, ctx->read_stream, __VA_ARGS__); \
if (key) {format_key_read_stream(indent,ctx, __VA_ARGS__);}

#define union_switch "switch (%s)\n"
#define union_discriminator_accessor "_d()"
#define union_temp_instance "obj"
#define union_temp_discriminator "_disc_temp"
#define union_case "case %s:\n"
#define default_case "default:\n"
#define union_case_ending "break;\n"
#define open_block "{\n"
#define close_block "}\n"
#define close_function close_block "\n"
#define instance_name "instance"
#define const_ref_cast "dynamic_cast<const %s&>(" instance_name ")"
#define ref_cast "dynamic_cast<%s&>(" instance_name ")"
#define member_access "%s.%s()"
#define ignore_str "(void)str;\n"
#define ignore_instance "(void)" instance_name ";\n"
#define write_instance "write(str, %s);\n"
#define read_instance "read(str, %s);\n"
#define move_instance "move(str, %s);\n"
#define max_instance "max(str, %s);\n"
#define key_write_instance "key_write(str, %s);\n"
#define key_read_instance "key_read(str, %s);\n"
#define key_move_instance "key_move(str, %s);\n"
#define key_max_instance "key_max(str, %s);\n"
#define template_decl "template<typename T>\n"

struct idl_streamer_output
{
  size_t indent;
  idl_ostream_t* impl_stream;
  idl_ostream_t* head_stream;
};

idl_streamer_output_t*
create_idl_streamer_output()
{
  idl_streamer_output_t* ptr = calloc(sizeof(idl_streamer_output_t), 1);
  if (NULL != ptr)
  {
    ptr->impl_stream = create_idl_ostream(NULL);
    ptr->head_stream = create_idl_ostream(NULL);
  }
  return ptr;
}

void
destruct_idl_streamer_output(idl_streamer_output_t* str)
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

idl_ostream_t*
get_idl_streamer_impl_buf(const idl_streamer_output_t* str)
{
  return str->impl_stream;
}

idl_ostream_t*
get_idl_streamer_head_buf(const idl_streamer_output_t* str)
{
  return str->head_stream;
}

typedef struct context context_t;

struct context
{
  idl_ostream_t* write_stream;
  idl_ostream_t* read_stream;
  idl_ostream_t* move_stream;
  idl_ostream_t* max_stream;
  idl_ostream_t* key_write_stream;
  idl_ostream_t* key_read_stream;
  idl_ostream_t* key_move_stream;
  idl_ostream_t* key_max_stream;
  idl_ostream_t* key_stream;
  size_t key_entries;
  size_t depth;
};

static context_t*
create_context()
{
  context_t* ptr = calloc(sizeof(context_t),1);
  if (NULL == ptr)
    goto fail0;

  if (NULL == (ptr->write_stream = create_idl_ostream(NULL)))
    goto fail3;
  if (NULL == (ptr->read_stream = create_idl_ostream(NULL)))
    goto fail4;
  if (NULL == (ptr->move_stream = create_idl_ostream(NULL)))
    goto fail5;
  if (NULL == (ptr->max_stream= create_idl_ostream(NULL)))
    goto fail6;
  if (NULL == (ptr->key_write_stream = create_idl_ostream(NULL)))
    goto fail7;
  if (NULL == (ptr->key_read_stream = create_idl_ostream(NULL)))
    goto fail8;
  if (NULL == (ptr->key_move_stream = create_idl_ostream(NULL)))
    goto fail9;
  if (NULL == (ptr->key_max_stream = create_idl_ostream(NULL)))
    goto fail10;
  if (NULL == (ptr->key_stream = create_idl_ostream(NULL)))
    goto fail11;
  goto success;

fail11:
  destruct_idl_ostream(ptr->key_max_stream);
fail10:
  destruct_idl_ostream(ptr->key_move_stream);
fail9:
  destruct_idl_ostream(ptr->key_read_stream);
fail8:
  destruct_idl_ostream(ptr->key_write_stream);
fail7:
  destruct_idl_ostream(ptr->max_stream);
fail6:
  destruct_idl_ostream(ptr->move_stream);
fail5:
  destruct_idl_ostream(ptr->read_stream);
fail4:
  destruct_idl_ostream(ptr->write_stream);
fail3:
  free(ptr);
success:
fail0:
  return ptr;
}

static void
close_context(context_t* ctx,
  idl_streamer_output_t* str)
{
  transfer_ostream_buffer(ctx->write_stream, str->head_stream);
  transfer_ostream_buffer(ctx->read_stream, str->head_stream);
  transfer_ostream_buffer(ctx->move_stream, str->head_stream);
  transfer_ostream_buffer(ctx->max_stream, str->head_stream);
  transfer_ostream_buffer(ctx->key_write_stream, str->head_stream);
  transfer_ostream_buffer(ctx->key_read_stream, str->head_stream);
  transfer_ostream_buffer(ctx->key_move_stream, str->head_stream);
  transfer_ostream_buffer(ctx->key_max_stream, str->head_stream);
  transfer_ostream_buffer(ctx->key_stream, str->head_stream);

  destruct_idl_ostream(ctx->write_stream);
  destruct_idl_ostream(ctx->read_stream);
  destruct_idl_ostream(ctx->move_stream);
  destruct_idl_ostream(ctx->max_stream);
  destruct_idl_ostream(ctx->key_write_stream);
  destruct_idl_ostream(ctx->key_read_stream);
  destruct_idl_ostream(ctx->key_move_stream);
  destruct_idl_ostream(ctx->key_max_stream);
  destruct_idl_ostream(ctx->key_stream);

  free(ctx);
}

static char*
generate_accessor(idl_declarator_t* decl)
{
  char* accessor = NULL;
  if (decl)
  {
    char* cpp11name = get_cpp11_name(idl_identifier(decl));
    if (!cpp11name)
      return NULL;

    idl_asprintf(&accessor, member_access, instance_name, cpp11name);
    free(cpp11name);
  }
  else
  {
    accessor = idl_strdup(union_temp_instance);
  }
  return accessor;
}

static bool
has_keys(idl_type_spec_t* spec)
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
    return ((idl_union_t*)spec)->switch_type_spec->key;
  }
  return false;
}

static idl_retcode_t
process_instance(context_t* ctx,
  idl_declarator_t* decl,
  idl_type_spec_t* spec,
  bool is_key)
{
  idl_retcode_t returnval = IDL_RETCODE_OK;
  char* accessor = generate_accessor(decl);
  if (!accessor)
  {
    returnval = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }

  if (is_key)
    ctx->key_entries++;

  //write functions for accessor
  if ((idl_is_struct(spec) || idl_is_union(spec)) && has_keys(spec))
  {
    format_write_stream(1, ctx, false, write_instance, accessor);
    format_read_stream(1, ctx, false, read_instance, accessor);
    format_move_stream(1, ctx, false, move_instance, accessor);
    format_max_stream(1, ctx, false, max_instance, accessor);
    if (is_key)
    {
      format_key_write_stream(1, ctx, key_write_instance, accessor);
      format_key_read_stream(1, ctx, key_read_instance, accessor);
      format_key_move_stream(1, ctx, key_move_instance, accessor);
      format_key_max_stream(1, ctx, key_max_instance, accessor);
    }
  }
  else
  {
    format_write_stream(1, ctx, is_key, write_instance, accessor);
    format_read_stream(1, ctx, is_key, read_instance, accessor);
    format_move_stream(1, ctx, is_key, move_instance, accessor);
    format_max_stream(1, ctx, is_key, max_instance, accessor);
  }

  idl_declarator_t* nextdecl = decl ? (idl_declarator_t*)(decl->node.next) : NULL;
  if (nextdecl)
    returnval = process_instance(ctx, nextdecl, spec, is_key);

  free(accessor);
fail1:
  return returnval;
}

static idl_retcode_t
process_member(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  context_t* ctx = (context_t*)user_data;
  idl_member_t* mem = (idl_member_t*)node;
  return process_instance(ctx, mem->declarators, mem->type_spec, mem->key && pstate->version == IDL4);
}

static void
print_constructed_type_close(context_t* ctx)
{
  if (ctx->key_entries == 0)
  {
    format_key_write_stream(1, ctx, ignore_str);
    format_key_write_stream(1, ctx, ignore_instance);

    format_key_read_stream(1, ctx, ignore_str);
    format_key_read_stream(1, ctx, ignore_instance);

    format_key_move_stream(1, ctx, ignore_str);
    format_key_move_stream(1, ctx, ignore_instance);

    format_key_max_stream(1, ctx, ignore_str);
    format_key_max_stream(1, ctx, ignore_instance);
  }
  ctx->key_entries = 0;

  ctx->depth--;

  format_write_stream(1, ctx, true, close_function);
  format_read_stream(1, ctx, true, close_function);
  format_move_stream(1, ctx, true, close_function);
  format_max_stream(1, ctx, true, close_function);
}

static void
print_constructed_type_open(context_t* ctx,
  const idl_node_t* node)
{
  char* struct_name = get_cpp11_fully_scoped_name(node);

  format_write_stream(1, ctx, true, template_decl);
  format_write_stream(1, ctx, false, "void write(T& str, const %s& " instance_name ")\n", struct_name);
  format_key_write_stream(1, ctx, "void key_write(T& str, const %s& " instance_name ")\n", struct_name);
  format_write_stream(1, ctx, true, open_block);

  format_read_stream(1, ctx, true, template_decl);
  format_read_stream(1, ctx, false, "void read(T& str, %s& " instance_name ")\n", struct_name);
  format_key_read_stream(1, ctx, "void key_read(T& str, %s& " instance_name ")\n", struct_name);
  format_read_stream(1, ctx, true, open_block);

  format_move_stream(1, ctx, true, template_decl);
  format_move_stream(1, ctx, false, "void move(T& str, const %s& " instance_name ")\n", struct_name);
  format_key_move_stream(1, ctx, "void key_move(T& str, const %s& " instance_name ")\n", struct_name);
  format_move_stream(1, ctx, true, open_block);

  format_max_stream(1, ctx, true, template_decl);
  format_max_stream(1, ctx, false, "void max(T& str, const %s& " instance_name ")\n", struct_name);
  format_key_max_stream(1, ctx, "void key_max(T& str, const %s& " instance_name ")\n", struct_name);
  format_max_stream(1, ctx, true, open_block);

  ctx->depth++;

  free(struct_name);
}

static idl_retcode_t
process_case(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)path;

  context_t* ctx = (context_t*)user_data;
  const idl_case_t* _case = (const idl_case_t*)node;
  idl_retcode_t ret = IDL_RETCODE_OK;

  if (revisit)
  {
    format_read_stream(1, ctx, false, "{\n");
    format_max_stream(1, ctx, false, "{\n");
    format_max_stream(1, ctx, false, "  size_t pos = str.position();\n");
    format_max_stream(1, ctx, false, "  size_t alignment = str.alignment();\n");
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

    char* cpp11accessor = NULL;
    if (idl_asprintf(&cpp11accessor, member_access, instance_name, cpp11name) == -1)
      goto fail3;

    format_write_stream(1, ctx, false, write_instance, cpp11accessor);
    format_write_stream(1, ctx, false, union_case_ending);

    format_move_stream(1, ctx, false, move_instance, cpp11accessor);
    format_move_stream(1, ctx, false, union_case_ending);

    format_read_stream(1, ctx, false, "%s %s;\n", cpp11type, union_temp_instance);
    format_read_stream(1, ctx, false, read_instance, union_temp_instance);
    format_read_stream(1, ctx, false, "%s(%s,%s);\n", cpp11name, union_temp_instance, union_temp_discriminator);

    format_max_stream(1, ctx, false, max_instance, cpp11accessor);
    format_max_stream(1, ctx, false, "union_max = std::max(str.position(),union_max);\n");
    format_max_stream(1, ctx, false, "str.position(pos);\n");
    format_max_stream(1, ctx, false, "str.alignment(alignment);\n");

    ctx->depth--;

    format_read_stream(1, ctx, false, "}\n");
    format_read_stream(1, ctx, false, "  " union_case_ending);

    format_max_stream(1, ctx, false, "}\n");

    free(cpp11accessor);
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

static idl_retcode_t
process_inherit_spec(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  context_t* ctx = (context_t*)user_data;
  const idl_inherit_spec_t* ispec = (const idl_inherit_spec_t*)node;

  idl_retcode_t returnval = IDL_RETCODE_NO_MEMORY;

  ctx->key_entries++;

  char* base_cpp11name = get_cpp11_fully_scoped_name(ispec->base);
  if (!base_cpp11name)
    goto fail1;

  char* write_accessor = NULL, * read_accessor = NULL;
  if (idl_asprintf(&write_accessor, const_ref_cast, base_cpp11name) == -1)
    goto fail2;
  if (idl_asprintf(&read_accessor, ref_cast, base_cpp11name) == -1)
    goto fail3;

  format_write_stream(1, ctx, false, write_instance, write_accessor);
  format_read_stream(1, ctx, false, read_instance, read_accessor);
  format_move_stream(1, ctx, false, move_instance, write_accessor);
  format_max_stream(1, ctx, false, max_instance, write_accessor);
  format_key_write_stream(1, ctx, key_write_instance, write_accessor);
  format_key_read_stream(1, ctx, key_read_instance, read_accessor);
  format_key_move_stream(1, ctx, key_move_instance, write_accessor);
  format_key_max_stream(1, ctx, key_max_instance, write_accessor);

  returnval = IDL_RETCODE_OK;
  free(read_accessor);
fail3:
  free(write_accessor);
fail2:
  free(base_cpp11name);
fail1:
  return returnval;
}

static idl_retcode_t
process_keylist(context_t* ctx,
  const idl_keylist_t* keylist)
{
  idl_retcode_t returnval = IDL_VISIT_REVISIT;

  const idl_key_t* key = keylist->keys;
  while (key)
  {
    ctx->key_entries++;

    char* accessor = idl_strdup(instance_name);
    for (size_t fn = 0; fn < key->field_name->length; fn++)
    {
      char* temp = NULL;
      if (idl_asprintf(&temp, member_access, accessor, key->field_name->names[fn]->identifier) == -1)
      {
        returnval = IDL_RETCODE_NO_MEMORY;
        goto fail;
      }

      free(accessor);
      accessor = temp;
    }

    /*
    if ((idl_is_struct(spec) || idl_is_union(spec)) && has_keys(spec))
    {
      format_key_write_stream(1, ctx, key_write_instance, accessor);
      format_key_read_stream(1, ctx, key_read_instance, accessor);
      format_key_move_stream(1, ctx, key_move_instance, accessor);
      format_key_max_stream(1, ctx, key_max_instance, accessor);
    }
    else
    */
    {
      format_key_write_stream(1, ctx, write_instance, accessor);
      format_key_read_stream(1, ctx, read_instance, accessor);
      format_key_move_stream(1, ctx, move_instance, accessor);
      format_key_max_stream(1, ctx, max_instance, accessor);
    }

    key = (const idl_key_t*)key->node.next;
  fail:
    free(accessor);
  }

  return returnval;
}

static idl_retcode_t
process_struct_definition(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)path;

  idl_retcode_t returnval = IDL_RETCODE_OK;
  context_t* ctx = (context_t*)user_data;

  if (revisit)
  {
    print_constructed_type_close(ctx);
  }
  else
  {
    print_constructed_type_open(ctx, node);

    if (pstate->version == IDL35 &&
      ((idl_struct_t*)node)->keylist)
      returnval = process_keylist(ctx, (idl_keylist_t*)((idl_struct_t*)node)->keylist);
    else
      returnval = IDL_VISIT_REVISIT;
  }

  return returnval;
}

static idl_retcode_t
union_switch_start(context_t* ctx,
  idl_switch_type_spec_t* st)
{
  idl_retcode_t returnval = IDL_RETCODE_OK;

  char* discriminator_cast = get_cpp11_type(st->type_spec);
  if (!discriminator_cast)
  {
    returnval = IDL_RETCODE_NO_MEMORY;
    goto fail1;
  }

  format_write_stream(1, ctx, st->key, write_instance, union_discriminator_accessor);
  format_write_stream(1, ctx, false, union_switch, union_discriminator_accessor);
  format_write_stream(1, ctx, false, open_block);

  format_read_stream(1, ctx, st->key, "%s %s;\n", discriminator_cast, union_temp_discriminator);
  format_read_stream(1, ctx, st->key, read_instance, union_temp_discriminator);
  format_read_stream(1, ctx, false, union_switch, union_temp_discriminator);
  format_read_stream(1, ctx, false, open_block);

  format_move_stream(1, ctx, st->key, move_instance, union_discriminator_accessor);
  format_move_stream(1, ctx, false, union_switch, union_discriminator_accessor);
  format_move_stream(1, ctx, false, open_block);

  format_max_stream(1, ctx, st->key, max_instance, union_discriminator_accessor);
  format_max_stream(1, ctx, false, "size_t union_max = str.position();\n");

  free(discriminator_cast);
fail1:
  return returnval;
}

static idl_retcode_t
union_switch_stop(context_t* ctx)
{
  format_write_stream(1, ctx, false, close_block);
  format_move_stream(1, ctx, false, close_block);
  format_read_stream(1, ctx, false, close_block);
  format_max_stream(1, ctx, false, "str.position(union_max);\n");

  return IDL_RETCODE_OK;
}

static idl_retcode_t
process_union_definition(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)path;

  idl_retcode_t returnval = IDL_RETCODE_NO_MEMORY;
  context_t* ctx = (context_t*)user_data;
  char* cpp11name = get_cpp11_name(idl_identifier(node));
  if (!cpp11name)
    goto fail1;

  if (revisit)
  {
    if ((returnval = union_switch_stop(ctx)))
      goto fail2;

    print_constructed_type_close(ctx);

    returnval = IDL_RETCODE_OK;
  }
  else
  {
    print_constructed_type_open(ctx, node);

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

static idl_retcode_t
add_default_case(context_t* ctx)
{
  format_write_stream(1, ctx, false, default_case);
  format_move_stream(1, ctx, false, default_case);
  format_read_stream(1, ctx, false, default_case);

  return IDL_RETCODE_OK;
}

static idl_retcode_t
process_case_label(const idl_pstate_t* pstate,
  const bool revisit,
  const idl_path_t* path,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)revisit;
  (void)path;

  context_t* ctx = (context_t*)user_data;
  const idl_literal_t *lit = (const idl_literal_t*)((idl_case_label_t*)node)->const_expr;
  if (lit)
  {
    char *buffer = get_cpp11_const_value(lit);
    if (!buffer)
      goto fail1;

    format_write_stream(1, ctx, false, union_case, buffer);
    format_move_stream(1, ctx, false, union_case, buffer);
    format_read_stream(1, ctx, false, union_case, buffer);

    free(buffer);
    return IDL_RETCODE_OK;
fail1:
    return IDL_RETCODE_NO_MEMORY;
  }
  else
  {
    return add_default_case(ctx);
  }
}

void idl_streamers_generate(const idl_pstate_t* tree,
  idl_streamer_output_t* str)
{
  context_t* ctx = create_context();

  const char *srcs[] = { tree->sources ? tree->sources->path->name : NULL, NULL };

  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_STRUCT | IDL_UNION | IDL_MEMBER | IDL_CASE | IDL_CASE_LABEL | IDL_INHERIT_SPEC;
  visitor.accept[IDL_ACCEPT_STRUCT] = &process_struct_definition;
  visitor.accept[IDL_ACCEPT_UNION] = &process_union_definition;
  visitor.accept[IDL_ACCEPT_MEMBER] = &process_member;
  visitor.accept[IDL_ACCEPT_CASE] = &process_case;
  visitor.accept[IDL_ACCEPT_CASE_LABEL] = &process_case_label;
  visitor.accept[IDL_ACCEPT_INHERIT_SPEC] = &process_inherit_spec;
  if (tree->sources)
    visitor.sources = srcs;

  idl_visit(tree, tree->root, &visitor, ctx);

  close_context(ctx, str);
}
