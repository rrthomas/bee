use memoffset::{offset_of};

use mijit::code::{Register, REGISTERS};

#[repr(C)]
pub struct Registers {
    pub pc: *const i64,
    pub s0: *mut i64,
    pub ssize: u64,
    pub sp: u64,
    pub d0: *mut i64,
    pub dsize: u64,
    pub dp: u64,
    pub handler_sp: u64,
}

/**
 * Native registers used by the VM.
 *
 * We avoid `REGISTER[0]`, which is used as scratch space.
 */
pub mod regs {
    use super::{Register, REGISTERS};

    pub const R1: Register = REGISTERS[1];
    pub const R2: Register = REGISTERS[2];
    pub const PC: Register = REGISTERS[3];
    pub const S0: Register = REGISTERS[4];
    /** "Bee register SP" - 1. */
    pub const SP: Register = REGISTERS[5];
    pub const SSIZE: Register = REGISTERS[6];
    pub const D0: Register = REGISTERS[7];
    /** "Bee register DP" - 1. */
    pub const DP: Register = REGISTERS[8];
    pub const DSIZE: Register = REGISTERS[9];
    pub const OPCODE: Register = REGISTERS[10];
    pub const TEST: Register = REGISTERS[11];
}
use regs::*;

/** Bee registers held in native registers. */
pub fn offsets() -> Vec<(Register, usize)> {
    vec![
        (PC, offset_of!(Registers, pc)),
        (S0, offset_of!(Registers, s0)),
        (SP, offset_of!(Registers, sp)),
        (SSIZE, offset_of!(Registers, ssize)),
        (D0, offset_of!(Registers, d0)),
        (DP, offset_of!(Registers, dp)),
        (DSIZE, offset_of!(Registers, dsize)),
    ]
}
