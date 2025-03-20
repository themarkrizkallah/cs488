#!lua

-- Additional Linux libs: "X11", "Xxf86vm", "Xi", "Xrandr", "stdc++"

includeDirList = {
    "../shared",
    "../shared/include",
    "../shared/gl3w",
    "../shared/imgui"
}

libDirectories = { 
    "../lib"
}


if os.host() == "macosx" then
    linkLibs = {
        "cs488-framework",
        "imgui",
        "glfw3"
    }
end

if os.host() == "linux" then
    linkLibs = {
        "cs488-framework",
        "imgui",
        "glfw3",
        "GL",
        "Xinerama",
        "Xcursor",
        "Xxf86vm",
        "Xi",
        "Xrandr",
        "X11",
        "stdc++",
        "dl",
        "pthread"
    }
end

-- Build Options:
if os.host() == "macosx" then
    linkOptionList = { "-framework IOKit", "-framework Cocoa", "-framework CoreVideo", "-framework OpenGL" }
end

buildOptions = {"-std=c++11"}

workspace "CS488-Projects"
    configurations { "Debug", "Release" }

    project "A0"
        kind "ConsoleApp"
        language "C++"
        location "build"
        objdir "build"
        targetdir "."
        buildoptions (buildOptions)
        libdirs (libDirectories)
        links (linkLibs)
        linkoptions (linkOptionList)
        includedirs (includeDirList)
        files { "*.cpp" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
