mod registers;
pub use registers::{Registers};
use mijit::code::{Global, Marshal, Register, REGISTERS};
use mijit::target::{Target, Word};
use mijit::jit::{Jit, EntryId};

//-----------------------------------------------------------------------------

const _TEMP: Register = REGISTERS[0];
const _R1: Register = REGISTERS[1];

/* mod builder;
use builder::{build, marshal}; */

/** The return code used to indicate normal exit from the hot code. */
const NOT_IMPLEMENTED: i64 = 0;
/** Dummy return code which should never actually occur. */
const _UNDEFINED: i64 = i64::MAX;

//-----------------------------------------------------------------------------

/** The performance-critical part of the virtual machine. */
#[derive(Debug)]
pub struct Bee<T: Target> {
    pub jit: Jit<T>,
    pub root: EntryId,
}

impl<T: Target> Bee<T> {
    pub fn new(target: T) -> Self {
        let mut jit = Jit::new(target, 1);
        let marshal = Marshal {prologue: Box::new([]), epilogue: Box::new([])};
        let root = jit.new_entry(&marshal, NOT_IMPLEMENTED);
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
