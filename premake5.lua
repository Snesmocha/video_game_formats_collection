-- premake5.lua

workspace "vgf"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "tfss"

project "tfss"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    files { "src/**.h", "src/**.c" }

    includedirs { "include" } -- your header folders
    libdirs { "libs" }        -- your library folders

    -- Example: link ZSTD if you have it in libs
    links { "zstd" }

    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }

    filter "system:linux"
        defines { "PLATFORM_LINUX" }
        buildoptions { "-pthread" }
        links { "pthread" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "On"
