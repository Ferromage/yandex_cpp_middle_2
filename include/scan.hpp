#pragma once

#include "parse.hpp"
#include "types.hpp"
#include <expected>

namespace stdx {

template <typename Tuple, size_t... Is>
void scanImpl(const std::vector<std::string_view> &inputs, const std::vector<std::string_view> &formats, Tuple &tuple,
              std::index_sequence<Is...>) {
    ((std::get<Is>(tuple) = details::parse_value_with_format<typename std::tuple_element<Is, Tuple>::type::value_type>(
          inputs[Is], formats[Is])),
     ...);
}

template <typename... Ts>
auto extractValues(const std::tuple<std::expected<Ts, details::scan_error>...> &input)
    -> std::expected<std::tuple<Ts...>, details::scan_error> {

    std::tuple<details::type<Ts>...> result;
    bool ok = true;
    details::scan_error err;

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ([&] {
            const auto &exp = std::get<Is>(input);
            if (!exp.has_value()) {
                err = exp.error();
                ok = false;
                return false;
            }
            std::get<Is>(result) = *exp;
            return true;
        }() &&
         ...);
    }(std::index_sequence_for<details::type<Ts>...>{});

    if (!ok) {
        return std::unexpected(err);
    }
    return result;
}

template <typename... Ts>
std::expected<details::scan_result<details::type<Ts>...>, details::scan_error> scan(std::string_view input,
                                                                                    std::string_view format) {
    const auto parseResult = details::parse_sources(input, format);
    if (!parseResult.has_value()) {
        return std::unexpected(parseResult.error());
    }

    const auto &[fmt, in] = parseResult.value();
    if (sizeof...(Ts) != fmt.size() || sizeof...(Ts) != in.size()) {
        return std::unexpected(details::scan_error{"Arguments count doesn't match"});
    }

    using Tuple = std::tuple<std::expected<details::type<Ts>, details::scan_error>...>;
    Tuple tuple;
    scanImpl(in, fmt, tuple, std::index_sequence_for<Ts...>{});

    auto result = extractValues(tuple);
    if (result.has_value()) {
        return details::scan_result(result.value());
    } else {
        return std::unexpected(result.error());
    }
}
}  // namespace stdx
