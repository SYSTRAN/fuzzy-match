#pragma once


namespace fuzzy {
    struct PairHasher {
        std::size_t operator()(const std::pair<int, int>& p) const {
            const long val = static_cast<long>(p.first) << 32 | p.second;
            std::hash<long> hash_fn;
            return hash_fn(val);
        }
    };
}