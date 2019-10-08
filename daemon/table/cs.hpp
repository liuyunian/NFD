/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef NFD_DAEMON_TABLE_CS_HPP
#define NFD_DAEMON_TABLE_CS_HPP

#include "cs-policy.hpp"
#include "cs-internal.hpp"
#include "cs-entry.hpp"

#include <boost/iterator/transform_iterator.hpp>

namespace nfd {
namespace cs {

/** \brief implements the Content Store -- fix
 *
 *  This Content Store implementation consists of a Table and a replacement policy.
 *
 *  The Table is a container ( \c std::set ) sorted by full Names of stored Data packets.
 *  Data packets are wrapped in Entry objects. Each Entry contains the Data packet itself,
 *  and a few additional attributes such as when the Data becomes non-fresh.
 *
 *  The replacement policy is implemented in a subclass of \c Policy.
 */
class Cs : noncopyable
{
public:
//   explicit
//   Cs(size_t nMaxPackets = 10);
    explicit Cs(size_t nMaxItems = 10);

  /** \brief inserts a Data packet
   */
  void
  insert(const Data& data, bool isUnsolicited = false);

  using AfterEraseCallback = std::function<void(size_t nErased)>;

  /** \brief asynchronously erases entries under \p prefix
   *  \param prefix name prefix of entries
   *  \param limit max number of entries to erase
   *  \param cb callback to receive the actual number of erased entries; it may be empty;
   *            it may be invoked either before or after erase() returns
   */
  void
  erase(const Name& prefix, size_t limit, const AfterEraseCallback& cb);

  using HitCallback = std::function<void(const Interest&, const Data&)>;
  using MissCallback = std::function<void(const Interest&)>;

  /** \brief finds the best matching Data packet
   *  \param interest the Interest for lookup
   *  \param hitCallback a callback if a match is found; must not be empty
   *  \param missCallback a callback if there's no match; must not be empty
   *  \note A lookup invokes either callback exactly once.
   *        The callback may be invoked either before or after find() returns
   */
  void
  find(const Interest& interest,
       const HitCallback& hitCallback,
       const MissCallback& missCallback) const;

  /** \brief get number of stored packets
   */
  size_t
  size() const
  {
    return m_table.size();
  }

public: // configuration
  /** \brief get capacity (in number of packets)
   */
  size_t
  getLimit() const
  {
    return m_policy->getLimit();
  }

  /** \brief change capacity (in number of packets)
   */
//   void
//   setLimit(size_t nMaxPackets)
//   {
//     return m_policy->setLimit(nMaxPackets);
//   }

    void
    setLimit(size_t nMaxItems)
    {
        return m_policy->setLimit(nMaxItems);
    }

  /** \brief get replacement policy
   */
  Policy*
  getPolicy() const
  {
    return m_policy.get();
  }

  /** \brief change replacement policy
   *  \pre size() == 0
   */
  void
  setPolicy(unique_ptr<Policy> policy);

  /** \brief get CS_ENABLE_ADMIT flag
   *  \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
   */
  bool
  shouldAdmit() const
  {
    return m_shouldAdmit;
  }

  /** \brief set CS_ENABLE_ADMIT flag
   *  \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
   */
  void
  enableAdmit(bool shouldAdmit);

  /** \brief get CS_ENABLE_SERVE flag
   *  \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
   */
  bool
  shouldServe() const
  {
    return m_shouldServe;
  }

  /** \brief set CS_ENABLE_SERVE flag
   *  \sa https://redmine.named-data.net/projects/nfd/wiki/CsMgmt#Update-config
   */
  void
  enableServe(bool shouldServe);

public: // enumeration
  struct EntryFromEntryImpl
  {
    typedef const Entry& result_type;

    const Entry&
    operator()(const Entry& entry) const // operator()(const EntryImpl& entry) const
    {
      return entry;
    }
  };

  /** \brief ContentStore iterator (public API)
   */
  typedef boost::transform_iterator<EntryFromEntryImpl, iterator, const Entry&> const_iterator;

  const_iterator
  begin() const
  {
    return boost::make_transform_iterator(m_table.begin(), EntryFromEntryImpl());
  }

  const_iterator
  end() const
  {
    return boost::make_transform_iterator(m_table.end(), EntryFromEntryImpl());
  }

private: // find
  /** \brief find leftmost match in [first,last)
   *  \return the leftmost match, or last if not found
   */
//   iterator
//   findLeftmost(const Interest& interest, iterator left, iterator right) const;

  /** \brief find rightmost match in [first,last)
   *  \return the rightmost match, or last if not found
   */
//   iterator
//   findRightmost(const Interest& interest, iterator first, iterator last) const;

  /** \brief find rightmost match among entries with exact Names in [first,last)
   *  \return the rightmost match, or last if not found
   */
//   iterator
//   findRightmostAmongExact(const Interest& interest, iterator first, iterator last) const;

  void
  setPolicyImpl(unique_ptr<Policy> policy);

// PUBLIC_WITH_TESTS_ELSE_PRIVATE:
//   void
//   dump();

private:
  Table m_table;
  unique_ptr<Policy> m_policy;
  signal::ScopedConnection m_beforeEvictConnection;

  bool m_shouldAdmit = true; ///< if false, no Data will be admitted
  bool m_shouldServe = true; ///< if false, all lookups will miss
};

} // namespace cs

using cs::Cs;

} // namespace nfd

#endif // NFD_DAEMON_TABLE_CS_HPP
