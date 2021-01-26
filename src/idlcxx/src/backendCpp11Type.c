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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include "idlcxx/backendCpp11Utils.h"
#include "idlcxx/backendCpp11Type.h"
#include "idlcxx/processor_options.h"
#include "idlcxx/token_replace.h"
#include "idl/processor.h"

typedef struct cpp11_member_state cpp11_member_state_s;
struct cpp11_member_state
{
  const idl_node_t *member_type_node;
  const idl_declarator_t *declarator_node;
  char* type_name;
  char* member_name;
  bool bare_base_type;
  uint64_t array_entries;

  cpp11_member_state_s* next;
};

cpp11_member_state_s* create_cpp11_member_state(const idl_node_t* member_type_node, const idl_declarator_t* declarator_node)
{
  cpp11_member_state_s* newstate = malloc(sizeof(cpp11_member_state_s));

  assert(newstate);

  newstate->member_type_node = member_type_node;
  newstate->declarator_node = declarator_node;

  newstate->type_name = get_cpp11_type(member_type_node);
  newstate->array_entries = generate_array_expression(&newstate->type_name, declarator_node->const_expr);

  newstate->bare_base_type = (newstate->array_entries == 0 && (idl_is_base_type(member_type_node) || idl_is_enum(member_type_node)));

  newstate->member_name = get_cpp11_name(declarator_node->name->identifier);
  newstate->next = NULL;

  return newstate;
}

void cleanup_cpp11_member_state(cpp11_member_state_s* in)
{
  if (in)
  {
    if (in->type_name)
      free(in->type_name);

    free(in->member_name);

    free(in);
  }
}

typedef struct cpp11_struct_state cpp11_struct_state_s;
struct cpp11_struct_state
{
  char* type_name, * inherit_name;
  cpp11_member_state_s* first, * last;
};

cpp11_struct_state_s* create_cpp11_struct_state(const idl_struct_t* struct_node)
{
  assert(struct_node);

  cpp11_struct_state_s* newstate = malloc(sizeof(cpp11_struct_state_s));

  assert(newstate);

  newstate->type_name = get_cpp11_name(idl_identifier(struct_node));
  if (struct_node->inherit_spec)
  {
    const idl_node_t* base_node = (const idl_node_t*)struct_node->inherit_spec->base;
    newstate->inherit_name = get_cpp11_fully_scoped_name(base_node);
  }
  else
  {
    newstate->inherit_name = NULL;
  }

  newstate->first = NULL;
  newstate->last = NULL;

  return newstate;
}

void add_cpp11_member_state(cpp11_struct_state_s* in, cpp11_member_state_s* toadd)
{
  assert(in);
  assert(toadd);

  if (in->first)
    in->last->next = toadd;
  else
    in->first = toadd;

  in->last = toadd;
}

void cleanup_cpp11_struct_state(cpp11_struct_state_s* in)
{
  if (!in)
    return;

  cpp11_member_state_s* state = in->first;
  while (state)
  {
    cpp11_member_state_s* nextstate = state->next;
    cleanup_cpp11_member_state(state);
    state = nextstate;
  }

  free(in->type_name);
  if (in->inherit_name)
    free(in->inherit_name);

  free(in);
}

static idl_retcode_t
emit_member(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  const idl_member_t* member_node = (const idl_member_t*)node;
  cpp11_struct_state_s* customctx = (cpp11_struct_state_s*)user_data;

  idl_declarator_t* decl_node = member_node->declarators;
  idl_node_t* type_node = member_node->type_spec;
  while (decl_node)
  {
    add_cpp11_member_state(customctx, create_cpp11_member_state(type_node, decl_node));
    decl_node = (idl_declarator_t*)decl_node->node.next;
  }

  return IDL_VISIT_DONT_RECURSE;
}

static idl_retcode_t
generate_streamer_interfaces(idl_backend_ctx ctx)
{
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "size_t write_struct(void* data, size_t position) const;\n");
  idl_file_out_printf(ctx, "size_t write_size(size_t offset) const;\n");
  idl_file_out_printf(ctx, "size_t max_size(size_t offset) const;\n");
  idl_file_out_printf(ctx, "size_t read_struct(const void* data, size_t position);\n");
  idl_file_out_printf(ctx, "size_t key_size(size_t position) const;\n");
  idl_file_out_printf(ctx, "size_t key_max_size(size_t position) const;\n");
  idl_file_out_printf(ctx, "size_t key_write(void* data, size_t position) const;\n");
  idl_file_out_printf(ctx, "size_t key_read(const void* data, size_t position);\n");
  idl_file_out_printf(ctx, "bool key(ddsi_keyhash_t& hash) const;\n");
  idl_indent_decr(ctx);
  return IDL_RETCODE_OK;
}

