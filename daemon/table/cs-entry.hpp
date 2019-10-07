#ifndef CSENTRY_H_
#define CSENTRY_H_

#include <map>
#include <string>

#include "cs-range.hpp"

namespace nfd {
namespace cs {

class Entry {
public:
    Entry() = default;
    ~Entry() = default;

    void insert(uint32_t index, uint16_t len, std::string content);

    // for test
    void print();

private:
    typedef std::map<Range, std::string>::const_iterator CIter;

    // typedef std::pair<Range, std::string> ContentEntry;

    void merge(CIter citer, Range range, const std::string& content);

    void merge(CIter citer1, CIter citer2);

    std::string merge_content(Range mergeRange, Range range, const std::string& firstContent, const std::string& secondContent);

    void adjust();

private:
    std::map<Range, std::string> m_contentMap;
};

} // cs
} // nfd

#endif // CSENTRY_H_