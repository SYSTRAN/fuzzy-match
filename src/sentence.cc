#include <fuzzy/sentence.hh>

#include <boost/algorithm/string/join.hpp>

namespace fuzzy
{
  static std::vector<std::string> split_string(const std::string& str, const char token) {
    std::vector<std::string> parts;
    parts.reserve(1 + std::count(str.begin(), str.end(), token));
    std::string accu;
    for (const auto c : str) {
      if (c == token) {
        parts.emplace_back(std::move(accu));
        accu.clear();
      } else {
        accu += c;
      }
    }
    if (!accu.empty())
      parts.emplace_back(std::move(accu));
    return parts;
  }

  Sentence::Sentence(const Tokens &s):_tokstring(boost::algorithm::join(s, "\t")) {
  }

  void Sentence::get_itoks(std::vector<const char*>& st, std::vector<int>& sn) const {
    for (const auto& pair : _itoks) {
      const size_t idx = pair.first;
      const std::string& token = pair.second;
      sn[idx] = token.length();
      st[idx] = token.c_str();
    }
  }

  Sentence::operator Tokens() const {
    return split_string(_tokstring, '\t');
  } 
}