static idl_retcode_t
emit_struct(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  const idl_struct_t* struct_node = (const idl_struct_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;
  idl_retcode_t ret = IDL_RETCODE_OK;

  cpp11_struct_state_s* strctx = create_cpp11_struct_state(struct_node);

  /*visit all the members of the struct*/
  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_MEMBER;
  visitor.accept[IDL_ACCEPT_MEMBER] = &emit_member;
  if (pstate->sources)
    visitor.glob = pstate->sources->path->name;
  if ((ret = idl_visit(pstate, struct_node->members, &visitor, strctx)))
    return ret;

  /* Create class declaration. */
  if (strctx->inherit_name)
    idl_file_out_printf(ctx, "class %s : public %s\n", strctx->type_name, strctx->inherit_name);
  else
    idl_file_out_printf(ctx, "class %s\n", strctx->type_name);

  idl_file_out_printf(ctx, "{\n");

  idl_file_out_printf(ctx, "private:\n");

  idl_indent_incr(ctx);

  /*members*/
  cpp11_member_state_s* ms = strctx->first;
  while (ms)
  {
    if (ms->bare_base_type)
    {
      char* def_val = get_default_value(ctx, ms->member_type_node);
      idl_file_out_printf(ctx, "%s %s_ = %s;\n", ms->type_name, ms->member_name, def_val);
      free(def_val);
    }
    else
    {
      idl_file_out_printf(ctx, "%s %s_%s;\n", ms->type_name, ms->member_name, ms->array_entries ? " = { }" : "");
    }

    ms = ms->next;
  }
  idl_file_out_printf(ctx, "\n");

  idl_indent_decr(ctx);

  idl_file_out_printf(ctx, "public:\n");

  idl_indent_incr(ctx);

  /*constructors*/
  idl_file_out_printf(ctx, "%s() = default;\n\n", strctx->type_name);
  idl_file_out_printf(ctx, "explicit %s(\n", strctx->type_name);

  idl_indent_double_incr(ctx);

  /*constructor arguments*/
  ms = strctx->first;
  while (ms)
  {
    if (ms->bare_base_type)
      idl_file_out_printf(ctx, "%s %s%s\n", ms->type_name, ms->member_name, strctx->last != ms ? "," : ") :");
    else
      idl_file_out_printf(ctx, "const %s& %s%s\n", ms->type_name, ms->member_name, strctx->last != ms ? "," : ") :");

    ms = ms->next;
  }
  idl_indent_double_incr(ctx);

  /*constructor member initialization*/
  ms = strctx->first;
  while (ms)
  {
    idl_file_out_printf(ctx, "%s_(%s)%s\n", ms->member_name, ms->member_name, strctx->last != ms ? "," : " {}");

    ms = ms->next;
  }
  idl_indent_double_decr(ctx);
  idl_indent_double_decr(ctx);
  idl_file_out_printf(ctx, "\n");

  /*accessors*/
  ms = strctx->first;
  while (ms)
  {
    if (ms->bare_base_type)
    {
      idl_file_out_printf(ctx, "%s %s() const { return this->%s_; }\n", ms->type_name, ms->member_name, ms->member_name);
      idl_file_out_printf(ctx, "%s& %s() { return this->%s_; }\n", ms->type_name, ms->member_name, ms->member_name);
      idl_file_out_printf(ctx, "void %s(%s _val_) { this->%s_ = _val_; }\n", ms->member_name, ms->type_name, ms->member_name);
    }
    else
    {
      idl_file_out_printf(ctx, "const %s& %s() const { return this->%s_; }\n", ms->type_name, ms->member_name, ms->member_name);
      idl_file_out_printf(ctx, "%s& %s() { return this->%s_; }\n", ms->type_name, ms->member_name, ms->member_name);
      idl_file_out_printf(ctx, "void %s(const %s& _val_) { this->%s_ = _val_; }\n", ms->member_name, ms->type_name, ms->member_name);
      idl_file_out_printf(ctx, "void %s(%s&& _val_) { this->%s_ = _val_; }\n", ms->member_name, ms->type_name, ms->member_name);
    }

    ms = ms->next;
  }
  idl_file_out_printf(ctx, "\n");

  idl_indent_decr(ctx);

  /*streamer interfaces*/
  generate_streamer_interfaces(ctx);

  idl_file_out_printf(ctx, "};\n\n");

  cleanup_cpp11_struct_state(strctx);

  return ret;
}

static idl_retcode_t
emit_enum(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)visit;

  const idl_enum_t* enum_node = (const idl_enum_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  char* cpp11Name = get_cpp11_name(idl_identifier(enum_node));
  idl_file_out_printf(ctx, "enum class %s\n", cpp11Name);
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);

  uint32_t skip = 0;
  for (const idl_enumerator_t* enumerator = enum_node->enumerators; enumerator; )
  {
    const idl_enumerator_t* next = idl_next(enumerator);

    /* IDL3.5 did not support fixed enumerator values */
    uint32_t value = enumerator->value;
    if (value == skip || (pstate->flags & IDL_FLAG_VERSION_35))
      idl_file_out_printf(ctx, "%s%s\n", enumerator->name->identifier, next ? "," : "");
    else
      idl_file_out_printf(ctx, "%s = %" PRIu32"%s\n", enumerator->name->identifier, value, next ? "," : "");

    skip = value + 1;
    enumerator = next;
  }

  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "};\n\n");
  free(cpp11Name);
  return IDL_VISIT_DONT_RECURSE;
}

