const std = @import("std");
const mem = std.mem;
const os = std.os;
const shl = std.math.shl;

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

    pub fn readByte(self: *ROM, address: u24) u8 {
        return self.data[address];
    }

    pub fn readWord(self: *ROM, address: u24) u16 {
        return shl(u16, self.data[address + 1], usize(8)) |
                        self.data[address];
    }

    pub fn getBlockAddress(self: *ROM, i: u24) u24 {
        var bank_index: u24 = self.readByte(0x3E4E00 + i/2);
        if (i % 2 != 0) {
            bank_index >>= 4;
        }
        bank_index &= 0x0F;

        const bank   = self.readByte(0xEA91 + bank_index) - 0xC0;
        const offset = self.readWord(0x3E2E00 + i*2);

        return shl(u24, bank, usize(16)) | offset;
    }
};
