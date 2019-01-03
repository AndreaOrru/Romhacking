const std = @import("std");
const os = std.os;
const assert = std.debug.assert;
const warn = std.debug.warn;

const ROM = @import("rom.zig").ROM;

pub fn main() !void {
    // Initialize allocator.
    var direct_allocator = std.heap.DirectAllocator.init();
    defer direct_allocator.deinit();
    var arena = std.heap.ArenaAllocator.init(&direct_allocator.allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    // Allocate arguments.
    const args = try os.argsAlloc(allocator);
    defer os.argsFree(allocator, args);
    assert(args.len == 2);

    // Open ROM file.
    const filename = args[1];
    var file = try os.File.openRead(filename);
    defer file.close();

    // Load ROM.
    var rom = try ROM.fromFile(file, allocator);
    defer rom.deinit();

    warn("{}\n", rom.getBlockAddress(0));
}