static idl_retcode_t
emit_typedef(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  const idl_typedef_t* typedef_node = (const idl_typedef_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  char *cpp11Name = get_cpp11_name(idl_identifier(typedef_node->declarators));
  char *cpp11Type = get_cpp11_type(typedef_node->type_spec);

  idl_file_out_printf(ctx, "typedef %s %s;\n\n", cpp11Type, cpp11Name);

  free(cpp11Type);
  free(cpp11Name);
  return IDL_RETCODE_OK;
}

static idl_retcode_t
emit_module(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;

  const idl_module_t* module_node = (const idl_module_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  if (visit->type == IDL_EXIT)
  {
    idl_indent_decr(ctx);
    idl_file_out_printf(ctx, "}\n\n");
    return IDL_RETCODE_OK;
  }
  else
  {
    char* cpp11Name = get_cpp11_name(idl_identifier(module_node));
    idl_file_out_printf(ctx, "namespace %s\n", cpp11Name);
    idl_file_out_printf(ctx, "{\n", cpp11Name);
    idl_indent_incr(ctx);
    free(cpp11Name);
    return IDL_VISIT_REVISIT;
  }
}

static idl_retcode_t
emit_forward(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  const idl_forward_t * forward_node = (const idl_forward_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  char *cpp11Name = get_cpp11_name(idl_identifier(forward_node));
  //check for correct creation of cpp11name
  assert(idl_mask(&forward_node->node) & (IDL_STRUCT | IDL_UNION));
  idl_file_out_printf(
      ctx,
      "%s %s;\n\n",
      (idl_mask(&forward_node->node) & IDL_STRUCT) ? "struct" : "union",
      cpp11Name);

  //check for correct print
  free(cpp11Name);
  return IDL_RETCODE_OK;
}

static idl_retcode_t
emit_const(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  const idl_const_t* const_node = (const idl_const_t*)node;
  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  char* cpp11Name = get_cpp11_name(idl_identifier(const_node));
  char* cpp11Type = get_cpp11_type(const_node->type_spec);
  char* cpp11Value = get_cpp11_const_value((const idl_constval_t*)const_node->const_expr);

  //check for correct creation of cpp11*
  idl_file_out_printf(
    ctx,
    "const %s %s = %s;\n\n",
    cpp11Type,
    cpp11Name,
    cpp11Value);

  free(cpp11Value);
  free(cpp11Type);
  free(cpp11Name);
  return IDL_RETCODE_OK;
}

typedef enum
{
  idl_no_dep                = 0x0,
  idl_string_bounded_dep    = 0x01 << 0,
  idl_string_unbounded_dep  = 0x01 << 1,
  idl_array_dep             = 0x01 << 2,
  idl_vector_bounded_dep    = 0x01 << 3,
  idl_vector_unbounded_dep  = 0x01 << 4,
  idl_variant_dep           = 0x01 << 5,
  idl_optional_dep          = 0x01 << 6,
  idl_all_dep               = (idl_optional_dep*2-1)
} idl_include_dep;

static idl_retcode_t
type_spec_includes(
  idl_type_spec_t* ts,
  idl_include_dep* id)
{
  if (idl_is_masked(ts, IDL_SEQUENCE))
  {
    uint64_t bound = ((idl_sequence_t*)ts)->maximum;
    if (bound != 0 &&
      (idl_vector_bounded_dep & *id) == idl_no_dep)
      *id |= idl_vector_bounded_dep;
    else if (bound == 0 && (idl_vector_unbounded_dep & *id) == idl_no_dep)
      *id |= idl_vector_unbounded_dep;
  }

  if (idl_is_masked(ts, IDL_STRING))
  {
    uint64_t bound = ((idl_string_t*)ts)->maximum;
    if (bound != 0 &&
      (idl_string_bounded_dep & *id) == idl_no_dep)
      *id |= idl_string_bounded_dep;
    else if (bound == 0 &&
      (idl_string_unbounded_dep & *id) == idl_no_dep)
      *id |= idl_string_unbounded_dep;
  }

  return IDL_RETCODE_OK;
}

typedef struct cpp11_case_state_s
{
  const idl_node_t *typespec_node;
  const idl_declarator_t *declarator_node;
  char *name;
  char *type_name;
  uint32_t label_count;
  char **labels;
} cpp11_case_state;

typedef struct cpp11_union_context_s
{
  cpp11_case_state *cases;
  uint32_t case_count;
  uint64_t total_label_count, nr_unused_discr_values;
  const idl_node_t *discr_node;
  char *discr_type;
  const char *name;
  const cpp11_case_state *default_case;
  char *default_label;
  bool has_impl_default;
} cpp11_union_context;

static idl_retcode_t
count_labels(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;
  uint32_t* nr_labels = (uint32_t*)user_data;
  const idl_case_label_t* label = (const idl_case_label_t*)node;
  if (label->const_expr) ++(*nr_labels);
  return IDL_RETCODE_OK;
}

static idl_retcode_t
count_cases(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;
  uint32_t* nr_cases = (uint32_t*)user_data;
  (void)node;
  ++(*nr_cases);
  return IDL_RETCODE_OK;
}

static char *
get_label_value(const idl_case_label_t *label)
{
  char *label_value;

  if (idl_mask(label->const_expr) & IDL_ENUMERATOR) {
    label_value = get_cpp11_fully_scoped_name(label->const_expr);
  } else {
    label_value = get_cpp11_const_value((const idl_constval_t *) label->const_expr);
  }
  return label_value;
}

static idl_retcode_t
emit_case_label(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;
  cpp11_union_context *union_data = (cpp11_union_context *) user_data;
  const idl_case_label_t *label = (const idl_case_label_t *) node;
  cpp11_case_state *case_data = union_data->cases + union_data->case_count;
  /* Check if there is a label: if not it represents the default case. */
  if (label->const_expr) {
    case_data->labels[case_data->label_count] = get_label_value(label);
    ++(case_data->label_count);
  } else {
    /* Assert that there can only be one default case */
    assert(union_data->default_case == NULL);
    union_data->default_case = case_data;
  }
  return IDL_RETCODE_OK;
}

static idl_retcode_t
emit_case(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)visit;

  idl_retcode_t result;
  cpp11_union_context *union_ctx = (cpp11_union_context *) user_data;
  cpp11_case_state *case_data = &union_ctx->cases[union_ctx->case_count];
  const idl_case_t *case_node = (const idl_case_t *) node;

  uint32_t case_labels = 0;
  case_data->label_count = 0;
  /*count the number of labels, maybe better solved by linked list approach?*/
  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_CASE_LABEL;
  visitor.accept[IDL_ACCEPT_CASE_LABEL] = &count_labels;
  if (pstate->sources)
    visitor.glob = pstate->sources->path->name;
  if ((result = idl_visit(pstate, case_node->case_labels, &visitor, &case_labels)))
    return result;

  union_ctx->total_label_count += case_labels;

  case_data->typespec_node = case_node->type_spec;
  case_data->declarator_node = case_node->declarator;
  case_data->name = get_cpp11_name(idl_identifier(case_node->declarator));
  case_data->type_name = get_cpp11_type(case_node->type_spec);
  /* Check if the declarator contains also an array expression... */
  generate_array_expression(&case_data->type_name, case_data->declarator_node->const_expr);

  if (case_labels > 0) {
    case_data->labels = malloc(sizeof(char*) * case_labels);
  }
  else {
    case_data->labels = NULL;
  }

  /*visit the individual labels*/
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_CASE_LABEL;
  visitor.accept[IDL_ACCEPT_CASE_LABEL] = &emit_case_label;
  if (pstate->sources)
    visitor.glob = pstate->sources->path->name;
  if ((result = idl_visit(pstate, case_node->case_labels, &visitor, union_ctx)))
    return result;

  ++(union_ctx->case_count);
  return IDL_RETCODE_OK;
}

static uint64_t
get_potential_nr_discr_values(const idl_union_t *union_node)
{
  uint64_t nr_discr_values = 0;
  const idl_type_spec_t *node = union_node->switch_type_spec ? union_node->switch_type_spec->type_spec : NULL;

  switch (idl_mask(node) & (IDL_BASE_TYPE | IDL_CONSTR_TYPE))
  {
  case IDL_BASE_TYPE:
    switch (idl_mask(node) & IDL_BASE_TYPE_MASK)
    {
    case IDL_INTEGER_TYPE:
      switch(idl_mask(node) & IDL_INTEGER_MASK_IGNORE_SIGN)
      {
      case IDL_INT8:
        nr_discr_values = UINT8_MAX;
        break;
      case IDL_INT16:
      case IDL_SHORT:
        nr_discr_values = UINT16_MAX;
        break;
      case IDL_INT32:
      case IDL_LONG:
        nr_discr_values = UINT32_MAX;
        break;
      case IDL_INT64:
      case IDL_LLONG:
        nr_discr_values = UINT64_MAX;
        break;
      default:
        assert(0);
        break;
      }
      break;
    default:
      switch(idl_mask(node) & IDL_BASE_OTHERS_MASK)
      {
      case IDL_CHAR:
      case IDL_OCTET:
        nr_discr_values = UINT8_MAX;
        break;
      case IDL_WCHAR:
        nr_discr_values = UINT16_MAX;
        break;
      case IDL_BOOL:
        nr_discr_values = 2;
        break;
      default:
        assert(0);
        break;
      }
      break;
    }
    break;
  case IDL_CONSTR_TYPE:
    switch (idl_mask(node) & IDL_ENUM)
    {
    case IDL_ENUM:
    {
      /* Pick the first of the available enumerators. */
      const idl_enum_t *enumeration = (const idl_enum_t *) node;
      const idl_enumerator_t* enumerator = enumeration->enumerators;
      nr_discr_values = 0;
      while(enumerator)
      {
        ++nr_discr_values;
        enumerator = (const idl_enumerator_t*)enumerator->node.next;
      }
      break;
    }
    default:
      assert(0);
      break;
    }
    break;
  default:
    assert(0);
    break;
  }
  return nr_discr_values;
}

static idl_constval_t
get_min_value(const idl_node_t *node)
{
  idl_constval_t result;
  static const idl_mask_t mask = (IDL_BASE_TYPE|(IDL_BASE_TYPE-1));

  result.node = *node;
  result.node.symbol.mask &= mask;
  switch (idl_mask(node) & mask)
  {
  case IDL_BOOL:
    result.value.bln = false;
    break;
  case IDL_CHAR:
    result.value.chr = 0;
    break;
  case IDL_INT8:
    result.value.int8 = INT8_MIN;
    break;
  case IDL_UINT8:
  case IDL_OCTET:
    result.value.uint8 = 0;
    break;
  case IDL_INT16:
  case IDL_SHORT:
    result.value.int16 = INT16_MIN;
    break;
  case IDL_UINT16:
  case IDL_USHORT:
    result.value.uint16 = 0;
    break;
  case IDL_INT32:
  case IDL_LONG:
    result.value.int32 = INT32_MIN;
    break;
  case IDL_UINT32:
  case IDL_ULONG:
    result.value.uint32 = 0;
    break;
  case IDL_INT64:
  case IDL_LLONG:
    result.value.int64 = INT64_MIN;
    break;
  case IDL_UINT64:
  case IDL_ULLONG:
    result.value.uint64 = 0ULL;
    break;
  default:
    assert(0);
    break;
  }
  result.node.symbol.mask |= IDL_CONST;
  return result;
}

static void *
enumerator_incr_value(void *val)
{
  const idl_enumerator_t *enum_val = (const idl_enumerator_t *) val;
  return enum_val->node.next;
}

static void *
constval_incr_value(void *val)
{
  idl_constval_t *cv = (idl_constval_t *)val;
  static const idl_mask_t mask = (IDL_BASE_TYPE|(IDL_BASE_TYPE-1));

  switch (idl_mask(&cv->node) & mask)
  {
  case IDL_BOOL:
    cv->value.bln = true;
    break;
  case IDL_INT8:
    cv->value.int8++;
    break;
  case IDL_UINT8:
    cv->value.uint8++;
    break;
  case IDL_INT16:
    cv->value.int16++;
    break;
  case IDL_UINT16:
    cv->value.uint16++;
    break;
  case IDL_INT32:
    cv->value.int32++;
    break;
  case IDL_UINT32:
    cv->value.uint32++;
    break;
  case IDL_INT64:
    cv->value.int64++;
    break;
  case IDL_UINT64:
    cv->value.uint64++;
    break;
  default:
    assert(0);
    break;
  }
  return cv;
}

typedef enum { IDL_LT, IDL_EQ, IDL_GT} idl_equality_t;
typedef idl_equality_t (*idl_comparison_fn) (const void *element1, const void *element2);
typedef void *(*idl_incr_element_fn) (void *element1);

static idl_equality_t
compare_enum_elements(const void *element1, const void *element2)
{
  const idl_enumerator_t *enumerator1 = (const idl_enumerator_t *) element1;
  const idl_enumerator_t *enumerator2 = (const idl_enumerator_t *) element2;
  idl_equality_t result = IDL_EQ;

  if (enumerator1->value < enumerator2->value) result = IDL_LT;
  if (enumerator1->value > enumerator2->value) result = IDL_GT;
  return result;
}

static idl_equality_t
compare_const_elements(const void *element1, const void *element2)
{
#define EQ(a,b) ((a<b) ? IDL_LT : ((a>b) ? IDL_GT : IDL_EQ))
  const idl_constval_t *cv1 = (const idl_constval_t *) element1;
  const idl_constval_t *cv2 = (const idl_constval_t *) element2;
  static const idl_mask_t mask = IDL_BASE_TYPE|(IDL_BASE_TYPE-1);

  assert((idl_mask(&cv1->node) & mask) == (idl_mask(&cv2->node) & mask));
  switch (idl_mask(&cv1->node) & mask)
  {
  case IDL_BOOL:
    return EQ(cv1->value.bln, cv2->value.bln);
  case IDL_INT8:
    return EQ(cv1->value.int8, cv2->value.int8);
  case IDL_UINT8:
  case IDL_OCTET:
    return EQ(cv1->value.uint8, cv2->value.uint8);
  case IDL_INT16:
  case IDL_SHORT:
    return EQ(cv1->value.int16, cv2->value.int16);
  case IDL_UINT16:
  case IDL_USHORT:
    return EQ(cv1->value.uint16, cv2->value.uint16);
  case IDL_INT32:
  case IDL_LONG:
    return EQ(cv1->value.int32, cv2->value.int32);
  case IDL_UINT32:
  case IDL_ULONG:
    return EQ(cv1->value.uint32, cv2->value.uint32);
  case IDL_INT64:
  case IDL_LLONG:
    return EQ(cv1->value.int64, cv2->value.int64);
  case IDL_UINT64:
  case IDL_ULLONG:
    return EQ(cv1->value.uint64, cv2->value.uint64);
  default:
    assert(0);
    break;
  }
  return IDL_EQ;
#undef EQ
}

static void
swap(void **element1, void **element2)
{
  void *tmp = *element1;
  *element1 = *element2;
  *element2 = tmp;
}

static uint64_t
manage_pivot (void **array, uint64_t low, uint64_t high, idl_comparison_fn compare_elements)
{
  uint64_t i = low;
  void *pivot = array[high];

  for (uint64_t j = low; j <= high- 1; j++)
  {
    if (compare_elements(array[j], pivot) == IDL_LT)
    {
      swap(&array[i++], &array[j]);
    }
  }
  swap(&array[i], &array[high]);
  return i;
}

static void quick_sort(void **array, uint64_t low, uint64_t high, idl_comparison_fn compare_elements)
{
  uint64_t pivot_index;

  if (low < high)
  {
    pivot_index = manage_pivot(array, low, high, compare_elements);
    quick_sort(array, low, pivot_index - 1, compare_elements);
    quick_sort(array, pivot_index + 1, high, compare_elements);
  }
}

static char *
get_first_unused_discr_value(
    void **array,
    void *min_value,
    void *max_value,
    idl_comparison_fn compare_elements,
    idl_incr_element_fn incr_element)
{

  uint32_t i = 0;
  do
  {
    if (compare_elements(min_value, array[i]) == IDL_LT)
    {
      return get_cpp11_const_value(min_value);
    }
    min_value = incr_element(min_value);
    if (array[i] != max_value) ++i;
  } while (compare_elements(min_value, array[i]) != IDL_GT);
  return get_cpp11_const_value(min_value);
}

static char *
get_default_discr_value(idl_backend_ctx ctx, const idl_union_t *union_node)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);
  char *def_value = NULL;
  union_ctx->nr_unused_discr_values =
      get_potential_nr_discr_values(union_node) - union_ctx->total_label_count;
  idl_constval_t min_const_value;
  void *min_value;
  idl_comparison_fn compare_elements;
  idl_incr_element_fn incr_element;

  if (union_ctx->nr_unused_discr_values) {
    /* find the smallest available unused discr_value. */
    void **all_labels = malloc(sizeof(void *) * (size_t)union_ctx->total_label_count);
    uint32_t i = 0;
    const idl_case_t *case_data = union_node->cases;
    while (case_data)
    {
      const idl_case_label_t *label = case_data->case_labels;
      while (label)
      {
        if (label->const_expr) all_labels[i++] = label->const_expr;
        label = (const idl_case_label_t *) label->node.next;
      }
      case_data = (const idl_case_t *) case_data->node.next;
    }
    if (idl_mask(union_node->switch_type_spec->type_spec) & IDL_ENUM) {
      compare_elements = compare_enum_elements;
      incr_element = enumerator_incr_value;
      min_value = ((const idl_enum_t *)union_ctx->discr_node)->enumerators;
    } else {
      compare_elements = compare_const_elements;
      incr_element = constval_incr_value;
      min_const_value = get_min_value((idl_node_t*)union_node->switch_type_spec->type_spec);
      min_value = &min_const_value;
    }
    quick_sort(all_labels, 0, union_ctx->total_label_count - 1, compare_elements);
    def_value = get_first_unused_discr_value(
        all_labels,
        min_value,
        all_labels[union_ctx->total_label_count - 1],
        compare_elements,
        incr_element);
    if (!union_ctx->default_case) union_ctx->has_impl_default = true;
    free(all_labels);
  } else {
    /* Pick the first available literal value from the first available case. */
    const idl_const_expr_t *const_expr = union_node->cases->case_labels->const_expr;
    def_value = get_cpp11_const_value((const idl_constval_t *) const_expr);
  }

  return def_value;
}

