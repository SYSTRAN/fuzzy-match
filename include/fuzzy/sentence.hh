#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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