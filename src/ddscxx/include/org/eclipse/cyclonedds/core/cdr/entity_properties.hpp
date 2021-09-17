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
#ifndef ENTITY_PROPERTIES_HPP_
#define ENTITY_PROPERTIES_HPP_

#include <dds/core/macros.hpp>
#include <cstdint>
#include <list>

namespace org {
namespace eclipse {
namespace cyclonedds {
namespace core {
namespace cdr {

/**
 * @brief
 * Bit bound descriptors.
 *
 * @enum bit_bound Describes the minimal bit width for enum and bitmask types.
 *
 * This value is unset for anything other than enums and bitmasks.
 * It describes the smallest piece of memory which is able to represent the entire range of values.
 *
 * @var bit_bound::bb_unset The bit width of the entity is unset.
 * @var bit_bound::bb_8_bits The bit width of the entity is at most 8 bits (1 byte).
 * @var bit_bound::bb_16_bits The bit width of the entity is at most 16 bits (2 bytes).
 * @var bit_bound::bb_32_bits The bit width of the entity is at most 32 bits (4 bytes).
 * @var bit_bound::bb_64_bits The bit width of the entity is at most 64 bits (8 bytes).
 */
enum bit_bound {
  bb_unset = 0,
  bb_8_bits = 1,
  bb_16_bits = 2,
  bb_32_bits = 4,
  bb_64_bits = 8
};

/**
 * @brief
 * Entity extensibility descriptors.
 *
 * @enum extensibility Describes the extensibility of entities.
 *
 * This value is set for entities and their parents.
 *
 * @var extensibility::ext_final The entity representation is complete, no fields can be added or removed.
 * @var extensibility::ext_appendable The entity representation can be extended, no fields can be removed.
 * @var extensibility::ext_mutable The entity representation can be modified, fields can be removed or added.
 */
enum extensibility {
  ext_final,
  ext_appendable,
  ext_mutable
};

typedef struct entity_properties entity_properties_t;
typedef std::list<entity_properties_t> proplist;


/**
 * @brief
 * Entity properties struct.
 *
 * This is a container for data fields inside message classes, both as a representation passed for writing
 * as well as headers taken from streams when reading in the appropriate manner.
 * Normally these are not used by the end-user, and the streaming functions all interact with these objects
 * through the get_type_props function, which is generated for all user supplied message types.
 */
struct OMG_DDS_API entity_properties
{
  entity_properties(
    uint32_t _m_id = 0,
    bool _is_optional = false):
      m_id(_m_id),
      is_optional(_is_optional) {;}

  extensibility e_ext = ext_final; /**< The extensibility of the entity itself. */
  extensibility p_ext = ext_final; /**< The extensibility of the entity's parent. */
  size_t e_off = 0; /**< The current offset in the stream at which the member field starts, does not include header. */
  size_t d_off = 0; /**< The current offset in the stream at which the struct starts, does not include header.*/
  uint32_t e_sz = 0; /**< The size of the current entity as member field (only used in reading from streams).*/
  uint32_t d_sz = 0; /**< The size of the current entity as struct (only used in reading from streams).*/
  uint32_t m_id = 0; /**< The member id of the entity, it is the global field by which the entity is identified. */
  bool must_understand = false; /**< If the reading end cannot parse a field with this header, it must discard the entire object. */
  bool implementation_extension = false; /**< ???TODO??? */
  bool keylist_is_pragma = false; /**< Indicates whether the keylist is #pragma*/
  bool is_last = false; /**< Indicates terminating entry for reading/writing entities, will cause the current subroutine to end and decrement the stack.*/
  bool ignore = false; /**< Indicates that this field must be ignored.*/
  bool is_optional = false; /**< Indicates that this field can be empty (length 0) for reading/writing purposes.*/
  bit_bound e_bb = bb_unset; /**< The minimum number of bytes necessary to represent this entity/bitmask.*/

  DDSCXX_WARNING_MSVC_OFF(4251)
  proplist m_members_by_seq; /**< Fields in normal streaming mode, ordered by their declaration.*/
  proplist m_keys_by_seq; /**< Fields in key streaming mode, ordered by their declaration.*/
  proplist m_members_by_id; /**< Fields in normal streaming mode, ordered by their member id.*/
  proplist m_keys_by_id; /**< Fields in key streaming mode, ordered by their member id.*/
  DDSCXX_WARNING_MSVC_ON(4251)

  /**
   * @brief
   * Conversion to boolean operator.
   *
   * Checks whether the is_last flag is NOT set.
   * Exists to make iterating over lists of entity properties easier, as the last entry of a list should be
   * the one that converts to 'false', and the rest are all 'true'.
   */
  operator bool() const {return !is_last;}

