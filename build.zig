const std = @import("std");

const freetype_sources: []const []const u8 = &.{
    "autofit/autofit.c",
    "base/ftbase.c",
    "base/ftbbox.c",
    "base/ftbdf.c",
    "base/ftbitmap.c",
    "base/ftcid.c",
    "base/ftfstype.c",
    "base/ftgasp.c",
    "base/ftglyph.c",
    "base/ftgxval.c",
    "base/ftinit.c",
    "base/ftmm.c",
    "base/ftotval.c",
    "base/ftpatent.c",
    "base/ftpfr.c",
    "base/ftstroke.c",
    "base/ftsynth.c",
    "base/ftsystem.c",
    "base/fttype1.c",
    "base/ftwinfnt.c",
    "bdf/bdf.c",
    "cache/ftcache.c",
    "cff/cff.c",
    "cid/type1cid.c",
    "gzip/ftgzip.c",
    "lzw/ftlzw.c",
    "pcf/pcf.c",
    "pfr/pfr.c",
    "psaux/psaux.c",
    "pshinter/pshinter.c",
    "psnames/psnames.c",
    "raster/raster.c",
    "sfnt/sfnt.c",
    "smooth/smooth.c",
    "sdf/sdf.c",
    "truetype/truetype.c",
    "type1/type1.c",
    "type42/type42.c",
    "winfonts/winfnt.c",
    "svg/ftsvg.c",
};

const sdl_files_to_skip = [_][]const u8{ "ska_sdl.c", "sdl_input.c" };

pub fn collectCSources(b: *std.Build, sub_path: []const u8) []const []const u8 {
    const io = b.graph.io;
    var sources: std.ArrayList([]const u8) = .empty;
    var dir = b.build_root.handle.openDir(io, sub_path, .{ .iterate = true }) catch return &.{};
    defer dir.close(io);
    var walker = dir.walk(b.allocator) catch return &.{};
    defer walker.deinit();
    outer: while (walker.next(io) catch return sources.toOwnedSlice(b.allocator) catch &.{}) |entry| {
        if (entry.kind != .file or !std.mem.endsWith(u8, entry.basename, ".c")) continue;
        for (sdl_files_to_skip) |skip| {
            if (std.mem.eql(u8, entry.basename, skip)) continue :outer;
        }
        sources.append(b.allocator, b.dupe(entry.path)) catch continue;
    }
    return sources.toOwnedSlice(b.allocator) catch &.{};
}

// window_cocoa.m/input_cocoa.m (macOS backend, Objective-C) need this -- collectCSources only
// picks up .c files. Only ever called for macOS targets (see buildSeika below).
pub fn collectObjCSources(b: *std.Build, sub_path: []const u8) []const []const u8 {
    const io = b.graph.io;
    var sources: std.ArrayList([]const u8) = .empty;
    var dir = b.build_root.handle.openDir(io, sub_path, .{ .iterate = true }) catch return &.{};
    defer dir.close(io);
    var walker = dir.walk(b.allocator) catch return &.{};
    defer walker.deinit();
    while (walker.next(io) catch return sources.toOwnedSlice(b.allocator) catch &.{}) |entry| {
        if (entry.kind != .file or !std.mem.endsWith(u8, entry.basename, ".m")) continue;
        sources.append(b.allocator, b.dupe(entry.path)) catch continue;
    }
    return sources.toOwnedSlice(b.allocator) catch &.{};
}

