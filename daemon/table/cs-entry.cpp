#include <iostream>

#include <assert.h>

#include "cs-entry.hpp"

namespace nfd {
namespace cs {

Entry::Entry(const Name& name) : m_name(name){
    BOOST_ASSERT(this->isQuery());
}

Entry::Entry(shared_ptr<const Data> data) : 
    m_name(data->getName()),
    m_freshnessPeriod(data->getFreshnessPeriod()),
    m_signature(data->getSignature())
{
    insert(data);
    BOOST_ASSERT(!this->isQuery());
}

bool Entry::operator<(const Entry& other) const {
    return m_name < other.getName();
}

void Entry::insert(shared_ptr<const Data> data){
    BOOST_ASSERT(!this->isQuery());

    uint32_t index = data->getContentIndex();
    uint16_t len = data->getContentLength();
    const ndn::Block & content = data->getContent();
    assert(content.value_size() == len);
    std::string dataContent(reinterpret_cast<const char*>(content.value()), content.value_size()); // fix: no copy

    Range range(index, index + len - 1);
    auto iter = m_contentMap.lower_bound(range);
    if(iter != m_contentMap.end() && iter->first.can_merge(range)){
        merge(iter, range, dataContent);
        adjust();
    }
    else{
        if(m_contentMap.empty()){
            m_contentMap.insert({range, dataContent});
            return;
        }
        
        -- iter;
        if(iter->first.can_merge(range)){
            merge(iter, range, dataContent);
            adjust();
        }
        else{
            m_contentMap.insert({range, dataContent});
        }
    }
}

Data* Entry::match(uint32_t index, uint16_t len) {
	if (m_contentMap.empty()){
		return nullptr;
	}

    Range range(index, index + len - 1);
    auto iter = m_contentMap.lower_bound(range);
	if(m_contentMap.size() == 1 && iter != m_contentMap.end()){
		return nullptr;
	}

    --iter;
	if(!iter->first.contain(range)){
		return nullptr;
	}

    Data * data = new Data(m_name);
    data->setFreshnessPeriod(m_freshnessPeriod);
    size_t pos = index - iter->first.get_lowerBound();
    data->setContent(reinterpret_cast<const uint8_t *>(&iter->second.at(pos)), len);
    data->setSignature(m_signature); // fix?

    return data;
}

void Entry::merge(CIter citer, Range range, const std::string& content){
    if(citer->first.contain(range)){
        return;
    }
    else{
        Range mergeRange = citer->first.merge(range);
        std::string mergeContent = mergeRange.get_lowerBound() == citer->first.get_lowerBound() ? 
            merge_content(mergeRange, citer->first, citer->second, content) : 
            merge_content(mergeRange, range, content, citer->second);
        m_contentMap.insert({mergeRange, mergeContent});

        m_contentMap.erase(citer);
    }
}

void Entry::merge(CIter citer1, CIter citer2){
    merge(citer1, citer2->first, citer2->second);
    m_contentMap.erase(citer2);
}

std::string Entry::merge_content(Range mergeRange, Range range, const std::string& firstContent, const std::string& secondContent){
    std::string mergeContent(std::move(firstContent));
    size_t len = mergeRange.get_upperBound() - range.get_upperBound();
    if(len > 0){
        mergeContent.append(secondContent, secondContent.size()-len, len);
    }

    assert(mergeContent.size() == mergeRange.length());
    return mergeContent;
}

void Entry::adjust(){
label:
    for(auto iter = m_contentMap.begin(); iter != m_contentMap.end(); ){
        auto curIter = iter;
        auto nextIter = ++ iter;
        if(iter == m_contentMap.end()){
            return;
        }

        if(curIter->first.can_merge(nextIter->first)){
            merge(curIter, nextIter);
            goto label;
        }
    }
}

bool isQuery() const {
    return m_contentMap.empty();
}

} // cs
} // nfd