  /**
   * @brief
   * Comparison operator.
   *
   * @param[in] other The other entity to compare to.
   *
   * @return True when member and sequence ids are the same.
   */
  bool operator==(const entity_properties_t &other) const {return m_id == other.m_id;}

  /**
   * @brief
   * Comparison function.
   *
   * Sorts by is_final and m_id, in that precedence.
   * Used in sorting lists of entity_properties by member id, which makes lookup of the entity
   * when receiving member id fields much quicker.
   *
   * @param[in] lhs First entity to be compared.
   * @param[in] rhs Second entity to be compared.
   *
   * @return Whether lhs should be sorted before rhs.
   */
  static bool member_id_comp(const entity_properties_t &lhs, const entity_properties_t &rhs);

  /**
   * @brief
   * Finishing function.
   *
   * Calls populate_empty_keys() and sort_by_member_id().
   * Calls itself recursively on all property lists contained within, setting at_root to false.
   *
   * @param[in] at_root Whether this function is called on the entity itself, or a member of another entity.
   */
  void finish(bool at_root = true);

  /**
   * @brief
   * Member property setting function.
   *
   * Sets the m_id and is_optional values.
   * Created to not have to have a constructor with a prohibitively large number of parameters.
   *
   * @param[in] member_id Sets the m_id field.
   * @param[in] optional Sets the is_optional field.
   */
  void set_member_props(uint32_t member_id, bool optional);

  /**
   * @brief
   * Entity printing function.
   *
   * Prints the following information of the entity:
   * - m_id, s_id, (key)member status, which ordering is followed: sequence(declaration)/id
   * - whether it is a list terminating entry
   * - the extensibility of the parent and the entity itself
   *
   * @param[in] recurse Whether to print its own children.
   * @param[in] depth At which depth we are printing, determining the indentation at which is printed.
   * @param[in] prefix Which prefix preceeds the printed entity information.
   */
  void print(bool recurse = true, size_t depth = 0, const char *prefix = "") const;
private:

  /**
   * @brief
   * Entity property list sorting function.
   *
   * Sorts the contents of a list and merges duplicate entries.
   *
   * @param[in] in The list to sort.
   *
   * @return The sorted contents of in.
   */
  static proplist sort_proplist(const proplist &in);

  /**
   * @brief
   * Creates member ordered lists from the sequence ordered lists.
   */
  void sort_by_member_id();

  /**
   * @brief
   * Finishes the list of key entries.
   *
   * This function is preparation for the sort_by_member_id function.
   * For non-root structures, if the key list is empty (only containing a final entry or none), the key
   * list is populated with the member list.
   * All key list entries are set to must_understand is true, and their (parent)extensibility set to final.
   *
   * @param[in] at_root Whether the current call is being done on the entity itself or one of its members.
   */
  void finish_keys(bool at_root);

  /**
   * @brief
   * Merges the contents of one entity into this one.
   *
   * The contents of the member and key list are appended to their counterparts in this object.
   * This function should be followed by a call to sort_by_member_id to remove duplicate entries.
   *
   * @param[in] other The entity whose contents are to be merged into this one.
   */
  void merge(const entity_properties_t &other);

  /**
   * @brief
   * Sets the must_understand flags from key list on member lists.
   *
   * Takes entities in keys_by_id and sets the must_understand flag on the matching entities in
   * members_by_seq and members_by_id. Then calls this function on the sublists for the matching
   * entities.
   *
   * @param[in] keys_by_id The list of keys to match in member lists.
   * @param[in, out] members_by_seq The list members by sequence id to set the must understand flag on.
   * @param[in, out] members_by_id The list members by member id to set the must understand flag on.
   */
  void copy_must_understand(
    const proplist &keys_by_id,
    proplist &members_by_seq,
    proplist &members_by_id);
};

struct OMG_DDS_API final_entry: public entity_properties_t {
  final_entry(): entity_properties_t() {
    is_last = true;
  }
};

template<typename T>
entity_properties_t& get_type_props() {
  thread_local static bool initialized = false;
  thread_local static entity_properties_t props;
  if (!initialized) {
    switch (sizeof(T)) {
      case 1:
        props.e_bb = bb_8_bits;
        break;
      case 2:
        props.e_bb = bb_16_bits;
        break;
      case 4:
        props.e_bb = bb_32_bits;
        break;
      case 8:
        props.e_bb = bb_64_bits;
        break;
    }
    props.m_members_by_seq.push_back(final_entry());
    props.m_keys_by_seq.push_back(final_entry());
    props.m_members_by_id.push_back(final_entry());
    props.m_keys_by_id.push_back(final_entry());
    initialized = true;
  }
  return props;
}

}
}
}
}
}  /* namespace org / eclipse / cyclonedds / core / cdr */

#endif
