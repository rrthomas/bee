use mijit::target::{Native, native};

mod machine;
pub use machine::{Registers, Bee};

pub type Jit = Bee<Native>;

/** Allocates a new Jit. */
#[no_mangle]
pub extern fn mijit_bee_new() -> Box<Jit> {
    Box::new(Bee::new(native()))
}

/** Frees a Jit. */
#[no_mangle]
pub extern fn mijit_bee_drop(_vm: Box<Jit>) {}

/** Run the code at address `registers.ep`. */
#[no_mangle]
pub unsafe extern fn mijit_bee_run(jit: &mut Jit, registers: &mut Registers) {
    jit.run(registers);
}
