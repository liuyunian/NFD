#include <iostream>

#include <assert.h>

#include "cs-entry.hpp"

namespace nfd {
namespace cs {

void Entry::insert(uint32_t index, uint16_t len, std::string content){
    assert(content.length() == len);

    Range range(index, index + len - 1);
    auto iter = m_contentMap.lower_bound(range);
    if(iter != m_contentMap.end() && iter->first.can_merge(range)){
        merge(iter, range, content);
        adjust();
    }
    else{
        if(m_contentMap.empty()){
            m_contentMap.insert({range, content});
            return;
        }
        
        -- iter;
        if(iter->first.can_merge(range)){
            merge(iter, range, content);
            adjust();
        }
        else{
            m_contentMap.insert({range, content});
        }
    }
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

// for test
void Entry::print(){
    std::cout << "============= contentMap ==============" << std::endl;
    for(auto & item : m_contentMap){
        std::cout << "range: [" << item.first.get_lowerBound() << ", " << item.first.get_upperBound() << "]" << std::endl;
        std::cout << "content: " << item.second << std::endl;
    }
}

} // cs
} // nfd