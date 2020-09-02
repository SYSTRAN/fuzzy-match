#pragma once

#include <string>
#include <fuzzy/fuzzy_match.hh>

namespace fuzzy
{
  void export_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, const FuzzyMatch& fuzzy_matcher);
  /// @throw std::exception if can't read file, or file is not an FMI
  void import_binarized_fuzzy_matcher(const std::string& binarized_tm_filename, FuzzyMatch& fuzzy_matcher);
}
