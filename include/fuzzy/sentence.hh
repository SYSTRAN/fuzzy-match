#pragma once

#include <string>
#include <vector>
#include <unordered_map>

//
// Even latest Debian Sid (March 2023) uses Boost 1.74 which does not behave well with very fresh compilers and triggers this error:
// https://github.com/pavel-odintsov/fastnetmon/issues/970
// This bug was fixed in fresh Boost versions: https://github.com/boostorg/serialization/issues/219 and we apply workaround only for 1.74
//

#include <boost/serialization/version.hpp>
#if BOOST_VERSION / 100000 == 1 && BOOST_VERSION / 100 % 1000 == 74
#include <boost/serialization/library_version_type.hpp>
#endif

#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>

namespace fuzzy
{
  typedef std::vector<std::string> Tokens;
  class Sentence {
  public:
    Sentence(const Tokens &s);
    Sentence();

    /* access operator to tokens */
    operator Tokens() const;
    const std::string &operator[](size_t) const;
    void push_back(const std::string &token);
    void reserve(size_t);
    size_t size() const;
    bool empty() const;

    /* push intermediate token - considered only through penalty token */
    void set_itok(size_t idx, const std::string &itok);
    void get_itoks(std::vector<const char*>& st, std::vector<int>& sn) const;
  private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version);

    std::string _tokstring;
    std::unordered_map<size_t, std::string> _itoks;
  };
}

#include "fuzzy/sentence.hxx"