static void
union_generate_attributes(idl_backend_ctx ctx)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  idl_file_out_printf(ctx, "private:\n");
  idl_indent_incr(ctx);

  /* Declare a union attribute representing the discriminator. */
  idl_file_out_printf(ctx, "%s m__d;\n", union_ctx->discr_type);

  /* Declare a union attribute comprising of all the branch types. */
  idl_file_out_printf(ctx, "%s<\n", union_template);
  idl_indent_double_incr(ctx);
  for (uint32_t i = 0; i < union_ctx->case_count; ++i) {
    idl_file_out_printf(
        ctx,
        "%s%s\n",
        union_ctx->cases[i].type_name,
        i == (union_ctx->case_count - 1) ? "" : ",");
  }
  idl_indent_double_decr(ctx);
  idl_file_out_printf(ctx, ">");
  for (uint32_t i = 0; i < union_ctx->case_count; ++i) {
    idl_file_out_printf_no_indent(
        ctx,
        " %s%s",
        union_ctx->cases[i].name,
        (i ==  (union_ctx->case_count - 1)) ? ";" : ",");
  }
  idl_file_out_printf_no_indent(ctx, "\n\n");
  idl_indent_decr(ctx);
}

static void
union_generate_constructor(idl_backend_ctx ctx)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  idl_file_out_printf(ctx, "public:\n");
  idl_indent_incr(ctx);

  /* Start building default (empty) constructor. */
  idl_file_out_printf(ctx, "%s() :\n", union_ctx->name);
  idl_indent_double_incr(ctx);
  idl_file_out_printf(ctx, "m__d(%s)", union_ctx->default_label);

  /* If there is no implicit default case, then a value should be set explicitly. */
  if (!union_ctx->has_impl_default)
  {
    idl_file_out_printf_no_indent(ctx, ",\n");

    /* If there is an explicit default case, then pick that one. */
    if (union_ctx->default_case) {
      idl_file_out_printf(ctx, "%s()", union_ctx->default_case->name);
    } else {
      /* Otherwise pick the first case that is specified. */
      idl_file_out_printf(ctx, "%s()", union_ctx->cases[0].name);
    }
  }
  idl_file_out_printf_no_indent(ctx, " {}\n\n");
  idl_indent_double_decr(ctx);
  idl_indent_decr(ctx);
}
static void
union_generate_discr_getter_setter(idl_backend_ctx ctx)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  idl_indent_incr(ctx);
  /* Generate a getter for the discriminator. */
  idl_file_out_printf(ctx, "%s _d() const\n", union_ctx->discr_type);
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "return m__d;\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");

  /* Generate a setter for the discriminator. */
  idl_file_out_printf(ctx, "void _d(%s val)\n", union_ctx->discr_type);
  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);

  if ((idl_mask(union_ctx->discr_node) & IDL_BOOL) == IDL_BOOL) {
    idl_file_out_printf(ctx, "bool valid = (val == m__d);\n\n");
  } else {
    idl_file_out_printf(ctx, "bool valid = true;\n");
    idl_file_out_printf(ctx, "switch (val) {\n");
    for (uint32_t i = 0; i < union_ctx->case_count; ++i) {
      if (&union_ctx->cases[i] == union_ctx->default_case)
      {
        continue;
      }
      else
      {
        for (uint32_t j = 0; j < union_ctx->cases[i].label_count; ++j) {
          idl_file_out_printf(ctx, "case %s:\n", union_ctx->cases[i].labels[j]);
        }
        idl_indent_incr(ctx);
        for (uint32_t j = 0; j < union_ctx->cases[i].label_count; ++j) {
          if (j == 0) {
            idl_file_out_printf(ctx, "if (m__d != %s", union_ctx->cases[i].labels[j]);
            idl_indent_double_incr(ctx);
          } else {
            idl_file_out_printf_no_indent(ctx, " &&\n");
            idl_file_out_printf(ctx, "m__d != %s", union_ctx->cases[i].labels[j]);
          }
        }
        idl_indent_decr(ctx);
        idl_file_out_printf_no_indent(ctx, ") {\n");
        idl_file_out_printf(ctx, "valid = false;\n");
        idl_indent_decr(ctx);
        idl_file_out_printf(ctx, "}\n");
        idl_file_out_printf(ctx, "break;\n");
        idl_indent_decr(ctx);
      }
    }
    if (union_ctx->default_case || union_ctx->has_impl_default) {
      idl_file_out_printf(ctx, "default:\n");
      idl_indent_incr(ctx);
      for (uint32_t i = 0; i < union_ctx->case_count; ++i) {
        for (uint32_t j = 0; j < union_ctx->cases[i].label_count; ++j) {
          if (i == 0 && j == 0) {
            idl_file_out_printf(ctx, "if (m__d == %s", union_ctx->cases[i].labels[j]);
            idl_indent_double_incr(ctx);
          }
          else
          {
            idl_file_out_printf_no_indent(ctx, " ||\n");
            idl_file_out_printf(ctx, "m__d == %s", union_ctx->cases[i].labels[j]);
          }
        }
      }
      idl_file_out_printf_no_indent(ctx, ") {\n");
      idl_indent_decr(ctx);
      idl_file_out_printf(ctx, "valid = false;\n");
      idl_indent_decr(ctx);
      idl_file_out_printf(ctx, "}\n");
      idl_file_out_printf(ctx, "break;\n");
    }

    idl_indent_decr(ctx);
    idl_file_out_printf(ctx, "}\n\n");
  }

  idl_file_out_printf(ctx, "if (!valid) {\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "throw dds::core::InvalidArgumentError(\"New discriminator value does not match current discriminator\");\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_file_out_printf(ctx, "m__d = val;\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
  idl_indent_decr(ctx);
}

static void
union_generate_getter_body(idl_backend_ctx ctx, uint32_t i)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  if (&union_ctx->cases[i] != union_ctx->default_case) {
    for (uint32_t j = 0; j < union_ctx->cases[i].label_count; ++j) {
      if (j == 0) {
        idl_file_out_printf(ctx, "if (m__d == %s", union_ctx->cases[i].labels[j]);
        idl_indent_double_incr(ctx);
      } else {
        idl_file_out_printf_no_indent(ctx, " || \n");
        idl_file_out_printf(ctx, "m__d == %s", union_ctx->cases[i].labels[j]);
      }
    }
    idl_file_out_printf_no_indent(ctx, ") {\n");
  } else {
    for (uint32_t j = 0; j < union_ctx->case_count; ++j) {
      for (uint32_t k = 0; k < union_ctx->cases[j].label_count; k++) {
        if (j == 0 && k == 0) {
          idl_file_out_printf(ctx, "if (m__d != %s", union_ctx->cases[j].labels[k]);
          idl_indent_double_incr(ctx);
        } else {
          idl_file_out_printf_no_indent(ctx, " && \n");
          idl_file_out_printf(ctx, "m__d != %s", union_ctx->cases[j].labels[k]);
        }
      }
    }
    idl_file_out_printf_no_indent(ctx, ") {\n");
  }
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "return %s<%s>(%s);\n", union_getter_template, union_ctx->cases[i].type_name, union_ctx->cases[i].name);
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "} else {\n");
  idl_indent_incr(ctx);
  idl_file_out_printf(ctx, "throw dds::core::InvalidArgumentError(\"Requested branch does not match current discriminator\");\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n");
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
}

