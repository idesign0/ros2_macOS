// macOS libc++ lacks the PSTL <execution> policies, and std::is_execution_policy_v is
// _LIBCPP_NO_SPECIALIZATIONS (always false, cannot be specialized). Provide the policy
// types/objects + serial overloads of the algorithms Beluga uses, and a Beluga-local
// is_execution_policy_v trait. Beluga sources are patched std::is_execution_policy_v ->
// beluga::is_execution_policy_v. Serial semantics are correct (Beluga only needs seq here).
#pragma once
#include <version>
#include <type_traits>
#if !defined(__cpp_lib_execution)
#include <algorithm>
#include <utility>
namespace std { namespace execution {
struct sequenced_policy {};
struct parallel_policy {};
struct parallel_unsequenced_policy {};
struct unsequenced_policy {};
inline constexpr sequenced_policy seq{};
inline constexpr parallel_policy par{};
inline constexpr parallel_unsequenced_policy par_unseq{};
inline constexpr unsequenced_policy unseq{};
}}  // namespace std::execution
namespace beluga_pstl_detail {
template <class T> inline constexpr bool is_ep =
    std::is_same_v<std::decay_t<T>, std::execution::sequenced_policy> ||
    std::is_same_v<std::decay_t<T>, std::execution::parallel_policy> ||
    std::is_same_v<std::decay_t<T>, std::execution::parallel_unsequenced_policy> ||
    std::is_same_v<std::decay_t<T>, std::execution::unsequenced_policy>;
}
namespace std {
template <class EP, class... A, class = enable_if_t<beluga_pstl_detail::is_ep<EP>>>
auto transform(EP&&, A&&... a) { return std::transform(std::forward<A>(a)...); }
template <class EP, class... A, class = enable_if_t<beluga_pstl_detail::is_ep<EP>>>
void for_each(EP&&, A&&... a) { std::for_each(std::forward<A>(a)...); }
}  // namespace std
namespace beluga { template <class T> inline constexpr bool is_execution_policy_v = beluga_pstl_detail::is_ep<T>; }
#else
namespace beluga { template <class T> inline constexpr bool is_execution_policy_v = std::is_execution_policy_v<std::decay_t<T>>; }
#endif
