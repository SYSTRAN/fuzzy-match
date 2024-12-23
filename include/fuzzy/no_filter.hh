#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <math.h> 
#include <Eigen/Sparse>

#include <fuzzy/utils.hh>
#include <fuzzy/filter.hh>

#include <boost/multi_array.hpp>
#include <boost/format.hpp>
#include <boost/container/vector.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/array.hpp>

namespace fuzzy
{
  class NoFilter : public Filter
  {
  public:
    NoFilter(const FilterIndexParams &params=FilterIndexParams());
    ~NoFilter();
    // unsigned add_sentence(const std::vector<unsigned>& sentence) override;
    using Filter::add_sentence;

    using Filter::dump;
    using Filter::num_sentences;
    using Filter::get_sentence;

    void prepare(size_t vocab_size);

    std::ostream& dump(std::ostream&) const;

    unsigned get_sentence_length(size_t s_id) const;

  private:
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive&, unsigned int version) const;

    template<class Archive>
    void load(Archive&, unsigned int version);

    BOOST_SERIALIZATION_SPLIT_MEMBER()
  };
}

BOOST_CLASS_VERSION(fuzzy::NoFilter, 1)

#include "fuzzy/no_filter.hxx"