pub fn collectCSourcesExclude(
    b: *std.Build,
    sub_path: []const u8,
    skip_files: []const []const u8,
    skip_dir_prefixes: []const []const u8,
) []const []const u8 {
    const io = b.graph.io;
    var sources: std.ArrayList([]const u8) = .empty;
    var dir = b.build_root.handle.openDir(io, sub_path, .{ .iterate = true }) catch return &.{};
    defer dir.close(io);
    var walker = dir.walk(b.allocator) catch return &.{};
    defer walker.deinit();
    outer: while (walker.next(io) catch return sources.toOwnedSlice(b.allocator) catch &.{}) |entry| {
        if (entry.kind != .file or !std.mem.endsWith(u8, entry.basename, ".c")) continue;
        for (skip_files) |skip| {
            if (std.mem.eql(u8, entry.basename, skip)) continue :outer;
        }
        for (skip_dir_prefixes) |prefix| {
            if (std.mem.startsWith(u8, entry.path, prefix)) continue :outer;
        }
        sources.append(b.allocator, b.dupe(entry.path)) catch continue;
    }
    return sources.toOwnedSlice(b.allocator) catch &.{};
}

// X11/GL are system *shared* libraries -- must only ever be linked onto a module that becomes an
// executable, never onto one that becomes a static library (see buildSeika's Linux branch comment
// for why). Every final executable that links seika's rendering-enabled build on Linux (seika's
// own seika-test executables below, koa-game/scene_unit_test in koa/build.zig) calls this.
pub fn linkLinuxRenderingLibs(mod: *std.Build.Module, target: std.Build.ResolvedTarget) void {
    if (target.result.os.tag == .linux) {
        mod.linkSystemLibrary("X11", .{});
        mod.linkSystemLibrary("GL", .{});
    }
}

pub fn addSeikaDefines(mod: *std.Build.Module, use_main: bool) void {
    mod.addCMacro("SKA_ECS", "1");
    mod.addCMacro("SKA_INPUT", "1");
    mod.addCMacro("SKA_RENDERING", "1");
    mod.addCMacro("SKA_AUDIO", "1");
    mod.addCMacro("SKA_THREAD", "1");
    mod.addCMacro("SKA_NETWORK", "0");
    mod.addCMacro("SKA_HTTP_SERVER", "1");
    // seika_main.c's main() is only wanted by consumers that don't provide their own (e.g.
    // test_game) -- callers that already have their own entry point (koa's CLI runtime) must
    // pass false here, or linking fails with a duplicate 'main' symbol.
    mod.addCMacro("SKA_USE_MAIN", if (use_main) "1" else "0");
}

pub fn buildZip(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/zip/zip.c"),
        .flags = &.{"-std=gnu99"},
    });
    mod.addIncludePath(b.path("thirdparty/zip"));
    return b.addLibrary(.{ .name = "zip", .root_module = mod, .linkage = .static });
}

pub fn buildFreetype(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCMacro("FT2_BUILD_LIBRARY", "1");
    mod.addIncludePath(b.path("thirdparty/freetype/include"));
    mod.addCSourceFiles(.{
        .root = b.path("thirdparty/freetype/src"),
        .files = freetype_sources,
        .flags = &.{},
    });
    if (target.result.os.tag == .windows) {
        mod.addCSourceFile(.{
            .file = b.path("thirdparty/freetype/builds/windows/ftdebug.c"),
            .flags = &.{},
        });
    } else {
        mod.addCSourceFile(.{
            .file = b.path("thirdparty/freetype/src/base/ftdebug.c"),
            .flags = &.{},
        });
    }
    return b.addLibrary(.{ .name = "freetype", .root_module = mod, .linkage = .static });
}

pub fn buildGlad(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/glad/glad.c"),
        .flags = &.{},
    });
    mod.addIncludePath(b.path("thirdparty"));
    return b.addLibrary(.{ .name = "glad", .root_module = mod, .linkage = .static });
}

pub fn buildUnity(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/unity/unity.c"),
        .flags = &.{},
    });
    mod.addIncludePath(b.path("thirdparty/unity"));
    mod.addCMacro("UNITY_INCLUDE_DOUBLE", "1");
    mod.addCMacro("UNITY_SUPPORT_64", "1");
    return b.addLibrary(.{ .name = "unity", .root_module = mod, .linkage = .static });
}

pub fn buildStbImage(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/stb_image/stb_image.c"),
        .flags = &.{},
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/stb_image/stb_image_write_impl.c"),
        .flags = &.{},
    });
    mod.addIncludePath(b.path("thirdparty"));
    return b.addLibrary(.{ .name = "stb_image", .root_module = mod, .linkage = .static });
}

