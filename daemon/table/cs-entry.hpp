#ifndef CSENTRY_H_
#define CSENTRY_H_

#include <map>
#include <string>

#include "content-range.hpp"
#include "core/common.hpp"

namespace nfd {
namespace cs {

class Entry {
public:
    Entry(const Name& name);

    explicit Entry(shared_ptr<const Data> data, bool isUnsolicited);

    ~Entry() = default;

    inline const Name& getName() const {
        return m_name;
    }

    inline bool isUnsolicited() const {
        BOOST_ASSERT(!this->isQuery());
        return m_isUnsolicited;
    }

    inline void unsetUnsolicited(){
        BOOST_ASSERT(!this->isQuery());
        m_isUnsolicited = false;
    }

    void insert(shared_ptr<const Data> data);

    shared_ptr<Data> match(uint32_t index, uint16_t len);

    bool operator<(const Entry& other) const;

private:
    typedef std::map<ContentRange, std::string>::const_iterator CIter;

    // typedef std::pair<ContentRange, std::string> ContentEntry;

    void merge(CIter citer, ContentRange range, const std::string& content);

    void merge(CIter citer1, CIter citer2);

    std::string merge_content(ContentRange mergeRange, ContentRange range, const std::string& firstContent, const std::string& secondContent);

    void adjust();

    bool isQuery() const;

private:
    Name m_name;
    bool m_isUnsolicited = false;
    ndn::Signature m_signature;
    time::milliseconds m_freshnessPeriod;
    std::map<ContentRange, std::string> m_contentMap;
};

} // cs
} // nfd

#endif // CSENTRY_H_