static void
union_generate_setter_body(idl_backend_ctx ctx, uint32_t i)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  idl_file_out_printf(ctx, "{\n");
  idl_indent_incr(ctx);
  /* Check if a setter with optional discriminant value is present (when more than 1 label). */
  if (union_ctx->cases[i].label_count < 2)
  {
    const char *new_discr_value;

    /* If not, check whether the current case is the default case. */
    if (&union_ctx->cases[i] != union_ctx->default_case) {
      /* If not the default case, pick the first available value. */
      new_discr_value = union_ctx->cases[i].labels[0];
    } else {
      /* If it is the default case, pick the default label. */
      new_discr_value = union_ctx->default_label;
    }
    idl_file_out_printf(ctx, "m__d = %s;\n", new_discr_value);
  }
  else
  {
    /* In case the discriminant is explicitly passed, check its validity and then take that value. */
    for (uint32_t j = 0; j < union_ctx->cases[i].label_count; ++j) {
      if (j == 0) {
        idl_file_out_printf(ctx, "if (m__d != %s", union_ctx->cases[i].labels[j]);
        idl_indent_double_incr(ctx);
      } else {
        idl_file_out_printf_no_indent(ctx, " &&\n");
        idl_file_out_printf(ctx, "m__d != %s", union_ctx->cases[i].labels[j]);
      }
    }
    idl_indent_decr(ctx);
    idl_file_out_printf_no_indent(ctx, ") {\n");
    idl_file_out_printf(ctx, "throw dds::core::InvalidArgumentError(\"Provided discriminator does not match selected branch\");\n");
    idl_indent_decr(ctx);
    idl_file_out_printf(ctx, "} else {\n");
    idl_indent_incr(ctx);
    idl_file_out_printf(ctx, "m__d = _d;\n");
    idl_indent_decr(ctx);
    idl_file_out_printf(ctx, "}\n");
  }
  idl_file_out_printf(ctx, "%s = val;\n", union_ctx->cases[i].name);
  idl_indent_decr(ctx);
  idl_file_out_printf(ctx, "}\n\n");
}

