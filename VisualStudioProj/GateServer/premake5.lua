workspace "GateServer"
    architecture "x64"
    --startproject "NAME"       --[[启动项目]]

    configurations
    {
        "Debug",
        "Release"
    }

    -- 针对 MSVC 工具链启用 /utf-8
    filter "toolset:msc*"
        buildoptions { "/utf-8" }


project "GateServer"
    location "GateServer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"        --C++标准（编译时）

    targetdir ("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

    pchheader "JCpch.h"
    pchsource "%{prj.name}/src/JCpch.cpp"

    -- 宏定义
    defines
    {
       
    }

    -- 添加源文件
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.cc",

        "%{prj.name}/src/**.grpc.pb.cc",
        "%{prj.name}/src/**.pb.cc",
        "%{prj.name}/src/**.proto",

        "%{prj.name}/src/config.ini"
    }

    -- 添加包含目录
    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/Boost1_88_0/include",
        "%{prj.name}/vendor/Jsoncpp_1_9_6/include",
        "%{prj.name}/vendor/Spdlog_1_15_3/include",

        "%{prj.name}/vendor/grpc/include",
        "%{prj.name}/vendor/grpc/third_party/abseil-cpp",
        "%{prj.name}/vendor/grpc/third_party/address_sorting/include",
        "%{prj.name}/vendor/grpc/third_party/protobuf/src",
        "%{prj.name}/vendor/grpc/third_party/re2",

        "%{prj.name}/vendor/redis/include/hiredis",

        "%{prj.name}/vendor/mysql_connector/include"
    }

    -- 添加库目录
    libdirs
    {
        "%{prj.name}/vendor/Boost1_88_0/lib",
    }

    -- 在 Debug 和 Release 配置同样的链接库
    links
    {
    }

    -- 对于 jsoncpp 需要动态设置库目录和链接库（因为二者有 debug 和 release 不同环境下的运行库）
    filter "configurations:Debug"
        -- Debug 模式下的附件库目录(目前我们只生成了 jsoncpp 和 grpc 在 Debug 模式下的库，如果需要 Release 版本的库，需要外部编译生成，然后在脚本中添加)
        libdirs
        {
            "%{prj.name}/vendor/Jsoncpp_1_9_6/lib/Debug",

            "%{prj.name}/vendor/grpc/visual_pro/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/re2/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/base/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/container/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/crc/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/debugging/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/flags/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/hash/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/log/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/numeric/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/profiling/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/random/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/status/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/strings/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/synchronization/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/types/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/abseil-cpp/absl/time/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/boringssl-with-bazel/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/cares/cares/lib/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/protobuf/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/protobuf/third_party/utf8_range/Debug",
            "%{prj.name}/vendor/grpc/visual_pro/third_party/zlib/Debug",

            "%{prj.name}/vendor/redis/lib/Debug",

            "%{prj.name}/vendor/mysql_connector/lib/debug/vs14"
        }
        
        links       -- Debug 模式下的附加依赖项
        { 
            "jsoncppd.lib",
            "libprotobufd.lib",

            "gpr.lib",
            "grpc.lib",
            "grpc++.lib",
            "grpc++_reflection.lib",
            "address_sorting.lib",
            "ws2_32.lib",
            "cares.lib",
            "zlibstaticd.lib",
            "upb_base_lib.lib",
            "upb_hash_lib.lib",
            "upb_json_lib.lib",       -- 注意：检查文件名是否实际为 upb_json_lib.lib
            "upb_lex_lib.lib",
            "upb_mem_lib.lib",
            "upb_message_lib.lib",
            "upb_mini_descriptor_lib.lib",
            "upb_mini_table_lib.lib",
            "upb_reflection_lib.lib",
            "upb_textformat_lib.lib",
            "upb_wire_lib.lib",
            "ssl.lib",
            "crypto.lib",
            "absl_bad_any_cast_impl.lib",
            "absl_bad_optional_access.lib",
            "absl_bad_variant_access.lib",
            "absl_base.lib",
            "absl_city.lib",
            "absl_civil_time.lib",
            "absl_cord.lib",
            "absl_cord_internal.lib",
            "absl_cordz_functions.lib",
            "absl_cordz_handle.lib",
            "absl_cordz_info.lib",
            "absl_cordz_sample_token.lib",
            "absl_crc32c.lib",
            "absl_crc_cord_state.lib",
            "absl_crc_internal.lib",
            "absl_hash.lib",
            "absl_str_format_internal.lib",
            "absl_strings.lib",
            "absl_string_view.lib",
            "absl_strings.lib",
            "absl_strings_internal.lib",
            "absl_debugging_internal.lib",
            "absl_demangle_internal.lib",
            "absl_examine_stack.lib",
            "absl_exponential_biased.lib",
            "absl_failure_signal_handler.lib",
            "absl_flags_commandlineflag.lib",
            "absl_flags_commandlineflag_internal.lib",
            "absl_flags_config.lib",
            "absl_flags_internal.lib",
            "absl_flags_marshalling.lib",
            "absl_flags_parse.lib",
            "absl_flags_private_handle_accessor.lib",
            "absl_flags_program_name.lib",
            "absl_flags_reflection.lib",
            "absl_flags_usage.lib",
            "absl_flags_usage_internal.lib",
            "absl_graphcycles_internal.lib",
            "absl_hash.lib",
            "absl_hashtablez_sampler.lib",
            "absl_int128.lib",
            "absl_kernel_timeout_internal.lib",
            "absl_leak_check.lib",
            --"absl_leak_check_disable.lib",        --这个库好像 abseli 已经不支持了
            "absl_log_entry.lib",
            "absl_log_flags.lib",
            "absl_log_globals.lib",
            "absl_log_initialize.lib",
            "absl_log_internal_check_op.lib", 
            "absl_log_internal_conditions.lib",
            "absl_log_internal_fnmatch.lib",
            "absl_log_internal_format.lib",
            "absl_log_internal_globals.lib",
            "absl_log_internal_log_sink_set.lib",
            "absl_log_internal_message.lib",
            "absl_log_internal_nullguard.lib",
            "absl_log_internal_proto.lib",
            "absl_log_internal_structured_proto.lib",
            "absl_log_severity.lib",                -- 这个库好像也不支持了
            "absl_log_sink.lib",
            "absl_low_level_hash.lib",
            "absl_vlog_config_internal.lib",
            "absl_malloc_internal.lib",
            "absl_periodic_sampler.lib",
            "absl_random_distributions.lib",
            "absl_random_internal_distribution_test_util.lib",
            "absl_random_internal_platform.lib",
            "absl_random_internal_pool_urbg.lib",
            "absl_random_internal_randen.lib",
            "absl_random_internal_randen_hwaes.lib",
            "absl_random_internal_randen_hwaes_impl.lib",
            "absl_random_internal_randen_slow.lib",
            "absl_random_internal_seed_material.lib",
            "absl_random_seed_gen_exception.lib",
            "absl_random_seed_sequences.lib",
            "absl_raw_hash_set.lib",
            "absl_raw_logging_internal.lib",
            "absl_scoped_set_env.lib",
            "absl_spinlock_wait.lib",
            "absl_stacktrace.lib",
            "absl_status.lib",
            "absl_strerror.lib",
            "absl_strings.lib",
            "absl_strings_internal.lib",
            "absl_str_format_internal.lib",
            "absl_symbolize.lib",
            "absl_synchronization.lib",
            "absl_throw_delegate.lib",
            "absl_time.lib",
            "absl_time_zone.lib",
            "absl_statusor.lib",
            "libutf8_range.lib",
            "libutf8_validity.lib",
            "re2.lib",

            "hiredis.lib",
            "Win32_Interop.lib",

            "mysqlcppconn.lib",
            "mysqlcppconn-static.lib",
            "mysqlcppconnx.lib",
            "mysqlcppconnx-static.lib"
        }

        -- buildoptions {
        --     "/GR",  -- 启用 RTTI
        --     "/EHsc" -- 启用异常
        -- }

    filter "configurations:Release"
        libdirs
        {
            "%{prj.name}/vendor/Jsoncpp_1_9_6/lib/Release"  -- Release 库路径
        }
        links { "jsoncpp.lib" }  -- Release 库

    -- 添加DLL拷贝命令
    filter "configurations:Debug"
        postbuildcommands           --构建项目完成后执行的指令
        {
            '{MKDIR} "%{cfg.targetdir}"',
            '{COPY} "E:/VS/JustinChat/VisualStudioProj/GateServer/GateServer/vendor/mysql_connector/lib/debug/mysqlcppconn-10-vs14.dll" "%{cfg.targetdir}"',
            '{COPY} "E:/VS/JustinChat/VisualStudioProj/GateServer/GateServer/vendor/mysql_connector/lib/debug/mysqlcppconnx-2-vs14.dll" "%{cfg.targetdir}"'
        }

    -- 运行时库配置
    filter "configurations:Debug"
        defines { "JC_DEBUG" }
        symbols "On"
        runtime "Debug"  -- 对应 /MDd

    filter "configurations:Release"
        defines { "JC_RELEASE" }
        optimize "On"
        runtime "Release"  -- 对应 /MD