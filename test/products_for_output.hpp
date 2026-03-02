#ifndef TEST_PRODUCTS_FOR_OUTPUT_HPP
#define TEST_PRODUCTS_FOR_OUTPUT_HPP

#include "phlex/model/product_store.hpp"

#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

#include <ranges>

namespace phlex::experimental::test {
  struct products_for_output {
    void save(product_store const& store) const
    {
      namespace views = std::ranges::views;
      spdlog::debug("Saving data for store id: {} from source: {} \n -> {}",
                    store.index()->to_string(),
                    store.source().full(),
                    fmt::join(store | views::keys | views::elements<1>, "\n -> "));
    }
  };

}

#endif // TEST_PRODUCTS_FOR_OUTPUT_HPP