static void
union_generate_branch_getters_setters(idl_backend_ctx ctx)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);

  /* Start building the getter/setter methods for each attribute. */
  idl_indent_incr(ctx);
  for (uint32_t i = 0; i < union_ctx->case_count; ++i) {
    bool is_primitive = idl_declarator_is_primitive(union_ctx->cases[i].declarator_node);
    const cpp11_case_state *case_state = &union_ctx->cases[i];

    /* Build const-getter. */
    idl_file_out_printf(ctx, "%s%s%s %s() const\n",
        is_primitive ? "" : "const ",
        case_state->type_name,
        is_primitive ? "" : "&",
        case_state->name);
    union_generate_getter_body(ctx, i);

    /* Build ref-getter. */
    idl_file_out_printf(ctx, "%s& %s()\n",
        case_state->type_name, case_state->name);
    union_generate_getter_body(ctx, i);

    /* Build setter. */
    if (case_state->label_count < 2)
    {
      /* No need for optional discriminant parameter if there is only one applicable label. */
      idl_file_out_printf(ctx, "void %s(%s%s%s val)\n",
          case_state->name,
          is_primitive ? "": "const ",
          case_state->type_name,
          is_primitive ? "" : "&");
    }
    else
    {
      /* Use optional discriminant parameter if there is more than one applicable label. */
      idl_file_out_printf(ctx, "void %s(%s%s%s val, %s _d = %s)\n",
          case_state->name,
          is_primitive ? "": "const ",
          case_state->type_name,
          is_primitive ? "" : "&",
          union_ctx->discr_type,
          case_state->labels[0]);
    }
    union_generate_setter_body(ctx, i);

    /* When appropriate, build setter with move semantics. */
    if (!is_primitive) {
      if (case_state->label_count < 2)
      {
        idl_file_out_printf(ctx, "void %s(%s&& val)\n",
            case_state->name,
            case_state->type_name);
      }
      else
      {
        idl_file_out_printf(ctx, "void %s(%s&& val, %s _d = %s)\n",
            case_state->name,
            case_state->type_name,
            union_ctx->discr_type,
            case_state->labels[0]);
      }
      union_generate_setter_body(ctx, i);
    }
  }
  idl_indent_decr(ctx);
}

