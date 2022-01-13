use mijit::target::{Native, native};

mod machine;
pub use machine::{Registers, Bee};

pub struct Jit {
    /** The compiled code. */
    bee: Option<Bee<Native>>,
}

/** Allocates a new Jit. */
#[no_mangle]
pub extern fn mijit_bee_new() -> Box<Jit> {
    Box::new(Jit {bee: Some(Bee::new(native()))})
}

/** Frees a Jit. */
#[no_mangle]
pub extern fn mijit_bee_drop(_vm: Box<Jit>) {}

/** Run the code at address `registers.ep`. */
#[no_mangle]
pub unsafe extern fn mijit_bee_run(jit: &mut Jit, registers: &mut Registers) {
    let bee = jit.bee.take().expect("Trying to call run() after error");
    let bee = bee.run(registers).expect("Execute failed");
    jit.bee = Some(bee);
}
