/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "cs.hpp"
#include "common/logger.hpp"
#include "core/algorithm.hpp"

#include <ndn-cxx/lp/tags.hpp>
#include <ndn-cxx/util/concepts.hpp>

namespace nfd {
namespace cs {

NDN_CXX_ASSERT_FORWARD_ITERATOR(Cs::const_iterator);

NFD_LOG_INIT(ContentStore);

// static unique_ptr<Policy>
// makeDefaultPolicy()
// {
//   return Policy::create("lru");
// }

Cs::Cs(size_t nMaxPackets)
{
//   setPolicyImpl(makeDefaultPolicy());
//   m_policy->setLimit(nMaxPackets);
}

void
Cs::insert(const Data& data, bool isUnsolicited)
{
//   if (!m_shouldAdmit || m_policy->getLimit() == 0) {
//     return;
//   }

  // recognize CachePolicy -- 确认缓存策略
//   shared_ptr<lp::CachePolicyTag> tag = data.getTag<lp::CachePolicyTag>();
//   if (tag != nullptr) {
//     lp::CachePolicyType policy = tag->get().getPolicy();
//     if (policy == lp::CachePolicyType::NO_CACHE) {
//       return;
//     }
//   }

//   iterator it;
//   bool isNewEntry = false;
//   std::tie(it, isNewEntry) = m_table.emplace(data.shared_from_this(), isUnsolicited);
//   EntryImpl& entry = const_cast<EntryImpl&>(*it);

//   entry.updateStaleTime();   -- 设置生存时间

//   if (!isNewEntry) { // existing entry
//     // XXX This doesn't forbid unsolicited Data from refreshing a solicited entry.
//     if (entry.isUnsolicited() && !isUnsolicited) {
//       entry.unsetUnsolicited();
//     }

//     m_policy->afterRefresh(it);
//   }
//   else {
//     m_policy->afterInsert(it);
//   }

    if(!m_shouldAdmit){
        return;
    }
    NFD_LOG_DEBUG("insert " << data.getName());

    bool isNewEntry = false;                                // for policy
    iterator iter = m_table.find(data.getName());                      
    if(iter == m_table.end()){
        std::tie(iter, isNewEntry) = m_table.emplace(data.shared_from_this);
        BOOST_ASSERT(isNewEntry == true);
    }
    else{
        iter->second.insert(data.shared_from_this);
    }

    // if (!isNewEntry) { // existing entry
}

void
Cs::erase(const Name& prefix, size_t limit, const AfterEraseCallback& cb)
{
//   BOOST_ASSERT(static_cast<bool>(cb));

//   iterator first = m_table.lower_bound(prefix);
//   iterator last = m_table.end();
//   if (prefix.size() > 0) {
//     last = m_table.lower_bound(prefix.getSuccessor());
//   }

//   size_t nErased = 0;
//   while (first != last && nErased < limit) {
//     m_policy->beforeErase(first);
//     first = m_table.erase(first);
//     ++nErased;
//   }

//   if (cb) {
//     cb(nErased);
//   }
}

void
Cs::find(const Interest& interest,
         const HitCallback& hitCallback,
         const MissCallback& missCallback) const
{
  BOOST_ASSERT(static_cast<bool>(hitCallback));
  BOOST_ASSERT(static_cast<bool>(missCallback));

//   if (!m_shouldServe || m_policy->getLimit() == 0) {
//     missCallback(interest);
//     return;
//   }

//   bool isRightmost = interest.getChildSelector() == 1;
//   NFD_LOG_DEBUG("find " << prefix << (isRightmost ? " R" : " L"));

//   iterator first = m_table.lower_bound(prefix);
//   iterator last = m_table.end();
//   if (prefix.size() > 0) {
//     last = m_table.lower_bound(prefix.getSuccessor());
//   }

//   iterator match = last;
//   if (isRightmost) {
//     match = this->findRightmost(interest, first, last);
//   }
//   else {
//     match = this->findLeftmost(interest, first, last);
//   }

//   if (match == last) {
//     NFD_LOG_DEBUG("  no-match");
//     missCallback(interest);
//     return;
//   }
//   NFD_LOG_DEBUG("  matching " << match->getName());
//   m_policy->beforeUse(match);
//   hitCallback(interest, match->getData());

    if(!m_shouldServe){
        missCallback(interest);
        return;
    }

    const Name& prefix = interest.getName();
    iterator it = m_table.find(prefix);
    if(it == m_table.end()){
        NFD_LOG_DEBUG("  no-match");
        missCallback(interest);
        return;
    }

    uint32_t index = interest.getContentIndex();
    uint16_t length = interest.getContentLength();

    Data* data = it->second.match(index, length);
    if(data == nullptr){
        NFD_LOG_DEBUG("  no-match");
        missCallback(interest);
        return;
    }

    NFD_LOG_DEBUG("  matching " << match->getName());
    hitCallback(interest, *data);
}

// iterator
// Cs::findLeftmost(const Interest& interest, iterator first, iterator last) const
// {
//   return std::find_if(first, last, [&interest] (const auto& entry) { return entry.canSatisfy(interest); });
// }

// iterator
// Cs::findRightmost(const Interest& interest, iterator first, iterator last) const
// {
//   // Each loop visits a sub-namespace under a prefix one component longer than Interest Name.
//   // If there is a match in that sub-namespace, the leftmost match is returned;
//   // otherwise, loop continues.

//   size_t interestNameLength = interest.getName().size();
//   for (iterator right = last; right != first;) {
//     iterator prev = std::prev(right);

//     // special case: [first,prev] have exact Names
//     if (prev->getName().size() == interestNameLength) {
//       NFD_LOG_TRACE("  find-among-exact " << prev->getName());
//       iterator matchExact = this->findRightmostAmongExact(interest, first, right);
//       return matchExact == right ? last : matchExact;
//     }

//     Name prefix = prev->getName().getPrefix(interestNameLength + 1);
//     iterator left = m_table.lower_bound(prefix);

//     // normal case: [left,right) are under one-component-longer prefix
//     NFD_LOG_TRACE("  find-under-prefix " << prefix);
//     iterator match = this->findLeftmost(interest, left, right);
//     if (match != right) {
//       return match;
//     }
//     right = left;
//   }
//   return last;
// }

// iterator
// Cs::findRightmostAmongExact(const Interest& interest, iterator first, iterator last) const
// {
//   return find_last_if(first, last, [&interest] (const auto& entry) { return entry.canSatisfy(interest); });
// }

// void
// Cs::dump()
// {
//   NFD_LOG_DEBUG("dump table");
//   for (const EntryImpl& entry : m_table) {
//     NFD_LOG_TRACE(entry.getFullName());
//   }
// }

// void
// Cs::setPolicy(unique_ptr<Policy> policy)
// {
//   BOOST_ASSERT(policy != nullptr);
//   BOOST_ASSERT(m_policy != nullptr);
//   size_t limit = m_policy->getLimit();
//   this->setPolicyImpl(std::move(policy));
//   m_policy->setLimit(limit);
// }

// void
// Cs::setPolicyImpl(unique_ptr<Policy> policy)
// {
//   NFD_LOG_DEBUG("set-policy " << policy->getName());
//   m_policy = std::move(policy);
//   m_beforeEvictConnection = m_policy->beforeEvict.connect([this] (iterator it) {
//       m_table.erase(it);
//     });

//   m_policy->setCs(this);
//   BOOST_ASSERT(m_policy->getCs() == this);
// }

void
Cs::enableAdmit(bool shouldAdmit)
{
  if (m_shouldAdmit == shouldAdmit) {
    return;
  }
  m_shouldAdmit = shouldAdmit;
  NFD_LOG_INFO((shouldAdmit ? "Enabling" : "Disabling") << " Data admittance");
}

void
Cs::enableServe(bool shouldServe)
{
  if (m_shouldServe == shouldServe) {
    return;
  }
  m_shouldServe = shouldServe;
  NFD_LOG_INFO((shouldServe ? "Enabling" : "Disabling") << " Data serving");
}

} // namespace cs
} // namespace nfd
