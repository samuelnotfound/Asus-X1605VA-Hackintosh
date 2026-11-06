const std = @import("std");
const u = std.os.uefi;
const hid = @import("I2cIo.zig");

const State = extern struct { x: i32, y: i32, z: i32, left: bool, right: bool };
const Mode = extern struct { x: u64, y: u64, z: u64, left: bool, right: bool };
const Pointer = extern struct {
    reset: *const fn (*Pointer, bool) callconv(u.cc) u.Status,
    get_state: *const fn (*Pointer, *State) callconv(u.cc) u.Status,
    wait: u.Event,
    mode: *Mode,
};
const pointer_guid align(8) = u.protocol.SimplePointer.guid;
var mode: Mode = .{ .x = 4, .y = 4, .z = 0, .left = true, .right = true };
var pointer: Pointer = .{ .reset = reset, .get_state = getState, .wait = undefined, .mode = &mode };
var state: State = std.mem.zeroes(State);
var ready = false;
var io: *hid.I2cIo = undefined;
var completion: u.Event = undefined;
var timer: u.Event = undefined;
var pending = false;
var result: u.Status = .success;
var report: [16]u8 = undefined;
var packet: hid.Packet = .{ .count = 1, .operations = .{
    .{ .flags = 1, .length = report.len, .data = &report },
    undefined,
} };

fn bs() *u.tables.BootServices {
    return u.system_table.boot_services.?;
}

fn reset(_: *Pointer, _: bool) callconv(u.cc) u.Status {
    const old = bs().raiseTpl(16);
    defer bs().restoreTpl(old);
    state = std.mem.zeroes(State);
    ready = false;
    return .success;
}

fn getState(_: *Pointer, out: *State) callconv(u.cc) u.Status {
    const old = bs().raiseTpl(16);
    defer bs().restoreTpl(old);
    if (!ready) return .not_ready;
    out.* = state;
    state.x = 0;
    state.y = 0;
    ready = false;
    return .success;
}

fn waitForInput(event: u.Event, _: ?*anyopaque) callconv(u.cc) void {
    if (ready) _ = bs().signalEvent(event);
}

fn finished(_: u.Event, _: ?*anyopaque) callconv(u.cc) void {
    pending = false;
    if (result != .success or report[0] != 11 or report[1] != 0 or report[2] != 1) return;
    const dx: i8 = @bitCast(report[4]);
    const dy: i8 = @bitCast(report[5]);
    const left = report[3] & 1 != 0;
    const right = report[3] & 2 != 0;
    const old = bs().raiseTpl(16);
    defer bs().restoreTpl(old);
    if (dx == 0 and dy == 0 and left == state.left and right == state.right) return;
    state.x +|= dx;
    state.y +|= dy;
    state.left = left;
    state.right = right;
    ready = true;
}

fn tick(_: u.Event, _: ?*anyopaque) callconv(u.cc) void {
    if (pending) return;
    pending = true;
    result = .not_ready;
    report = @splat(0);
    if (io.queue(io, 0, completion, &packet, &result) != .success) pending = false;
}

pub fn main() u.Status {
    io = hid.findTouchpad() orelse return .not_found;
    var status = bs().createEvent(0x100, 8, waitForInput, null, &pointer.wait);
    if (status != .success) return status;

    status = bs().createEvent(0x200, 8, finished, null, &completion);
    if (status != .success) {
        _ = bs().closeEvent(pointer.wait);
        return status;
    }
    status = bs().createEvent(0x80000200, 8, tick, null, &timer);
    if (status != .success) {
        _ = bs().closeEvent(completion);
        _ = bs().closeEvent(pointer.wait);
        return status;
    }
    // Defer the first request until after successful protocol installation.
    // TPL_CALLBACK keeps the timer from firing during setup.
    const old = bs().raiseTpl(8);
    defer bs().restoreTpl(old);
    status = bs().setTimer(timer, .timer_periodic, 200000);
    if (status == .success) {
        // Zig 0.14.1 declares this parameter as Handle; UEFI takes *Handle.
        const install: *const fn (*u.Handle, *align(8) const u.Guid, u.tables.InterfaceType, *anyopaque) callconv(u.cc) u.Status = @ptrCast(bs().installProtocolInterface);
        var image_handle = u.handle;
        status = install(&image_handle, &pointer_guid, .efi_native_interface, &pointer);
    }

    if (status != .success) {
        _ = bs().closeEvent(timer);
        _ = bs().closeEvent(completion);
        _ = bs().closeEvent(pointer.wait);
    }
    return status;
}
