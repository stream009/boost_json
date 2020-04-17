//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_CONFIG_HPP
#define BOOST_JSON_CONFIG_HPP

#include <boost/json/detail/config.hpp>

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE

/// The type of string view used by the library.
using string_view = boost::string_view;

/// The type of memory_resource used by the library.
using memory_resource = boost::container::pmr::memory_resource;

#else

using string_view = std::string_view;
using memory_resource = std::pmr::memory_resource;

#endif

} // json
} // boost

#endif
