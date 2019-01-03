const std = @import("std");
const mem = std.mem;
const os = std.os;

pub const ROM = struct {
    allocator: *mem.Allocator,
    data: []u8,

    const MAX_SIZE = 6 * 1024 * 1024;

    pub fn fromFile(file: os.File, allocator: *mem.Allocator) !ROM {
        var file_stream = file.inStream();
        var stream = &file_stream.stream;
        var data = try stream.readAllAlloc(allocator, MAX_SIZE);

        return ROM {
            .allocator = allocator,
            .data      = data,
        };
    }

    pub fn deinit(self: *ROM) void {
        self.allocator.free(self.data);
    }
};
