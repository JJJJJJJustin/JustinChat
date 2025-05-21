#include "JCpch.h"

#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include "CServer.h"

int main()
{
    JChat::Log::Init();
    JC_CORE_INFO("Justin Chat GataServer is running ...");

    try {
        boost::asio::io_context iocontext{ 1 };
        unsigned short port = static_cast<unsigned short>(8080);
        boost::asio::signal_set signals(iocontext, SIGINT, SIGTERM);

        signals.async_wait([&iocontext](const boost::system::error_code& ec, int signalNumber)
            {
                if (ec) {
                    JC_CORE_ERROR("{}, in {}", ec.what(), __FILE__);
                    return;
                }
                else
                {
                    iocontext.stop();
                }
            }
        );

        std::shared_ptr<CServer> cs = std::make_shared<CServer>(iocontext, port);
        cs->Start();
        iocontext.run();
    }
    catch(std::exception& ex)
    {
        JC_CORE_CRITICAL("{}", ex.what());
        return EXIT_FAILURE;
    }


}