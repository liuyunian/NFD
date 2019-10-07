#ifndef CSENTRY_H_
#define CSENTRY_H_

#include <map>
#include <string>

#include "cs-range.hpp"
#include "core/common.hpp"

namespace nfd {
namespace cs {

class Entry {
public:
    Entry(const Name& name);

    Entry(shared_ptr<const Data> data);

    ~Entry() = default;

    inline Name& getName(){
        return m_name;
    }

    void insert(shared_ptr<const Data> data);

    Data* match(uint32_t index, uint16_t len);

    bool operator<(const Entry& other) const;

private:
    typedef std::map<Range, std::string>::const_iterator CIter;

    // typedef std::pair<Range, std::string> ContentEntry;

    void merge(CIter citer, Range range, const std::string& content);

    void merge(CIter citer1, CIter citer2);

    std::string merge_content(Range mergeRange, Range range, const std::string& firstContent, const std::string& secondContent);

    void adjust();

    bool isQuery() const;

private:
    Name m_name;
    ndn::Signature m_signature;
    time::milliseconds m_freshnessPeriod;
    std::map<Range, std::string> m_contentMap;
};

} // cs
} // nfd

#endif // CSENTRY_H_