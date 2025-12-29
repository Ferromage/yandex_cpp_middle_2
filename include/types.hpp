#pragma once

#include <string>
#include <tuple>

namespace stdx::details {

template <typename T>
using type = std::remove_cvref_t<T>;

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
struct scan_result {
    scan_result(Ts &&...ts) : data_(std::forward<type<Ts>>(ts)...) {}
    scan_result(const std::tuple<Ts...> &data) : data_(data) {}

    std::tuple<type<Ts>...> values() const { return data_; }

private:
    std::tuple<type<Ts>...> data_;
};

}  // namespace stdx::details
