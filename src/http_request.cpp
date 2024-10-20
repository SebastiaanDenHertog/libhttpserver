/*
     This file is part of libhttpserver
     Copyright (C) 2011-2019 Sebastiano Merlino

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
     USA

*/

#include "httpserver/http_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "httpserver/http_utils.h"
#include "httpserver/string_utilities.h"

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

namespace httpserver
{

    const char http_request::EMPTY[] = "";

    struct arguments_accumulator
    {
        unescaper_ptr unescaper;
        std::map<std::string, std::vector<std::string>, http::arg_comparator> *arguments;
    };

    void http_request::set_method(const std::string &method)
    {
        this->method = string_utilities::to_upper_copy(method);
    }

    bool http_request::check_digest_auth(const std::string &realm, const std::string &password, int nonce_timeout, bool *reload_nonce) const
    {
        string_view digested_user = get_digested_user();

        int val = MHD_digest_auth_check(underlying_connection, realm.c_str(), digested_user.data(), password.c_str(), nonce_timeout);

        if (val == MHD_INVALID_NONCE)
        {
            *reload_nonce = true;
            return false;
        }
        else if (val == MHD_NO)
        {
            *reload_nonce = false;
            return false;
        }
        *reload_nonce = false;
        return true;
    }

    string_view http_request::get_connection_value(string_view key, enum MHD_ValueKind kind) const
    {
        const char *header_c = MHD_lookup_connection_value(underlying_connection, kind, key.data());

        if (header_c == nullptr)
            return EMPTY;

        return header_c;
    }

