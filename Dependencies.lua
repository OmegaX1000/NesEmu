IncludeDir = {}
IncludeDir["SDL2"] = "%{wks.location}/Dependencies/SDL2/include"
IncludeDir["Spdlog"] = "%{wks.location}/Dependencies/Spdlog/include"
IncludeDir["Diligent"] = "%{wks.location}/Dependencies/DiligentCore/include/"
IncludeDir["Imgui"] = "%{wks.location}/Dependencies/imgui/"
IncludeDir["ImGuiFonts"] = "%{wks.location}/Dependencies/IconFontCppHeaders/"
IncludeDir["Catch2"] = "%{wks.location}/Dependencies/Catch2/"
IncludeDir["NatFileDial"] = "%{wks.location}/Dependencies/nativefiledialog/src/include"

LibDir = {}
LibDir["SDL2"] = "%{wks.location}/Dependencies/SDL2/lib/x64"
LibDir["Diligent"] = "%{wks.location}/Dependencies/DiligentCore/lib/Release"