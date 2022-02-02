use memoffset::{offset_of};

mod registers;
pub use registers::{Registers};
use mijit::code::{
    BinaryOp, Precision, Width, AliasMask,
    Global, Register, REGISTERS,
    Action, Switch, EBB, Ending, Marshal,
};
use mijit::target::{Target, Word};
use mijit::jit::{Jit, EntryId};
use BinaryOp::*;
use Precision::*;

//-----------------------------------------------------------------------------

const TEMP: Register = REGISTERS[0];
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
            prologue: Box::new([
                Action::Constant(P64, TEMP, offset_of!(Registers, pc) as i64),
                Action::Binary(Add, P64, TEMP, Global(0).into(), TEMP.into()),
                Action::Load(PC, (TEMP.into(), Width::Eight), am::REGISTERS),
            ]),
            epilogue: Box::new([
                Action::Constant(P64, TEMP, offset_of!(Registers, pc) as i64),
                Action::Binary(Add, P64, TEMP, Global(0).into(), TEMP.into()),
                Action::Store(TEMP, PC.into(), (TEMP.into(), Width::Eight), am::REGISTERS),
            ]),
        };
        let root = jit.new_entry(&marshal, UNDEFINED);

        // Not implemented.
        let not_implemented2 = jit.new_entry(&marshal, NOT_IMPLEMENTED);
        let not_implemented = jit.new_entry(&marshal, UNDEFINED);
        jit.define(not_implemented, &EBB {
            actions: vec![
                Action::Constant(P64, TEMP, 8),
                Action::Binary(Sub, P64, PC, PC.into(), TEMP.into()),
            ],
            ending: Ending::Leaf(not_implemented2),
        });

        let op_insns: Vec<_> = (0..NUM_OP_INSNS - 1).map(
            |_| EBB {actions: vec![], ending: Ending::Leaf(not_implemented)}
        ).collect();

        let op_types: Vec<_> = (0..NUM_OP_TYPES - 1).map(
            |_| EBB {actions: vec![], ending: Ending::Leaf(not_implemented)}
        ).collect();

        // Main dispatch loop.
        jit.define(root, &EBB {
            actions: vec![
                Action::Load(OPCODE, (PC.into(), Width::Eight), am::MEMORY),
                Action::Constant(P64, TEMP, 8),
                Action::Binary(Add, P64, PC, PC.into(), TEMP.into()),
                Action::Constant(P64, TEMP, (NUM_OP_TYPES - 1) as i64),
                Action::Binary(And, P64, TEMP, OPCODE.into(), TEMP.into()),
            ],
            ending: Ending::Switch(Switch::new(
                TEMP.into(),
                op_types.into(),
                EBB {
                    actions: vec![
                        Action::Constant(P64, TEMP, 3),
                        Action::Binary(Lsr, P64, OPCODE, OPCODE.into(), TEMP.into()),
                        Action::Constant(P64, TEMP, (NUM_OP_INSNS - 1) as i64),
                        Action::Binary(And, P64, TEMP, OPCODE.into(), TEMP.into()),
                    ],
                    ending: Ending::Switch(Switch::new(
                        TEMP.into(),
                        op_insns.into(),
                        EBB {actions: vec![], ending: Ending::Leaf(not_implemented)},
                    )),
                },
            )),
        });
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
