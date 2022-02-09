use memoffset::{offset_of};

mod registers;
pub use registers::{Registers};
use mijit::code::{
    BinaryOp, Width, AliasMask,
    Global, Register, REGISTERS,
    Marshal,
};
use mijit::target::{Target, Word};
use mijit::jit::{Jit, EntryId};
use BinaryOp::*;

//-----------------------------------------------------------------------------

const PC: Register = REGISTERS[1];
const OPCODE: Register = REGISTERS[2];

/* mod builder;
use builder::{build, marshal}; */

/** The return code used to indicate normal exit from the hot code. */
const NOT_IMPLEMENTED: i64 = 0;
/** Dummy return code which should never actually occur. */
const UNDEFINED: i64 = i64::MAX;

/** AliasMask constants. */
mod am {
    use super::{AliasMask};

    pub const _DATA_STACK: AliasMask = AliasMask(1);
    pub const _RETURN_STACK: AliasMask = AliasMask(2);
    pub const MEMORY: AliasMask = AliasMask(4);
    pub const REGISTERS: AliasMask = AliasMask(8);
}

/** Number of VM instruction types. */
const NUM_OP_TYPES: usize = 8;
/** Number of VM instruction opcodes. */
const NUM_OP_INSNS: usize = 0x40;

mod builder;
use builder::{TEMP, build};

//-----------------------------------------------------------------------------

/** The performance-critical part of the virtual machine. */
#[derive(Debug)]
pub struct Bee<T: Target> {
    pub jit: Jit<T>,
    pub root: EntryId,
}

impl<T: Target> Bee<T> {
    #[allow(clippy::too_many_lines)]
    pub fn new(target: T) -> Self {
        let mut jit = Jit::new(target, 1);
        let marshal = Marshal {
            prologue: build(&|mut b| {
                b.load(PC, (Global(0), offset_of!(Registers, pc) as i64), Width::Eight, am::REGISTERS);
                b.actions.into()
            }),
            epilogue: build(&|mut b| {
                b.store(TEMP, PC, (Global(0), offset_of!(Registers, pc) as i64), Width::Eight, am::REGISTERS);
                b.actions.into()
            }),
        };
        let root = jit.new_entry(&marshal, UNDEFINED);

        // Not implemented.
        let not_implemented2 = jit.new_entry(&marshal, NOT_IMPLEMENTED);
        let not_implemented = jit.new_entry(&marshal, UNDEFINED);
        jit.define(not_implemented, &build(&move |mut b| {
            b.const_binary(Sub, PC, PC, 8);
            b.jump(not_implemented2)
        }));

        let op_insns: &Vec<_> = &(0..NUM_OP_INSNS - 1).map(
            |_| build(&move |b| { b.jump(not_implemented) })
        ).collect();

        let op_types: &Vec<_> = &(0..NUM_OP_TYPES - 1).map(
            |_| build(&move |b| { b.jump(not_implemented) })
        ).collect();

        // Main dispatch loop.
        jit.define(root, &build(&move |mut b| {
            b.pop(OPCODE, PC, am::MEMORY);
            b.const_binary(And, TEMP, OPCODE, (NUM_OP_TYPES - 1) as i64);
            b.index(
                TEMP,
                op_types.clone().into(),
                build(&move |mut b| {
                    b.const_binary(Lsr, OPCODE, OPCODE, 3);
                    b.const_binary(And, TEMP, OPCODE, (NUM_OP_INSNS - 1) as i64);
                    b.index(
                        TEMP,
                        op_insns.clone().into(),
                        build(&move |b| { b.jump(not_implemented) }),
                    )
                })
            )
        }));
        Bee {jit, root}
    }

    pub unsafe fn run(mut self, registers: &mut Registers) -> std::io::Result<Self> {
        *self.jit.global_mut(Global(0)) = Word {mp: (registers as *mut Registers).cast()};
        let (jit, result) = self.jit.run(self.root)?;
        assert_eq!(result, Word {s: NOT_IMPLEMENTED});
        self.jit = jit;
        Ok(self)
    }
}