pub fn buildMsfGif(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/msf_gif/msf_gif_impl.c"),
        .flags = &.{},
    });
    mod.addIncludePath(b.path("thirdparty/msf_gif"));
    return b.addLibrary(.{ .name = "msf_gif", .root_module = mod, .linkage = .static });
}

pub fn buildCjson(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *std.Build.Step.Compile {
    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFile(.{
        .file = b.path("thirdparty/cjson/cJSON.c"),
        .flags = &.{},
    });
    mod.addIncludePath(b.path("thirdparty"));
    mod.addCMacro("CJSON_HIDE_SYMBOLS", "1");
    return b.addLibrary(.{ .name = "cjson", .root_module = mod, .linkage = .static });
}

pub fn buildSeika(
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    zip: *std.Build.Step.Compile,
    ft: *std.Build.Step.Compile,
    glad: *std.Build.Step.Compile,
    stb: *std.Build.Step.Compile,
    msf_gif: *std.Build.Step.Compile,
    use_main: bool,
) *std.Build.Step.Compile {
    const sources = collectCSources(b, "seika");

    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addCSourceFiles(.{
        .root = b.path("seika"),
        .files = sources,
        .flags = &.{"-std=gnu11"},
    });
    if (target.result.os.tag == .macos) {
        // window_cocoa.m/input_cocoa.m -- Objective-C, not picked up by collectCSources above.
        const objc_sources = collectObjCSources(b, "seika");
        mod.addCSourceFiles(.{
            .root = b.path("seika"),
            .files = objc_sources,
            .flags = &.{},
        });
    }
    mod.addIncludePath(b.path("."));
    mod.addIncludePath(b.path("thirdparty"));
    mod.addIncludePath(b.path("thirdparty/zip"));
    mod.addIncludePath(b.path("thirdparty/miniaudio"));
    mod.addIncludePath(b.path("thirdparty/cgltf"));
    mod.addIncludePath(b.path("thirdparty/freetype/include"));
    mod.addIncludePath(b.path("thirdparty/msf_gif"));
    addSeikaDefines(mod, use_main);
    mod.linkLibrary(zip);
    mod.linkLibrary(ft);
    mod.linkLibrary(glad);
    mod.linkLibrary(stb);
    mod.linkLibrary(msf_gif);
    if (target.result.os.tag == .windows) {
        mod.linkSystemLibrary("opengl32", .{});
        mod.linkSystemLibrary("gdi32", .{});
        mod.linkSystemLibrary("ws2_32", .{});
    } else if (target.result.os.tag == .macos) {
        // Matches CMakeLists.txt's APPLE branch. --sysroot (passed on the `zig build` command
        // line, e.g. `--sysroot "$(xcrun --show-sdk-path)"`) only gets Zig as far as finding
        // headers/libraries under the SDK root -- it does NOT automatically add
        // <sysroot>/System/Library/Frameworks as a framework search path, so linkFramework alone
        // still fails with "unable to find framework ... searched paths: none" even with a valid
        // --sysroot. b.sysroot is how build.zig reads back the --sysroot value that was passed in,
        // to derive that path explicitly.
        if (b.sysroot) |sysroot| {
            mod.addFrameworkPath(.{ .cwd_relative = b.pathJoin(&.{ sysroot, "System/Library/Frameworks" }) });
            // Cocoa.h -> Foundation.h -> Security.h -> oids.h pulls in <libDER/DERItem.h>, which
            // lives directly under the SDK's usr/include, not inside any framework's Headers dir.
            // --sysroot alone doesn't add <sysroot>/usr/include as a search path here (same class
            // of gap as the framework path above), so window_cocoa.m/input_cocoa.m fail with
            // "'libDER/DERItem.h' file not found" without this.
            mod.addSystemIncludePath(.{ .cwd_relative = b.pathJoin(&.{ sysroot, "usr/include" }) });
        }
        mod.linkFramework("OpenGL", .{});
        mod.linkFramework("Cocoa", .{});
        mod.linkFramework("CoreFoundation", .{});
    } else if (target.result.os.tag == .linux) {
        // X11/GL are deliberately NOT linked here (unlike opengl32/gdi32 on Windows and the
        // Cocoa/OpenGL frameworks on macOS above). Calling linkSystemLibrary for a *shared*
        // library (libX11.so/libGL.so) on a module that becomes a static-library artifact
        // (b.addLibrary(.linkage = .static) below) makes zig/LLD try to fold the resolved .so
        // path itself into the .a as an archive member ("archive member '.../libX11.so' is
        // neither ET_REL nor LLVM bitcode" -- LLD only warns, but zig's build system treats any
        // LLD stderr output as a hard failure). See koa/build.zig's linkLinuxRenderingLibs calls
        // (koa-game exe, scene_unit_test exe) and this file's own seika-test loop for where these
        // actually get linked -- always on the final *executable* module, never on a static lib.
        mod.linkSystemLibrary("m", .{});
    }
    return b.addLibrary(.{ .name = "seika", .root_module = mod, .linkage = .static });
}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const zip = buildZip(b, target, optimize);
    const ft = buildFreetype(b, target, optimize);
    const glad = buildGlad(b, target, optimize);
    const stb = buildStbImage(b, target, optimize);
    const msf_gif = buildMsfGif(b, target, optimize);
    // Off for consumers (koa's CLI runtime) that already have their own main() -- see the
    // SKA_USE_MAIN comment in addSeikaDefines. Default true preserves existing behavior for
    // direct `zig build`/`zig build seika-test` and other dependents (e.g. test_game) that rely
    // on seika_main.c's main().
    const use_main = b.option(bool, "use_main", "Compile seika_main.c's main() (off for consumers with their own entry point)") orelse true;
    const seika = buildSeika(b, target, optimize, zip, ft, glad, stb, msf_gif, use_main);
    b.installArtifact(seika);
    // Static libs don't bundle what they link -- installed so dependents can ship them (koa's SDK)
    b.installArtifact(ft);
    b.installArtifact(glad);
    b.installArtifact(stb);
    b.installArtifact(msf_gif);

    const cjson = buildCjson(b, target, optimize);
    b.installArtifact(cjson);

    const seika_test_step = b.step("seika-test", "Build and run seika unit tests");
    const unity = buildUnity(b, target, optimize);

    const test_sources = [_][]const u8{
        "test/seika_unit_test.c",
        "test/array_list_template_unit_test.c",
        "test/hash_map_template_unit_test.c",
        "test/event_template_unit_test.c",
        "test/math_unit_test.c",
    };
    const test_names = [_][]const u8{
        "seika_seika_unit_tests",
        "seika_array_list_template_unit_tests",
        "seika_hash_map_template_unit_tests",
        "seika_event_template_unit_tests",
        "seika_math_unit_tests",
    };
    for (test_sources, test_names) |src, name| {
        const test_mod = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        });
        test_mod.addCSourceFile(.{ .file = b.path(src), .flags = &.{"-std=c11"} });
        test_mod.addIncludePath(b.path("."));
        test_mod.addIncludePath(b.path("thirdparty/unity"));
        test_mod.addCMacro("UNITY_INCLUDE_DOUBLE", "1");
        test_mod.addCMacro("UNITY_SUPPORT_64", "1");
        test_mod.linkLibrary(seika);
        test_mod.linkLibrary(unity);
        // seika is always built with SKA_RENDERING=1 (see addSeikaDefines), so these test
        // executables need X11/GL themselves on Linux -- see linkLinuxRenderingLibs's doc comment.
        linkLinuxRenderingLibs(test_mod, target);
        const test_exe = b.addExecutable(.{ .name = name, .root_module = test_mod });
        const run_test = b.addRunArtifact(test_exe);
        seika_test_step.dependOn(&run_test.step);
    }
}
