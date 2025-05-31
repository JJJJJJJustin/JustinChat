#include "JCpch.h"
#include "CServer.h"
#include "ConfigMgr.h"

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

int main()
{
    JChat::Log::Init();
    JC_CORE_INFO("Justin Chat GataServer is running ...");

    ConfigMgr& globalConfigMgr = ConfigMgr::Inst();
    std::string GatePortStr = globalConfigMgr["GateServer"]["Port"];

    try {
        boost::asio::io_context iocontext{ 1 };
        unsigned short port = atoi(GatePortStr.c_str());
        boost::asio::signal_set signals(iocontext, SIGINT, SIGTERM);

        signals.async_wait([&iocontext](const boost::system::error_code& ec, int signalNumber)
            {
                if (ec) {
                    JC_CORE_ERROR("{}, in {}\n", ec.what(), __FILE__);
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
        JC_CORE_INFO("Justin Chat listening on port : {}\n", port);
        iocontext.run();
    }
    catch(std::exception& ex)
    {
        JC_CORE_CRITICAL("Justin Chat has been terminated -> message: {}\n", ex.what());
        return EXIT_FAILURE;
    }


}