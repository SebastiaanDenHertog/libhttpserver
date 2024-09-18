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

#ifndef SRC_HTTPSERVER_HPP_
#define SRC_HTTPSERVER_HPP_

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

#define _HTTPSERVER_HPP_INSIDE_

#include "httpserver/basic_auth_fail_response.h"
#include "httpserver/deferred_response.h"
#include "httpserver/digest_auth_fail_response.h"
#include "httpserver/file_response.h"
#include "httpserver/http_arg_value.h"
#include "httpserver/http_request.h"
#include "httpserver/http_resource.h"
#include "httpserver/http_response.h"
#include "httpserver/http_utils.h"
#include "httpserver/file_info.h"
#include "httpserver/string_response.h"
#include "httpserver/webserver.h"

#endif // SRC_HTTPSERVER_HPP_
