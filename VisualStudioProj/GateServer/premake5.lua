workspace "GateServer"
    architecture "x64"
    --startproject "NAME"       --[[启动项目]]

    configurations
    {
        "Debug",
        "Release"
    }

project "GateServer"
    location "GateServer"
    kind "ConsoleApp"
    language "C++"
    --cppdialect "C++17"        --C++标准（编译时）

    -- 宏定义
    defines
    {
       
    }

    -- 添加源文件
    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.cpp"
    }

    -- 添加包含目录
    includedirs
    {
        "%{prj.name}/vendor/Boost1_88_0/include",
        "%{prj.name}/vendor/Jsoncpp_1_9_6/include"
    }

    -- 添加库目录
    libdirs
    {
        "%{prj.name}/vendor/Boost1_88_0/lib"
    }

    -- 在 Debug 和 Release 配置同样的链接库
    links
    {
    }

    -- 对于 jsoncpp 需要动态设置库目录和链接库（因为二者有 debug 和 release 不同环境下的运行库）
    filter "configurations:Debug"
        libdirs
        {
            "%{prj.name}/vendor/Jsoncpp_1_9_6/lib/Debug"  -- Debug 库路径
        }
        links { "jsoncppd.lib" }  -- Debug 库

    filter "configurations:Release"
        libdirs
        {
            "%{prj.name}/vendor/Jsoncpp_1_9_6/lib/Release"  -- Release 库路径
        }
        links { "jsoncpp.lib" }  -- Release 库

    -- 运行时库配置
    filter "configurations:Debug"
        defines { "_DEBUG" }
        symbols "On"
        runtime "Debug"  -- 对应 /MDd

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"  -- 对应 /MD