static void
union_generate_implicit_default_setter(idl_backend_ctx ctx)
{
  cpp11_union_context *union_ctx = (cpp11_union_context *) idl_get_custom_context(ctx);
  const char *discr_value;

  /* Check if all possible discriminant values are covered. */
  if (union_ctx->has_impl_default)
  {
    idl_indent_incr(ctx);
    if (union_ctx->nr_unused_discr_values < 2)
    {
      idl_file_out_printf(ctx, "void _default()\n");
      discr_value = union_ctx->default_label;
    }
    else
    {
      idl_file_out_printf(ctx, "void _default(%s _d = %s)\n",
          union_ctx->discr_type,
          discr_value = union_ctx->default_label);
      discr_value = "_d";
    }
    idl_file_out_printf(ctx, "{\n");
    idl_indent_incr(ctx);
    idl_file_out_printf(ctx, "m__d = %s;\n", discr_value);
    idl_indent_decr(ctx);
    idl_file_out_printf(ctx, "}\n\n");
    idl_indent_decr(ctx);
  }
}

static idl_retcode_t
emit_union(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)visit;

  idl_backend_ctx ctx = (idl_backend_ctx)user_data;

  const idl_union_t* union_node = (const idl_union_t*)node;
  idl_retcode_t result = IDL_VISIT_DONT_RECURSE;
  char* cpp11Name = get_cpp11_name(idl_identifier(union_node));

  idl_file_out_printf(ctx, "class %s\n", cpp11Name);
  idl_file_out_printf(ctx, "{\n");

  /*count the number of cases, maybe better solved by linked list approach?*/
  uint32_t nr_cases = 0;
  idl_visitor_t visitor;
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_CASE;
  visitor.accept[IDL_ACCEPT_CASE] = &count_cases;
  if (pstate->sources)
    visitor.glob = pstate->sources->path->name;
  if ((result = idl_visit(pstate, union_node->cases, &visitor, &nr_cases)))
    return result;

  /*create container for the number of cases*/
  cpp11_union_context union_ctx;
  union_ctx.cases = malloc(sizeof(cpp11_case_state) * nr_cases);
  union_ctx.case_count = 0;
  union_ctx.discr_node = (idl_node_t*)union_node->switch_type_spec->type_spec;
  union_ctx.discr_type = get_cpp11_type((idl_node_t*)union_node->switch_type_spec->type_spec);
  union_ctx.name = cpp11Name;
  union_ctx.default_case = NULL;
  union_ctx.has_impl_default = false;
  union_ctx.total_label_count = 0;
  result = idl_set_custom_context(ctx, &union_ctx);
  if (result)
    return result;

  /*walk over the number of cases to collect the data*/
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_CASE;
  visitor.accept[IDL_ACCEPT_CASE] = &emit_case;
  if (pstate->sources)
    visitor.glob = pstate->sources->path->name;
  if ((result = idl_visit(pstate, union_node->cases, &visitor, &union_ctx)))
    return result;

  union_ctx.default_label = get_default_discr_value(ctx, union_node);

  /* Generate the union content. */
  union_generate_attributes(ctx);
  union_generate_constructor(ctx);
  union_generate_discr_getter_setter(ctx);
  union_generate_branch_getters_setters(ctx);
  union_generate_implicit_default_setter(ctx);
  generate_streamer_interfaces(ctx);

  idl_file_out_printf(ctx, "};\n\n");
  idl_reset_custom_context(ctx);

  for (uint32_t i = 0; i < nr_cases; ++i)
  {
    free(union_ctx.cases[i].name);
    free(union_ctx.cases[i].type_name);
    for (uint32_t j = 0; j < union_ctx.cases[i].label_count; ++j)
    {
      free(union_ctx.cases[i].labels[j]);
    }
    free(union_ctx.cases[i].labels);
  }
  free(union_ctx.cases);
  free(union_ctx.discr_type);
  free(union_ctx.default_label);
  free(cpp11Name);

  return result;
}

