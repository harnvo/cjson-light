add_rules("mode.debug", "mode.release" )


target("src")
    set_languages("c11")
    set_kind("shared")
    add_includedirs("include")
    -- set up debug keywords
    if is_mode("debug") then
        add_defines("_DEBUG")
        add_cxflags("-O0 -g3")
        set_optimize("none")
        -- set sanitize address
        if is_plat("linux") then
            add_cxflags("-fsanitize=address")
            add_ldflags("-fsanitize=address")

            add_cxflags("-fsanitize=undefined")
            add_ldflags("-fsanitize=undefined")

            add_cxflags("-fsanitize=leak")
            add_ldflags("-fsanitize=leak")
        end
    else
        add_cxflags("-O3")
        set_optimize("fastest")
    end

    add_files("src/str_view.c")
    add_files("src/json_parser.c")
    add_files("src/json_obj.c")
    add_files("src/json_list.c")
    add_files("src/json_array.c")

    -- target dir
    set_targetdir("$(buildir)/$(mode)")
    -- name
    if is_mode("debug") then
        set_basename("json-debug")
    else
        set_basename("json-c")
    end

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

