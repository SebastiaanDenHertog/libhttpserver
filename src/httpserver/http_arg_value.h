#if !defined(_HTTPSERVER_HPP_INSIDE_) && !defined(HTTPSERVER_COMPILATION)
#error "Only <httpserver.h> or <httpserverpp> can be included directly."
#endif

#ifndef SRC_HTTPSERVER_HTTP_ARG_VALUE_HPP_
#define SRC_HTTPSERVER_HTTP_ARG_VALUE_HPP_

// Custom string_view for C++11, standard string_view for C++17 and above
#if __cplusplus >= 201703L
#include <string_view>
namespace httpserver
{
    using string_view = std::string_view;
}
#else
#include "include/utils/string_view.h" // Use custom string_view for C++11
namespace httpserver
{
    using string_view = StringView;
}
#endif

#include <string>
#include <vector>

namespace httpserver
{

    class http_arg_value
    {
    public:
        string_view get_flat_value() const
        {
            return values.empty() ? "" : values[0];
        }

        std::vector<string_view> get_all_values() const
        {
            return values;
        }

        // Conversion to std::string
        operator std::string() const
        {
            string_view value = get_flat_value();
            return std::string(value.data(), value.size()); // Explicit conversion
        }

        // Conversion to string_view
        operator string_view() const
        {
            return get_flat_value();
        }

        // Conversion to std::vector<std::string>
        operator std::vector<std::string>() const
        {
            std::vector<std::string> result;
            for (auto const &value : values)
            {
                result.push_back(std::string(value.data(), value.size())); // Explicit conversion
            }
            return result;
        }

        std::vector<string_view> values;
    };

} // end namespace httpserver

#endif // SRC_HTTPSERVER_HTTP_ARG_VALUE_HPP_
