#!lua

includeDirList = {
    "../shared",
    "../shared/include",
    "../shared/gl3w",
    "../shared/imgui"
}

libDirectories = { 
    "../lib"
}


if os.target() == "macosx" then
    linkLibs = {
        "cs488-framework",
        "imgui",
        "glfw3",
        "lua",
		"lodepng"
    }
end

if os.target() == "linux" then
    linkLibs = {
        "cs488-framework",
        "imgui",
        "glfw3",
        "lua",
        "lodepng",
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

if os.target() == "macosx" then
    linkOptionList = { "-framework IOKit", "-framework Cocoa", "-framework CoreVideo", "-framework OpenGL" }
end

buildOptions = {"-std=c++11 -O2"}

workspace "CS488-Projects"
    configurations { "Debug", "Release" }

    project "A4"
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
