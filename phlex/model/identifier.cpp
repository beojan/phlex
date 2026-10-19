#include "phlex/model/identifier.hpp"

#include <compare>
#include <cstddef>
#include <string_view>
#include <utility>

namespace phlex::experimental {
  namespace {
    void validate_identifier(std::string_view str)
    {
// No empty identifiers
#define PHLEX_ALLOW_EMPTY_IDENTIFIER 1
#ifndef PHLEX_ALLOW_EMPTY_IDENTIFIER
      if (str.empty()) {
        throw invalid_identifier_error(
          "Invalid Identifier: The empty string is not a valid identifier!");
      }
#endif
      constexpr auto char_class = [](char c) {
        if ('0' <= c && '9' >= c) {
          return 'd';
        }

        if ('A' <= c && 'Z' >= c) {
          return 'L'; // capital Letter
        }

        if ('a' <= c && 'z' >= c) {
          return 'l'; // lower case letter
        }

        if (c == '_') {
          return '_';
        }

        return 'X';
      };

      if (char_class(str[0]) == 'd') {
        throw invalid_identifier_error(
          fmt::format("Invalid Identifier '{}': Starts with a digit!", str));
      }

      for (char c : str) {
        if (char_class(c) == 'X') {
          throw invalid_identifier_error(
            fmt::format("Invalid Identifier '{}': Contains invalid character '{}'!", str, c));
        }
      }
    }
  }

  identifier::identifier(std::string_view str) : content_(str), hash_(hash_string(content_))
  {
    validate_identifier(str);
  }
  identifier::identifier(std::string&& str) : content_(std::move(str)), hash_(hash_string(content_))
  {
    validate_identifier(str);
  }

  identifier::operator std::string_view() const noexcept { return std::string_view(content_); }
  bool identifier::operator==(identifier const& rhs) const noexcept
  {
    if (hash_ == rhs.hash_) {
      return content_ == rhs.content_;
    }
    return false;
  }
  std::strong_ordering identifier::operator<=>(identifier const& rhs) const noexcept
  {
    std::strong_ordering hash_cmp = hash_ <=> rhs.hash_;
    if (hash_cmp == 0) {
      return content_ <=> rhs.content_;
    }
    return hash_cmp;
  }

  bool operator==(identifier const& lhs, identifier_query rhs) { return lhs.hash_ == rhs.hash; }
  std::strong_ordering operator<=>(identifier const& lhs, identifier_query rhs)
  {
    return lhs.hash_ <=> rhs.hash;
  }

  identifier literals::operator""_id(char const* lit, std::size_t len)
  {
    return identifier{std::string_view(lit, len)};
  }

  bool identifier_query::operator()(identifier const& id) const noexcept { return id == (*this); }
}
