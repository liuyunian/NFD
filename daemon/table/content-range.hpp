#ifndef RANGE_H_
#define RANGE_H_

#include <stdint.h>

namespace nfd {

class ContentRange{
public:
    ContentRange(uint32_t low, uint32_t up) : 
        m_lowerBound(low),
        m_upperBound(up){}

    ContentRange(uint32_t low, uint16_t len) : 
        m_lowerBound(low),
        m_upperBound(low + len - 1){}

    // 采用默认的析构、拷贝构造、赋值运算符函数

    inline uint32_t get_lowerBound() const {
        return m_lowerBound; 
    }

    inline uint32_t get_upperBound() const {
        return m_upperBound;
    }

    inline size_t length(){
        return m_upperBound - m_lowerBound + 1;
    }

    bool operator<(const ContentRange& other) const {
        return ((m_lowerBound < other.get_lowerBound()) 
            || (!(other.get_lowerBound() < m_lowerBound) && m_upperBound < other.get_upperBound()));
    }

    bool can_merge(const ContentRange& other) const {
        return !((other.get_lowerBound() > m_upperBound + 1) || (other.get_upperBound() < m_lowerBound - 1));
    }

    bool contain(const ContentRange& other) const {
        return m_lowerBound <= other.get_lowerBound() && m_upperBound >= other.get_upperBound();
    }

    ContentRange merge(const ContentRange& other) const {
        uint32_t lowerBound = m_lowerBound <= other.get_lowerBound() ? m_lowerBound : other.get_lowerBound();
        uint32_t upperBound = m_upperBound >= other.get_upperBound() ? m_upperBound : other.get_upperBound();

        return ContentRange(lowerBound, upperBound);
    }

private:
    uint32_t m_lowerBound;
    uint32_t m_upperBound;
};

} // nfd

#endif // RANGE_H_