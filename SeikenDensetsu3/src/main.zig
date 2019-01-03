const std = @import("std");
const os = std.os;
const assert = std.debug.assert;
const warn = std.debug.warn;

pub fn main() !void {
    // Initialize allocator.
    var direct_allocator = std.heap.DirectAllocator.init();
    defer direct_allocator.deinit();
    var arena = std.heap.ArenaAllocator.init(&direct_allocator.allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    // Fetch arguments.
    const args = try os.argsAlloc(allocator);
    defer os.argsFree(allocator, args);
    assert(args.len == 2);

    warn("{}", args[1]);
}
