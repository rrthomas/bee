use memoffset::{offset_of};

use mijit::code::{
    UnaryOp, BinaryOp, Width, AliasMask,
    Global, Marshal,
};
use mijit::target::{Target, Word};
use mijit::jit::{Jit, EntryId};
use UnaryOp::*;
use BinaryOp::*;
use Width::*;

mod registers;
pub use registers::{Registers, offsets, regs};
use regs::*;

mod enums;
pub use enums::{Insn1, Insn2};

//-----------------------------------------------------------------------------

/** The return code used to indicate normal exit from the hot code. */
const NOT_IMPLEMENTED: i64 = 0;
/** Dummy return code which should never actually occur. */
const UNDEFINED: i64 = i64::MAX;

/** AliasMask constants. */
mod am {
    use super::{AliasMask};

    pub const DATA_STACK: AliasMask = AliasMask(1);
    pub const RETURN_STACK: AliasMask = AliasMask(2);
    pub const MEMORY: AliasMask = AliasMask(4);
    pub const REGISTERS: AliasMask = AliasMask(8);
}

/** Number of VM instruction types. */
const NUM_OP1_INSNS: usize = 8;
/** Number of VM instruction opcodes. */
const NUM_OP2_INSNS: usize = 0x40;

mod builder;
use builder::{build, build_block, Builder};

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
            prologue: build_block(&|b| {
                for (reg, offset) in offsets() {
                    b.load(reg, (Global(0), offset as i64), Eight, am::REGISTERS);
                }
                b.const_binary(Sub, SP, SP, 1);
                b.const_binary(Sub, DP, DP, 1);
            }),
            epilogue: build_block(&|b| {
                b.const_binary(Add, SP, SP, 1);
                b.const_binary(Add, DP, DP, 1);
                for (reg, offset) in offsets() {
                    b.store(reg, (Global(0), offset as i64), Eight, am::REGISTERS);
                }
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

        let mut op1_insns: Vec<_> = (0..NUM_OP1_INSNS - 1).map(
            |_| build(&move |b| { b.jump(not_implemented) })
        ).collect();

        let mut op2_insns: Vec<_> = (0..NUM_OP2_INSNS - 1).map(
            |_| build(&move |b| { b.jump(not_implemented) })
        ).collect();

        // Helper functions.
        let pop = |b: &mut Builder<EntryId>, dest| {
            b.array_load(dest, (D0, DP), Eight, am::DATA_STACK);
            b.const_binary(Sub, DP, DP, 1);
        };
        let push = |b: &mut Builder<EntryId>, src| {
            b.const_binary(Add, DP, DP, 1);
            b.array_store(src, (D0, DP), Eight, am::DATA_STACK);
        };
        let pop_s = |b: &mut Builder<EntryId>, dest| {
            b.array_load(dest, (S0, SP), Eight, am::RETURN_STACK);
            b.const_binary(Sub, SP, SP, 1);
        };
        let push_s = |b: &mut Builder<EntryId>, src| {
            b.const_binary(Add, SP, SP, 1);
            b.array_store(src, (S0, SP), Eight, am::RETURN_STACK);
        };
        let load = |width: Width| build(&move |mut b| {
            pop(&mut b, R1);
            b.const_binary(And, TEST, R1, (1 << (width as usize)) - 1);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, DP, DP, 1);
                b.jump(not_implemented)
            }));
            b.load(R1, (R1, 0), width, am::MEMORY);
            push(&mut b, R1);
            b.jump(root)
        });
        let store = |width: Width| build(&move |mut b| {
            pop(&mut b, R1);
            b.const_binary(And, TEST, R1, (1 << (width as usize)) - 1);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, DP, DP, 1);
                b.jump(not_implemented)
            }));
            pop(&mut b, R2);
            b.store(R2, (R1, 0), width, am::MEMORY);
            b.jump(root)
        });
        let unary = |op: UnaryOp| build(&move |mut b| {
            pop(&mut b, R1);
            b.unary(op, R1, R1);
            push(&mut b, R1);
            b.jump(root)
        });
        let binary_with_callback = |callback: &dyn Fn(&mut Builder<EntryId>)| build(&move |mut b| {
            pop(&mut b, R1);
            pop(&mut b, R2);
            callback(&mut b);
            push(&mut b, R1);
            b.jump(root)
        });
        let binary = |op: BinaryOp| binary_with_callback(&|b| {
            b.binary(op, R1, R2, R1);
        });
        let compare = |op: BinaryOp| binary_with_callback(&|b| {
            b.binary(op, R1, R2, R1);
            b.unary(Negate, R1, R1);
        });

        // Define the first level instructions.
        op1_insns[Insn1::CallI as usize] = build(&move |mut b| {
            push_s(&mut b, PC);
            b.const_binary(And, R1, OPCODE, !0x7);
            b.binary(Add, PC, PC, R1);
            b.const_binary(Sub, PC, PC, 8);
            b.jump(root)
        });
        op1_insns[Insn1::PushI as usize] = build(&move |mut b| {
            b.const_binary(Asr, R1, OPCODE, 3);
            push(&mut b, R1);
            b.jump(root)
        });
        op1_insns[Insn1::PushrelI as usize] = build(&move |mut b| {
            b.const_binary(And, R1, OPCODE, !0x7);
            b.binary(Add, R1, PC, R1);
            b.const_binary(Sub, R1, R1, 8);
            push(&mut b, R1);
            b.jump(root)
        });
        op1_insns[Insn1::JumpI as usize] = build(&move |mut b| {
            b.const_binary(And, R1, OPCODE, !0x7);
            b.binary(Add, PC, PC, R1);
            b.const_binary(Sub, PC, PC, 8);
            b.jump(root)
        });
        op1_insns[Insn1::JumpzI as usize] = build(&move |mut b| {
            pop(&mut b, TEST);
            b.guard(TEST, false, build(&move |b| { b.jump(root) }));
            b.const_binary(And, R1, OPCODE, !0x7);
            b.binary(Add, PC, PC, R1);
            b.const_binary(Sub, PC, PC, 8);
            b.jump(root)
        });

        // Define the second level instructions.
        op2_insns[Insn2::Nop as usize] = build(&move |b| {
            b.jump(root)
        });
        op2_insns[Insn2::Not as usize] = unary(Not);
        op2_insns[Insn2::And as usize] = binary(And);
        op2_insns[Insn2::Or as usize] = binary(Or);
        op2_insns[Insn2::Xor as usize] = binary(Xor);
        op2_insns[Insn2::LShift as usize] = binary(Lsl);
        op2_insns[Insn2::RShift as usize] = binary(Lsr);
        op2_insns[Insn2::ARShift as usize] = binary(Asr);
        op2_insns[Insn2::Pop as usize] = build(&move |mut b| {
            b.const_binary(Sub, DP, DP, 1);
            b.jump(root)
        });
        // TODO: Specialize for small arguments.
        op2_insns[Insn2::Dup as usize] = build(&move |mut b| {
            pop(&mut b, TEST);
            b.binary(Sub, TEST, DP, TEST);
            b.array_load(R2, (D0, TEST), Eight, am::DATA_STACK);
            push(&mut b, R2);
            b.jump(root)
        });
        // TODO: Specialize for small arguments.
        op2_insns[Insn2::Set as usize] = build(&move |mut b| {
            pop(&mut b, TEST);
            pop(&mut b, R1);
            b.binary(Sub, TEST, DP, TEST);
            b.array_store(R1, (D0, TEST), Eight, am::DATA_STACK);
            b.jump(root)
        });
        // TODO: Specialize for small arguments.
        op2_insns[Insn2::Swap as usize] = build(&move |mut b| {
            pop(&mut b, TEST);
            pop(&mut b, R1);
            b.binary(Sub, TEST, DP, TEST);
            b.array_load(R2, (D0, TEST), Eight, am::DATA_STACK);
            b.array_store(R1, (D0, TEST), Eight, am::DATA_STACK);
            push(&mut b, R2);
            b.jump(root)
        });
        op2_insns[Insn2::Jump as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            b.const_binary(And, TEST, R1, 7);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, DP, DP, 1);
                b.jump(not_implemented)
            }));
            b.move_(PC, R1);
            b.jump(root)
        });
        op2_insns[Insn2::JumpZ as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            pop(&mut b, TEST);
            b.guard(TEST, false, build(&move |b| { b.jump(root) }));
            b.const_binary(And, TEST, R1, 7);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, DP, DP, 2);
                b.jump(not_implemented)
            }));
            b.move_(PC, R1);
            b.jump(root)
        });
        op2_insns[Insn2::Call as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            b.const_binary(And, TEST, R1, 7);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, DP, DP, 1);
                b.jump(not_implemented)
            }));
            push_s(&mut b, PC);
            b.move_(PC, R1);
            b.jump(root)
        });
        op2_insns[Insn2::Ret as usize] = build(&move |mut b| {
            pop_s(&mut b, R1);
            b.const_binary(And, TEST, R1, 7);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, SP, SP, 1);
                b.jump(not_implemented)
            }));
            b.load(R2, (Global(0), offset_of!(Registers, handler_sp) as i64), Eight, am::REGISTERS);
            b.binary(Ult, TEST, SP, R2);
            b.guard(TEST, false, build(&move |mut b| {
                b.const_binary(Add, SP, SP, 1);
                b.jump(not_implemented)
            }));
            b.move_(PC, R1);
            b.jump(root)
        });
        op2_insns[Insn2::Load as usize] = load(Eight);
        op2_insns[Insn2::Store as usize] = store(Eight);
        op2_insns[Insn2::Load1 as usize] = load(One);
        op2_insns[Insn2::Store1 as usize] = store(One);
        op2_insns[Insn2::Load2 as usize] = load(Two);
        op2_insns[Insn2::Store2 as usize] = store(Two);
        op2_insns[Insn2::Load4 as usize] = load(Four);
        op2_insns[Insn2::Store4 as usize] = store(Four);
        op2_insns[Insn2::Neg as usize] = unary(Negate);
        op2_insns[Insn2::Add as usize] = binary(Add);
        op2_insns[Insn2::Mul as usize] = binary(Mul);
        op2_insns[Insn2::DivMod as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            pop(&mut b, R2);
            b.const_binary(Eq, TEST, R2, i64::MIN);
            b.const_binary(Eq, OPCODE, R1, -1);
            b.binary(And, TEST, TEST, OPCODE);
            b.guard(TEST, false, build(&move |mut b| {
                push(&mut b, R2); // Known to be i64::MIN.
                b.const_(R2, 0);
                push(&mut b, R2);
                b.jump(root)
            }));
            b.guard(R1, true, build(&move |mut b| {
                push(&mut b, R1); // Known to be 0.
                push(&mut b, R2); // The numerator.
                b.jump(root)
            }));
            b.binary(SDiv, TEST, R2, R1);
            push(&mut b, TEST);
            b.binary(Mul, TEST, TEST, R1);
            b.binary(Sub, TEST, R2, TEST);
            push(&mut b, TEST);
            b.jump(root)
        });
        op2_insns[Insn2::UDivMod as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            pop(&mut b, R2);
            b.guard(R1, true, build(&move |mut b| {
                push(&mut b, R1); // Known to be 0.
                push(&mut b, R2); // The numerator.
                b.jump(root)
            }));
            b.binary(UDiv, TEST, R2, R1);
            push(&mut b, TEST);
            b.binary(Mul, TEST, TEST, R1);
            b.binary(Sub, TEST, R2, TEST);
            push(&mut b, TEST);
            b.jump(root)
        });
        op2_insns[Insn2::Eq as usize] = compare(Eq);
        op2_insns[Insn2::Lt as usize] = compare(Lt);
        op2_insns[Insn2::ULt as usize] = compare(Ult);
        op2_insns[Insn2::PushR as usize] = build(&move |mut b| {
            pop(&mut b, R1);
            push_s(&mut b, R1);
            b.jump(root)
        });
        op2_insns[Insn2::PopR as usize] = build(&move |mut b| {
            pop_s(&mut b, R1);
            push(&mut b, R1);
            b.jump(root)
        });
        op2_insns[Insn2::DupR as usize] = build(&move |mut b| {
            b.array_load(R1, (S0, SP), Eight, am::RETURN_STACK);
            push(&mut b, R1);
            b.jump(root)
        });

        // Main dispatch loop.
        let op1_insns = &op1_insns;
        let op2_insns = &op2_insns;
        jit.define(root, &build(&move |mut b| {
            b.pop(OPCODE, PC, am::MEMORY);
            b.const_binary(And, TEST, OPCODE, (NUM_OP1_INSNS - 1) as i64);
            b.index(
                TEST,
                op1_insns.clone().into(),
                build(&move |mut b| {
                    b.const_binary(Lsr, OPCODE, OPCODE, 3);
                    b.const_binary(And, TEST, OPCODE, (NUM_OP2_INSNS - 1) as i64);
                    b.index(
                        TEST,
                        op2_insns.clone().into(),
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
