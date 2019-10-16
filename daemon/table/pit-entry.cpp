/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "pit-entry.hpp"
#include "content-range.hpp"

#include <algorithm>

namespace nfd {
namespace pit {

Entry::Entry(const Interest& interest)
  : m_interest(interest.shared_from_this())
{
}

bool
Entry::canMatch(const Interest& interest, size_t nEqualNameComps) const
{
  BOOST_ASSERT(m_interest->getName().compare(0, nEqualNameComps,
                                             interest.getName(), 0, nEqualNameComps) == 0);

    // return m_interest->getName().compare(nEqualNameComps, Name::npos, interest.getName(), nEqualNameComps) == 0 &&
    //     m_interest->getSelectors() == interest.getSelectors();
    /// \todo #3162 match Link field

    ContentRange range_self(m_interest->getContentIndex(), m_interest->getContentLength());
    ContentRange range_other(interest.getContentIndex(), interest.getContentLength());

    return m_interest->getName().compare(nEqualNameComps, Name::npos, interest.getName(), nEqualNameComps) == 0 &&
        m_interest->getSelectors() == interest.getSelectors() && 
        range_self.contain(range_other);
}

InRecordCollection::iterator
Entry::getInRecord(const Face& face, EndpointId endpointId)
{
  return std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face, endpointId] (const InRecord& inRecord) {
      return &inRecord.getFace() == &face && inRecord.getEndpointId() == endpointId;
    });
}

InRecordCollection::iterator
Entry::insertOrUpdateInRecord(Face& face, EndpointId endpointId, const Interest& interest)
{
  BOOST_ASSERT(this->canMatch(interest));

  auto it = std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face, endpointId] (const InRecord& inRecord) {
      return &inRecord.getFace() == &face && inRecord.getEndpointId() == endpointId;
    });
  if (it == m_inRecords.end()) {
    m_inRecords.emplace_front(face, endpointId);
    it = m_inRecords.begin();
  }

  it->update(interest);
  return it;
}

void
Entry::deleteInRecord(const Face& face, EndpointId endpointId)
{
  auto it = std::find_if(m_inRecords.begin(), m_inRecords.end(),
    [&face, endpointId] (const InRecord& inRecord) {
      return &inRecord.getFace() == &face && inRecord.getEndpointId() == endpointId;
    });
  if (it != m_inRecords.end()) {
    m_inRecords.erase(it);
  }
}

void
Entry::clearInRecords()
{
  m_inRecords.clear();
}

OutRecordCollection::iterator
Entry::getOutRecord(const Face& face, EndpointId endpointId)
{
  return std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face, endpointId] (const OutRecord& outRecord) {
      return &outRecord.getFace() == &face && outRecord.getEndpointId() == endpointId;
    });
}

OutRecordCollection::iterator
Entry::insertOrUpdateOutRecord(Face& face, EndpointId endpointId, const Interest& interest)
{
  BOOST_ASSERT(this->canMatch(interest));

  auto it = std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face, endpointId] (const OutRecord& outRecord) {
      return &outRecord.getFace() == &face && outRecord.getEndpointId() == endpointId;
    });
  if (it == m_outRecords.end()) {
    m_outRecords.emplace_front(face, endpointId);
    it = m_outRecords.begin();
  }

  it->update(interest);
  return it;
}

void
Entry::deleteOutRecord(const Face& face, EndpointId endpointId)
{
  auto it = std::find_if(m_outRecords.begin(), m_outRecords.end(),
    [&face, endpointId] (const OutRecord& outRecord) {
      return &outRecord.getFace() == &face && outRecord.getEndpointId() == endpointId;
    });
  if (it != m_outRecords.end()) {
    m_outRecords.erase(it);
  }
}

void
Entry::deleteInOutRecordsByFace(const Face& face)
{
  m_inRecords.remove_if([&face] (const InRecord& inRecord) {
    return &inRecord.getFace() == &face;
  });
  m_outRecords.remove_if([&face] (const OutRecord& outRecord) {
    return &outRecord.getFace() == &face;
  });
}

} // namespace pit
} // namespace nfd
