#ifndef PHLEX_MODEL_PRODUCTS_HPP
#define PHLEX_MODEL_PRODUCTS_HPP

#include "phlex/model/product_specification.hpp"

#include <cassert>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>

namespace phlex::experimental {

  struct product_base {
    virtual ~product_base() = default;
    virtual void const* address() const = 0;
    virtual std::type_info const& type() const = 0;
  };

  template <typename T>
  struct product : product_base {
    explicit product(T const& prod) : obj{prod} {}

    // The following constructor does NOT use a forwarding/universal reference!
    // It is not a template itself, but it uses the template parameter T from the
    // class template.
    explicit product(T&& prod) : obj{std::move(prod)} {}

    void const* address() const final { return &obj; }
    std::type_info const& type() const final { return typeid(T); }
    std::remove_cvref_t<T> obj;
  };

  class products {
    using product_ptr = std::unique_ptr<product_base>;
    using product_id = std::tuple<type_id, identifier>;
    using collection_t = std::map<product_id, product_ptr>;

  public:
    using const_iterator = collection_t::const_iterator;
    using size_type = collection_t::size_type;

    template <typename T>
    void add(product_specification const& spec, T t)
    {
      products_.emplace(spec_to_id<T>(spec),
                        std::make_unique<product<std::remove_cvref_t<T>>>(std::move(t)));
    }

    template <typename T>
    void add(product_specification const& spec, std::unique_ptr<product<T>> t)
    {
      products_.emplace(spec_to_id<T>(spec), std::move(t));
    }

    template <typename Ts>
    void add_all(product_specifications const& names, Ts ts)
    {
      assert(names.size() == 1ull);
      add(names[0], std::move(ts));
    }

    template <typename... Ts>
    void add_all(product_specifications const& names, std::tuple<Ts...> ts)
    {
      assert(names.size() == sizeof...(Ts));
      [this, &names]<std::size_t... Is>(auto tuple, std::index_sequence<Is...>) {
        (this->add(names[Is], std::move(std::get<Is>(tuple))), ...);
      }(std::move(ts), std::index_sequence_for<Ts...>{});
    }

    template <typename T>
    T const& get(product_specification const& spec) const
    {
      product_id const prod = spec_to_id<T>(spec);
      auto it = products_.find(prod);
      if (it == cend(products_)) {
        throw std::runtime_error(
          fmt::format("No product exists with the name '{}' and type '{}'.",
                      std::get<1>(prod),
                      std::get<0>(prod)));
      }

      auto const* available_product = it->second.get();

      if (auto const* desired_product = dynamic_cast<product<T> const*>(available_product)) {
        return desired_product->obj;
      }

      throw_mismatched_type(spec, typeid(T).name(), available_product->type().name());
    }

    bool contains(product_specification const& spec) const;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    size_type size() const noexcept;
    bool empty() const noexcept;

  private:
    static void throw_mismatched_type [[noreturn]] (product_specification const& spec,
                                                    char const* requested_type,
                                                    char const* available_type);

    template <typename T>
    static product_id spec_to_id(product_specification const& spec) {
      return product_id{spec.type().valid() ? spec.type() : make_type_id<T>(), spec.name()};
    }
    collection_t products_;
  };
}

#endif // PHLEX_MODEL_PRODUCTS_HPP
