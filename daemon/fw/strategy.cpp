/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "strategy.hpp"
#include "forwarder.hpp"
#include "common/logger.hpp"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace nfd {
namespace fw {

NFD_LOG_INIT(Strategy);

Strategy::Registry&
Strategy::getRegistry()
{
  static Registry registry;
  return registry;
}

Strategy::Registry::const_iterator
Strategy::find(const Name& instanceName)
{
  const Registry& registry = getRegistry();
  ParsedInstanceName parsed = parseInstanceName(instanceName);

  if (parsed.version) {
    // specified version: find exact or next higher version

    auto found = registry.lower_bound(parsed.strategyName);
    if (found != registry.end()) {
      if (parsed.strategyName.getPrefix(-1).isPrefixOf(found->first)) {
        NFD_LOG_TRACE("find " << instanceName << " versioned found=" << found->first);
        return found;
      }
    }

    NFD_LOG_TRACE("find " << instanceName << " versioned not-found");
    return registry.end();
  }

  // no version specified: find highest version

  if (!parsed.strategyName.empty()) { // Name().getSuccessor() would be invalid
    auto found = registry.lower_bound(parsed.strategyName.getSuccessor());
    if (found != registry.begin()) {
      --found;
      if (parsed.strategyName.isPrefixOf(found->first)) {
        NFD_LOG_TRACE("find " << instanceName << " unversioned found=" << found->first);
        return found;
      }
    }
  }

  NFD_LOG_TRACE("find " << instanceName << " unversioned not-found");
  return registry.end();
}

bool
Strategy::canCreate(const Name& instanceName)
{
  return Strategy::find(instanceName) != getRegistry().end();
}

unique_ptr<Strategy>
Strategy::create(const Name& instanceName, Forwarder& forwarder)
{
  auto found = Strategy::find(instanceName);
  if (found == getRegistry().end()) {
    NFD_LOG_DEBUG("create " << instanceName << " not-found");
    return nullptr;
  }

  unique_ptr<Strategy> instance = found->second(forwarder, instanceName);
  NFD_LOG_DEBUG("create " << instanceName << " found=" << found->first
                << " created=" << instance->getInstanceName());
  BOOST_ASSERT(!instance->getInstanceName().empty());
  return instance;
}

bool
Strategy::areSameType(const Name& instanceNameA, const Name& instanceNameB)
{
  return Strategy::find(instanceNameA) == Strategy::find(instanceNameB);
}

std::set<Name>
Strategy::listRegistered()
{
  std::set<Name> strategyNames;
  boost::copy(getRegistry() | boost::adaptors::map_keys,
              std::inserter(strategyNames, strategyNames.end()));
  return strategyNames;
}

Strategy::ParsedInstanceName
Strategy::parseInstanceName(const Name& input)
{
  for (ssize_t i = input.size() - 1; i > 0; --i) {
    if (input[i].isVersion()) {
      return {input.getPrefix(i + 1), input[i].toVersion(), input.getSubName(i + 1)};
    }
  }
  return {input, nullopt, PartialName()};
}

Name
Strategy::makeInstanceName(const Name& input, const Name& strategyName)
{
  BOOST_ASSERT(strategyName.at(-1).isVersion());

  bool hasVersion = std::any_of(input.rbegin(), input.rend(),
                                [] (const name::Component& comp) { return comp.isVersion(); });
  return hasVersion ? input : Name(input).append(strategyName.at(-1));
}

Strategy::Strategy(Forwarder& forwarder)
  : afterAddFace(forwarder.getFaceTable().afterAdd)
  , beforeRemoveFace(forwarder.getFaceTable().beforeRemove)
  , m_forwarder(forwarder)
  , m_measurements(m_forwarder.getMeasurements(), m_forwarder.getStrategyChoice(), *this)
{
}

Strategy::~Strategy() = default;

void
Strategy::beforeSatisfyInterest(const shared_ptr<pit::Entry>& pitEntry,
                                const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("beforeSatisfyInterest pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());
}

void
Strategy::afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
                               const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("afterContentStoreHit pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());

  this->sendData(pitEntry, data, ingress);
}

void
Strategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                           const FaceEndpoint& ingress, const Data& data)
{
  NFD_LOG_DEBUG("afterReceiveData pitEntry=" << pitEntry->getName()
                << " in=" << ingress << " data=" << data.getName());

  this->beforeSatisfyInterest(pitEntry, ingress, data);

  this->sendDataToAll(pitEntry, ingress, data);
}

void
Strategy::afterReceiveNack(const FaceEndpoint& ingress, const lp::Nack& nack,
                           const shared_ptr<pit::Entry>& pitEntry)
{
  NFD_LOG_DEBUG("afterReceiveNack in=" << ingress << " pitEntry=" << pitEntry->getName());
}

void
Strategy::onDroppedInterest(const FaceEndpoint& egress, const Interest& interest)
{
  NFD_LOG_DEBUG("onDroppedInterest out=" << egress << " name=" << interest.getName());
}

void
Strategy::sendData(const shared_ptr<pit::Entry>& pitEntry, const Data& data,
                   const FaceEndpoint& egress)
{
  BOOST_ASSERT(pitEntry->getInterest().matchesData(data));

  // delete the PIT entry's in-record based on egress,
  // since Data is sent to face and endpoint from which the Interest was received
  pitEntry->deleteInRecord(egress.face, egress.endpoint);

  m_forwarder.onOutgoingData(data, egress);
}

void
Strategy::sendDataToAll(const shared_ptr<pit::Entry>& pitEntry,
                        const FaceEndpoint& ingress, const Data& data)
{
//   std::set<std::pair<Face*, EndpointId>> pendingDownstreams;
//   auto now = time::steady_clock::now();

//   // remember pending downstreams
//   for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
//     if (inRecord.getExpiry() > now) {
//       if (inRecord.getFace().getId() == ingress.face.getId() &&
//           inRecord.getEndpointId() == ingress.endpoint &&
//           inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
//         continue;
//       }

//       pendingDownstreams.emplace(&inRecord.getFace(), inRecord.getEndpointId());
//     }
//   }

//   for (const auto& pendingDownstream : pendingDownstreams) {
//     this->sendData(pitEntry, data, FaceEndpoint(*pendingDownstream.first, pendingDownstream.second));
//   }

  std::set<std::tuple<Face*, EndpointId, std::shared_ptr<const Interest>>> pendingDownstreams;
  auto now = time::steady_clock::now();

  // remember pending downstreams
  for (const pit::InRecord& inRecord : pitEntry->getInRecords()) {
    if (inRecord.getExpiry() > now) {
      if (inRecord.getFace().getId() == ingress.face.getId() &&
          inRecord.getEndpointId() == ingress.endpoint &&
          inRecord.getFace().getLinkType() != ndn::nfd::LINK_TYPE_AD_HOC) {
        continue;
      }
    
      pendingDownstreams.emplace(&inRecord.getFace(), inRecord.getEndpointId(), inRecord.getInterest().shared_from_this());
    }
  }

  for (const auto& pendingDownstream : pendingDownstreams) {
    auto interest = std::get<2>(pendingDownstream);
    if(!interest->matchesData(data)){ // can't match index & length
        uint32_t index = interest->getContentIndex();
        uint16_t length = interest->getContentLength();

        Data tempData(data);
        tempData.setContentIndex(index);
        tempData.setContentLength(length);

        size_t offset = index - data.getContentIndex();
        const ndn::Block& content = data.getContent();
        tempData.setContent(content.value() + offset, static_cast<size_t>(length));

        this->sendData(pitEntry, tempData, FaceEndpoint(*std::get<0>(pendingDownstream), std::get<1>(pendingDownstream)));
    }
    else{
        this->sendData(pitEntry, data, FaceEndpoint(*std::get<0>(pendingDownstream), std::get<1>(pendingDownstream)));
    }
  }
}

void
Strategy::sendNacks(const shared_ptr<pit::Entry>& pitEntry, const lp::NackHeader& header,
                    std::initializer_list<FaceEndpoint> exceptFaceEndpoints)
{
  // populate downstreams with all downstreams faces
  std::set<std::pair<Face*, EndpointId>> downstreams;
  std::transform(pitEntry->in_begin(), pitEntry->in_end(), std::inserter(downstreams, downstreams.end()),
                 [] (const pit::InRecord& inR) {
                  return std::make_pair(&inR.getFace(), inR.getEndpointId());
                 });

  // delete excluded faces
  for (const auto& exceptFaceEndpoint : exceptFaceEndpoints) {
    downstreams.erase({&exceptFaceEndpoint.face, exceptFaceEndpoint.endpoint});
  }

  // send Nacks
  for (const auto& downstream : downstreams) {
    this->sendNack(pitEntry, FaceEndpoint(*downstream.first, downstream.second), header);
  }
  // warning: don't loop on pitEntry->getInRecords(), because in-record is deleted when sending Nack
}

const fib::Entry&
Strategy::lookupFib(const pit::Entry& pitEntry) const
{
  const Fib& fib = m_forwarder.getFib();

  const Interest& interest = pitEntry.getInterest();
  // has forwarding hint?
  if (interest.getForwardingHint().empty()) {
    // FIB lookup with Interest name
    const fib::Entry& fibEntry = fib.findLongestPrefixMatch(pitEntry);
    NFD_LOG_TRACE("lookupFib noForwardingHint found=" << fibEntry.getPrefix());
    return fibEntry;
  }

  const DelegationList& fh = interest.getForwardingHint();
  // Forwarding hint should have been stripped by incoming Interest pipeline when reaching producer region
  BOOST_ASSERT(!m_forwarder.getNetworkRegionTable().isInProducerRegion(fh));

  const fib::Entry* fibEntry = nullptr;
  for (const Delegation& del : fh) {
    fibEntry = &fib.findLongestPrefixMatch(del.name);
    if (fibEntry->hasNextHops()) {
      if (fibEntry->getPrefix().size() == 0) {
        // in consumer region, return the default route
        NFD_LOG_TRACE("lookupFib inConsumerRegion found=" << fibEntry->getPrefix());
      }
      else {
        // in default-free zone, use the first delegation that finds a FIB entry
        NFD_LOG_TRACE("lookupFib delegation=" << del.name << " found=" << fibEntry->getPrefix());
      }
      return *fibEntry;
    }
    BOOST_ASSERT(fibEntry->getPrefix().size() == 0); // only ndn:/ FIB entry can have zero nexthop
  }
  BOOST_ASSERT(fibEntry != nullptr && fibEntry->getPrefix().size() == 0);
  return *fibEntry; // only occurs if no delegation finds a FIB nexthop
}

} // namespace fw
} // namespace nfd