    MHD_Result http_request::build_request_header(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
    {
        // Parameters needed to respect MHD interface, but not used in the implementation.
        std::ignore = kind;

        http::header_view_map *dhr = static_cast<http::header_view_map *>(cls);
        (*dhr)[key] = value;
        return MHD_YES;
    }

    const http::header_view_map http_request::get_headerlike_values(enum MHD_ValueKind kind) const
    {
        http::header_view_map headers;

        MHD_get_connection_values(underlying_connection, kind, &build_request_header, reinterpret_cast<void *>(&headers));

        return headers;
    }

    string_view http_request::get_header(string_view key) const
    {
        return get_connection_value(key, MHD_HEADER_KIND);
    }

    const http::header_view_map http_request::get_headers() const
    {
        return get_headerlike_values(MHD_HEADER_KIND);
    }

    string_view http_request::get_footer(string_view key) const
    {
        return get_connection_value(key, MHD_FOOTER_KIND);
    }

    const http::header_view_map http_request::get_footers() const
    {
        return get_headerlike_values(MHD_FOOTER_KIND);
    }

    string_view http_request::get_cookie(string_view key) const
    {
        return get_connection_value(key, MHD_COOKIE_KIND);
    }

    const http::header_view_map http_request::get_cookies() const
    {
        return get_headerlike_values(MHD_COOKIE_KIND);
    }

    void http_request::populate_args() const
    {
        if (cache->args_populated)
        {
            return;
        }
        arguments_accumulator aa;
        aa.unescaper = unescaper;
        aa.arguments = &cache->unescaped_args;
        MHD_get_connection_values(underlying_connection, MHD_GET_ARGUMENT_KIND, &build_request_args, reinterpret_cast<void *>(&aa));

        cache->args_populated = true;
    }

    void http_request::grow_last_arg(const std::string &key, const std::string &value)
    {
        auto it = cache->unescaped_args.find(key);

        if (it != cache->unescaped_args.end())
        {
            if (!it->second.empty())
            {
                it->second.back() += value;
            }
            else
            {
                it->second.push_back(value);
            }
        }
        else
        {
            cache->unescaped_args[key] = {value};
        }
    }

    http_arg_value http_request::get_arg(string_view key) const
    {
        populate_args();

        // Convert string_view to std::string for the map lookup
        auto it = cache->unescaped_args.find(std::string(key.data(), key.size()));

        if (it != cache->unescaped_args.end())
        {
            http_arg_value arg;
            arg.values.reserve(it->second.size());
            for (const auto &value : it->second)
            {
                arg.values.push_back(value);
            }
            return arg;
        }
        return http_arg_value();
    }

    string_view http_request::get_arg_flat(string_view key) const
    {
        auto const it = cache->unescaped_args.find(std::string(key.data(), key.size()));

        if (it != cache->unescaped_args.end())
        {
            return it->second[0];
        }

        return get_connection_value(key, MHD_GET_ARGUMENT_KIND);
    }

    #if __cplusplus >= 201703L // C++17 and later

    const http::arg_view_map http_request::get_args() const
    {
        populate_args();

        http::arg_view_map arguments;
        for (const auto &[key, value] : cache->unescaped_args)
        {
            auto &arg_values = arguments[key];
            for (const auto &v : value)
            {
                arg_values.values.push_back(v);
            }
        }
        return arguments;
    }

    const std::map<string_view, string_view, http::arg_comparator> http_request::get_args_flat() const
    {
        populate_args();
        std::map<string_view, string_view, http::arg_comparator> ret;
        for (const auto &[key, val] : cache->unescaped_args)
        {
            ret[key] = val[0];
        }
        return ret;
    }

    #else // C++11

    const http::arg_view_map http_request::get_args() const
    {
        populate_args();

        http::arg_view_map arguments;
        for (const auto &pair : cache->unescaped_args)
        {
            const std::string &key = pair.first;
            const std::vector<std::string> &value = pair.second;
            auto &arg_values = arguments[key];
            for (const auto &v : value)
            {
                arg_values.values.push_back(v);
            }
        }
        return arguments;
    }

    const std::map<string_view, string_view, http::arg_comparator> http_request::get_args_flat() const
    {
        populate_args();
        std::map<string_view, string_view, http::arg_comparator> ret;
        for (const auto &pair : cache->unescaped_args)
        {
            const std::string &key = pair.first;
            const std::vector<std::string> &val = pair.second;
            ret[key] = val[0];
        }
        return ret;
    }

    #endif

    http::file_info &http_request::get_or_create_file_info(const std::string &key, const std::string &upload_file_name)
    {
        return files[key][upload_file_name];
    }

    string_view http_request::get_querystring() const
    {
        if (!cache->querystring.empty())
        {
            return cache->querystring;
        }

        MHD_get_connection_values(underlying_connection, MHD_GET_ARGUMENT_KIND, &build_request_querystring, reinterpret_cast<void *>(&cache->querystring));

        return cache->querystring;
    }

    MHD_Result http_request::build_request_args(void *cls, enum MHD_ValueKind kind, const char *key, const char *arg_value)
    {
        // Parameters needed to respect MHD interface, but not used in the implementation.
        std::ignore = kind;

        arguments_accumulator *aa = static_cast<arguments_accumulator *>(cls);
        std::string value = ((arg_value == nullptr) ? "" : arg_value);

        http::base_unescaper(&value, aa->unescaper);
        (*aa->arguments)[key].push_back(value);
        return MHD_YES;
    }

    MHD_Result http_request::build_request_querystring(void *cls, enum MHD_ValueKind kind, const char *key_value, const char *arg_value)
    {
        // Parameters needed to respect MHD interface, but not used in the implementation.
        std::ignore = kind;

        std::string *querystring = static_cast<std::string *>(cls);

        string_view key = key_value;
        string_view value = ((arg_value == nullptr) ? "" : arg_value);

        // Limit to a single allocation.
        querystring->reserve(querystring->size() + key.size() + value.size() + 3);

        *querystring += ((*querystring == "") ? "?" : "&");
        *querystring += std::string(key.data(), key.size());
        *querystring += "=";
        *querystring += std::string(value.data(), value.size());

        return MHD_YES;
    }

    void http_request::fetch_user_pass() const
    {
        char *password = nullptr;
        auto *username = MHD_basic_auth_get_username_password(underlying_connection, &password);

        if (username != nullptr)
        {
            cache->username = username;
            MHD_free(username);
        }
        if (password != nullptr)
        {
            cache->password = password;
            MHD_free(password);
        }
    }

    string_view http_request::get_user() const
    {
        if (!cache->username.empty())
        {
            return cache->username;
        }
        fetch_user_pass();
        return cache->username;
    }

    string_view http_request::get_pass() const
    {
        if (!cache->password.empty())
        {
            return cache->password;
        }
        fetch_user_pass();
        return cache->password;
    }

    string_view http_request::get_digested_user() const
    {
        if (!cache->digested_user.empty())
        {
            return cache->digested_user;
        }

        char *digested_user_c = MHD_digest_auth_get_username(underlying_connection);

        cache->digested_user = EMPTY;
        if (digested_user_c != nullptr)
        {
            cache->digested_user = digested_user_c;
            free(digested_user_c);
        }

        return cache->digested_user;
    }

#ifdef HAVE_GNUTLS
    bool http_request::has_tls_session() const
    {
        const MHD_ConnectionInfo *conninfo = MHD_get_connection_info(underlying_connection, MHD_CONNECTION_INFO_GNUTLS_SESSION);
        return (conninfo != nullptr);
    }

    gnutls_session_t http_request::get_tls_session() const
    {
        const MHD_ConnectionInfo *conninfo = MHD_get_connection_info(underlying_connection, MHD_CONNECTION_INFO_GNUTLS_SESSION);

        return static_cast<gnutls_session_t>(conninfo->tls_session);
    }
#endif // HAVE_GNUTLS

    string_view http_request::get_requestor() const
    {
        if (!cache->requestor_ip.empty())
        {
            return cache->requestor_ip;
        }

        const MHD_ConnectionInfo *conninfo = MHD_get_connection_info(underlying_connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);

        cache->requestor_ip = http::get_ip_str(conninfo->client_addr);
        return cache->requestor_ip;
    }

    uint16_t http_request::get_requestor_port() const
    {
        const MHD_ConnectionInfo *conninfo = MHD_get_connection_info(underlying_connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);

        return http::get_port(conninfo->client_addr);
    }

    std::ostream &operator<<(std::ostream &os, const http_request &r)
    {
        os << std::string(r.get_method().data(), r.get_method().size()) << " Request [user:\""
           << std::string(r.get_user().data(), r.get_user().size()) << "\" pass:\""
           << std::string(r.get_pass().data(), r.get_pass().size()) << "\"] path:\""
           << std::string(r.get_path().data(), r.get_path().size()) << "\"" << std::endl;

        http::dump_header_map(os, "Headers", r.get_headers());
        http::dump_header_map(os, "Footers", r.get_footers());
        http::dump_header_map(os, "Cookies", r.get_cookies());
        http::dump_arg_map(os, "Query Args", r.get_args());

        os << "    Version [ " << std::string(r.get_version().data(), r.get_version().size())
           << " ] Requestor [ " << std::string(r.get_requestor().data(), r.get_requestor().size())
           << " ] Port [ " << r.get_requestor_port() << " ]" << std::endl;

        return os;
    }

    http_request::~http_request()
    {
        for (const auto &file_key : get_files())
        {
            for (const auto &files : file_key.second)
            {
                // C++17 has std::filesystem::remove()
                remove(files.second.get_file_system_file_name().c_str());
            }
        }
    }

} // namespace httpserver
