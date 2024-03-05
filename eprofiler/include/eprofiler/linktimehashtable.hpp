#ifndef LINKTIMEHASHTABLE_HPP
#define LINKTIMEHASHTABLE_HPP

#include <span>
#include <eprofiler/stringconstant.hpp>

namespace eprofiler {

// class EProfilerTmpl to couple StringConstat_WithID with specific EProfiler
template<class EProfilerTmpl, class IDType, class ValueType>
class LinkTimeHashTable {
public:
    // StringConstant is used to create unique string literal tags for each profiler
    template<class CharT, CharT... Chars>
    struct StringConstant_WithID : public StringConstant<CharT, Chars...> {
        // declared in a generated translation unit
        IDType to_id() const noexcept;
    };

    // Convert StringConstant to StringConstant_WithID
    template<class CharT, CharT... Chars>
    static StringConstant_WithID<CharT, Chars...> convert_string_constant(StringConstant<CharT, Chars...> const& sc) noexcept {
        return StringConstant_WithID<CharT, Chars...>{sc};
    }

    // Key value storage externally defined in a generated translation unit
    const static std::span<const std::string_view> keys;
    const static std::span<ValueType> value_store;

private:
    // Private functions operating on StringConstant_WithID keys
    template<class CharT, CharT... Chars>
    static ValueType& at(StringConstant_WithID<CharT, Chars...> const& str) noexcept {
        return value_store[str.to_id()];
    }

public:

    // Public functions operating on StringConstant tags
    template<class CharT, CharT... Chars>
    static ValueType& at(StringConstant<CharT, Chars...> const& str) noexcept {
        return at(convert_string_constant(str));
    }

    template<class CharT, CharT... Chars>
    static IDType get_id(StringConstant<CharT, Chars...> const& str) noexcept {
        return convert_string_constant(str).to_id();
    }

}; // class EProfiler


} // namespace eprofiler



#endif