#pragma once

#include "types.hpp"
#include <charconv>
#include <expected>
#include <optional>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace stdx::details {

template <typename T>
    requires(std::is_same_v<type<T>, std::string>)
std::optional<type<T>> parse_value(std::string_view input) {
    return std::string(input);
}

template <typename T>
    requires(std::is_same_v<type<T>, std::string_view>)
std::optional<type<T>> parse_value(std::string_view input) {
    return input;
}

template <typename T>
    requires(std::is_integral_v<type<T>> || std::is_floating_point_v<type<T>>)
std::optional<type<T>> parse_value(std::string_view input) {
    type<T> value;
    if (std::from_chars(input.begin(), input.begin() + input.size(), value).ec == std::errc{}) {
        return value;
    }
    return std::nullopt;
}

template <typename T>
std::expected<type<T>, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    auto returnError = [fmt]() {
        return std::unexpected(scan_error{"parse_value_with_format: type " + std::string(typeid(type<T>).name()) +
                                          " is different from " + std::string(fmt)});
    };

    if (!fmt.empty()) {
        if constexpr (std::is_integral_v<type<T>>) {
            if constexpr (std::is_signed_v<type<T>>) {
                if (fmt != "%d") {
                    return returnError();
                }
            } else {
                if (fmt != "%u") {
                    return returnError();
                }
            }
        } else if constexpr (std::is_floating_point_v<type<T>>) {
            if (fmt != "%f") {
                return returnError();
            }
        } else if constexpr (std::is_same_v<type<T>, std::string> || std::is_same_v<type<T>, std::string_view>) {
            if (fmt != "%s") {
                return returnError();
            }
        } else {
            return returnError();
        }
    }

    const auto res = parse_value<type<T>>(input);
    if (!res) {
        return std::unexpected(scan_error{"parse_value_with_format: error on parse " + std::string(input) + " to " +
                                          std::string(typeid(type<T>).name())});
    }
    return res.value();
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

}  // namespace stdx::details