#pragma once

#include <iostream>
#include <memory.h>
#include <unordered_map>

#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include "Core/Base.h"
#include "Core/Log.h"

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#ifdef JC_PLATFORM_WINDOWS
	#include <Windows.h>
#endif