#ifndef SEYEON_RESOURCE_COMPILER_SRC_UTILS_H_
#define SEYEON_RESOURCE_COMPILER_SRC_UTILS_H_

#include <algorithm>
#include <string>
#include <cctype>
#include <string_view>
#include <sstream>
#include <functional>

// trim from start (in place)
inline void LTrimInplace(std::string& s) {
    s.erase(std::begin(s),
            std::find_if(std::begin(s), std::end(s),
                         [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
inline void RTrimInplace(std::string& s) {
    s.erase(std::find_if(std::rbegin(s), std::rend(s),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            std::end(s));
}

// trim from both ends (in place)
inline void TrimInplace(std::string& s) {
    RTrimInplace(s);
    LTrimInplace(s);
}

inline std::string LTrim(std::string s) {
    LTrimInplace(s);
    return s;
}

inline std::string RTrim(std::string s) {
    RTrimInplace(s);
    return s;
}

inline std::string Trim(std::string s) {
    TrimInplace(s);
    return s;
}

inline std::string ReplaceAll(std::string_view template_str,
                              std::string_view pattern,
                              std::string_view replace_str) {
    std::stringstream ss;
    std::boyer_moore_searcher searcher{std::begin(pattern), std::end(pattern)};

    auto it = std::begin(template_str);
    while (it != std::end(template_str)) {
        auto search_begin = it;
        it = std::search(search_begin, std::end(template_str), searcher);

        ss << std::string_view(&(*search_begin),
                               std::distance(search_begin, it));
        if (it != std::end(template_str)) {
            std::advance(it, pattern.size());
            ss << replace_str;
        }
    }

    return ss.str();
}

#endif  // SEYEON_RESOURCE_COMPILER_SRC_UTILS_H_