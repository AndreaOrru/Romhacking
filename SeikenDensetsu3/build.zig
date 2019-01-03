const Builder = @import("std").build.Builder;

pub fn build(b: *Builder) void {
    const seiken = b.addExecutable("seiken", "src/main.zig");
    seiken.setBuildMode(b.standardReleaseOptions());
    seiken.setOutputPath("seiken");

    b.default_step.dependOn(&seiken.step);
}
