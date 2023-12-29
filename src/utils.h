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

inline void ToUppercaseInline(std::string& str) {
    std::transform(
        std::begin(str), std::end(str), std::begin(str),
        [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
}

inline std::string ToUppercase(const std::string& str) {
    std::string ret{str};
    ToUppercaseInline(ret);
    return ret;
}

inline void ToLowercaseInline(std::string& str) {
    std::transform(
        std::begin(str), std::end(str), std::begin(str),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
}

inline std::string ToLowercase(const std::string& str) {
    std::string ret{str};
    ToLowercaseInline(ret);
    return ret;
}

// Return (function_prefix, define_prefix)
inline std::pair<std::string, std::string> GetPrefixes(
    const std::string_view libname) {
    if (libname.empty()) {
        return {"", ""};
    }

    std::string function_prefix;
    if (std::isdigit(static_cast<unsigned char>(libname[0]))) {
        function_prefix = "_" + std::string(libname.size(), ' ');
    } else {
        function_prefix = libname;
    }
    ToLowercaseInline(function_prefix);

    std::transform(std::begin(function_prefix), std::end(function_prefix),
                   std::begin(function_prefix), [](unsigned char ch) {
                       if (std::isalnum(ch)) {
                           return static_cast<char>(ch);
                       } else {
                           return '_';
                       }
                   });

    return {function_prefix, ToUppercase(function_prefix)};
}

#endif  // SEYEON_RESOURCE_COMPILER_SRC_UTILS_H_