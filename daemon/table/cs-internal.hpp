/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/** \file
 *  \brief declares ContentStore internal types
 */

#ifndef NFD_DAEMON_TABLE_CS_INTERNAL_HPP
#define NFD_DAEMON_TABLE_CS_INTERNAL_HPP

#include "core/common.hpp"

namespace nfd {
namespace cs {

class Entry;

typedef std::set<Entry> Table;
typedef Table::const_iterator iterator;

} // namespace cs
} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_INTERNAL_HPP
