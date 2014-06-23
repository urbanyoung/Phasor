#include <list>
#include <vector>

// http://stackoverflow.com/a/12045843/1520427
template <typename Container>
struct is_container : std::false_type { };

template <typename... Ts> struct is_container<std::list<Ts...> > : std::true_type{};
template <typename... Ts> struct is_container<std::vector<Ts...> > : std::true_type{};