static idl_retcode_t
emit_includes(
  const idl_pstate_t* pstate,
  idl_visit_t* visit,
  const void* node,
  void* user_data)
{
  (void)pstate;
  (void)visit;

  idl_backend_ctx ctx = (idl_backend_ctx)user_data;
  idl_include_dep *dependency_mask = (idl_include_dep *) idl_get_custom_context(ctx);

  if (idl_is_masked(node, IDL_UNION) &&
    (idl_variant_dep & *dependency_mask) == idl_no_dep)
    *dependency_mask |= idl_variant_dep;

  if (idl_is_masked(node, IDL_DECLARATOR))
  {
    if (((const idl_declarator_t*)node)->const_expr &&
      (idl_array_dep & *dependency_mask) == idl_no_dep)
      *dependency_mask |= idl_array_dep;

    idl_node_t *parent = ((idl_node_t*)node)->parent;
    if (idl_is_member(parent))
      type_spec_includes(((idl_member_t*)parent)->type_spec, dependency_mask);
    else if (idl_is_case(parent))
      type_spec_includes(((idl_case_t*)parent)->type_spec, dependency_mask);
    else if (idl_is_typedef(parent))
      type_spec_includes(((idl_typedef_t*)parent)->type_spec, dependency_mask);
  }

  return IDL_RETCODE_OK;
}

static idl_retcode_t
idl_generate_include_statements(idl_backend_ctx ctx, const idl_pstate_t *parse_tree)
{
  uint32_t nr_includes = 0;

  /* First determine the list of files included by our IDL file itself. */
  for (idl_source_t* include = parse_tree->sources->includes; include; include = include->next) {
    char *file, *dot;
    file = include->file->name;
    dot = strrchr(file, '.');
    if (!dot) dot = file + strlen(file);
    idl_file_out_printf(ctx, "#include \"%.*s.hpp\"\n", dot - file, file);
  }
  if (nr_includes == 0) {
    idl_file_out_printf(ctx, "#include <cstddef>\n");
    idl_file_out_printf(ctx, "#include <cstdint>\n\n");
    idl_file_out_printf(ctx, "#include \"dds/ddsi/ddsi_keyhash.h\"\n");
  }
  idl_file_out_printf(ctx, "\n");

  idl_visitor_t visitor;

  idl_include_dep util_depencencies = idl_no_dep;
  /* Next determine if we need to include any utility libraries... */
  idl_set_custom_context(ctx, &util_depencencies);
  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = ((idl_mask_t)-1);
  visitor.accept[IDL_ACCEPT] = &emit_includes;
  if (parse_tree->sources)
    visitor.glob = parse_tree->sources->path->name;
  idl_retcode_t ret;
  if ((ret = idl_visit(parse_tree, parse_tree->root, &visitor, ctx)))
    return ret;

  idl_reset_custom_context(ctx);
  if (util_depencencies) {
    if (strcmp(bounded_sequence_include, sequence_include))
    {
      if (util_depencencies & idl_vector_bounded_dep) {
        idl_file_out_printf(ctx, "#include %s\n", bounded_sequence_include);
      }
      if (util_depencencies & idl_vector_unbounded_dep) {
        idl_file_out_printf(ctx, "#include %s\n", sequence_include);
      }
    }
    else
    {
      if (util_depencencies & (idl_vector_bounded_dep | idl_vector_unbounded_dep)) {
        idl_file_out_printf(ctx, "#include %s\n", sequence_include);
      }
    }
    if (strcmp(bounded_string_include, string_include))
    {
      if (util_depencencies & idl_string_bounded_dep) {
        idl_file_out_printf(ctx, "#include %s\n", bounded_string_include);
      }
      if (util_depencencies & idl_string_unbounded_dep) {
        idl_file_out_printf(ctx, "#include %s\n", string_include);
      }
    }
    else
    {
      if (util_depencencies & (idl_string_bounded_dep | idl_string_unbounded_dep)) {
        idl_file_out_printf(ctx, "#include %s\n", string_include);
      }
    }
    if (util_depencencies & idl_variant_dep) {
      idl_file_out_printf(ctx, "#include %s\n", union_include);
    }
    if (util_depencencies & idl_array_dep) {
      idl_file_out_printf(ctx, "#include %s\n", array_include);
    }
    idl_file_out_printf(ctx, "\n");
  }

  return ret;
}

idl_retcode_t
idl_backendGenerateType(idl_backend_ctx ctx, const idl_pstate_t *parse_tree)
{
  /* If input comes from a file, generate appropriate include statements. */
  if (parse_tree->files) idl_generate_include_statements(ctx, parse_tree);

  idl_retcode_t ret = IDL_RETCODE_OK;
  idl_visitor_t visitor;

  memset(&visitor, 0, sizeof(visitor));
  visitor.visit = IDL_CONST | IDL_TYPEDEF | IDL_STRUCT | IDL_MODULE | IDL_ENUM | IDL_FORWARD | IDL_UNION;
  visitor.accept[IDL_ACCEPT_CONST] = &emit_const;
  visitor.accept[IDL_ACCEPT_FORWARD] = &emit_forward;
  visitor.accept[IDL_ACCEPT_TYPEDEF] = &emit_typedef;
  visitor.accept[IDL_ACCEPT_STRUCT] = &emit_struct;
  visitor.accept[IDL_ACCEPT_UNION] = &emit_union;
  visitor.accept[IDL_ACCEPT_ENUM] = &emit_enum;
  visitor.accept[IDL_ACCEPT_MODULE] = &emit_module;
  if (parse_tree->sources)
    visitor.glob = parse_tree->sources->path->name;
  ret = idl_visit(parse_tree, parse_tree->root, &visitor, ctx);

  return ret;
}

