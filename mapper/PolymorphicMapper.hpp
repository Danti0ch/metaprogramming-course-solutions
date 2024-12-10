#include <optional>
#include <type_traits>
#include <iostream>

template <typename From, auto targetValue>
struct Mapping
{
  using TargetType = decltype(targetValue);
};

template <typename Base, typename Target, typename... Mappings>
 requires(std::same_as<
               std::remove_cv_t<typename Mappings::TargetType>,
               Target> &&
           ...)
struct PolymorphicMapper
{
  static std::optional<Target> map(const Base&)
  { return std::nullopt; }
};

template <typename Base, typename Target, auto targetValue, typename Cast, typename... Mappings>
requires std::is_base_of_v<Base, Cast>
struct PolymorphicMapper<Base, Target, Mapping<Cast, targetValue>, Mappings...>
{
  static std::optional<Target> map(const Base& object)
  {
    if (const Cast* castObject = dynamic_cast<const Cast*>(&object))
      return std::optional<Target>{targetValue};
    return PolymorphicMapper<Base, Target, Mappings...>::map(object);
  }
};
