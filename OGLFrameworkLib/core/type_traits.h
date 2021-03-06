/**
 * @file   type_traits.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.08
 *
 * @brief  Definition of some type traits used.
 */

#ifndef CGU_TYPE_TRAITS_H
#define CGU_TYPE_TRAITS_H

#include <type_traits>
#include <vector>
#include <array>

namespace cgu {

    template<typename T>
    struct has_contiguous_memory : std::false_type {};

    template<typename T, typename U>
    struct has_contiguous_memory<std::vector<T, U>> : std::true_type{};

    template<typename T>
    struct has_contiguous_memory<std::vector<bool, T>> : std::false_type{};

    template<typename T, typename U, typename V>
    struct has_contiguous_memory<std::basic_string<T, U, V>> : std::true_type{};

    template<typename T, std::size_t N>
    struct has_contiguous_memory<std::array<T, N>> : std::true_type{};

    template<typename T>
    struct has_contiguous_memory<T[]> : std::true_type{};

    template<typename T, std::size_t N>
    struct has_contiguous_memory<T[N]> : std::true_type{};
}

#endif // CGU_TYPE_TRAITS_H
