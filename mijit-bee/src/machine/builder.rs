use mijit::code::{
    UnaryOp, BinaryOp, Precision, Width, AliasMask,
    Register, REGISTERS, Variable,
    Action, Switch, EBB, Ending,
};
use Precision::*;
use BinaryOp::*;
use Width::*;

const TEMP: Register = REGISTERS[0];

/** Build an EBB. Equivalent to `callback(Builder::new())`. */
pub fn build<T>(callback: &dyn Fn(Builder<T>) -> EBB<T>) -> EBB<T> {
    callback(Builder::new())
}

/** Similar to `build()` but only builds a basic block. */
pub fn build_block(callback: &dyn Fn(&mut Builder<()>)) -> Box<[Action]> {
    let mut b = Builder::new();
    callback(&mut b);
    b.actions.into()
}

//-----------------------------------------------------------------------------

/** Represents everything that was built up to and including a [`guard()`]. */
#[derive(Debug)]
pub struct Guard<T> {
    pub actions: Vec<Action>,
    pub condition: Variable,
    pub expected: bool,
    pub if_fail: EBB<T>,
}

//-----------------------------------------------------------------------------

/**
 * A utility for building [`EBB`]s. `T` is usually [`EntryId`].
 *
 * [`EntryId`]: mijit::jit::EntryId
 */
#[derive(Debug)]
pub struct Builder<T> {
    /** All [`Action`]s generated since the last call to `guard()`. */
    pub actions: Vec<Action>,
    /** One per call to `guard()`. */
    pub guards: Vec<Guard<T>>,
}

impl<T> Builder<T> {
    pub fn new() -> Self {
        Builder {actions: Vec::new(), guards: Vec::new()}
    }

    pub fn move_(&mut self, dest: impl Into<Variable>, src: impl Into<Variable>) {
        self.actions.push(Action::Move(dest.into(), src.into()));
    }

    pub fn const_(&mut self, dest: impl Into<Register>, value: i64) {
        self.actions.push(Action::Constant(P64, dest.into(), value));
    }

    pub fn unary(
        &mut self,
        op: UnaryOp,
        dest: impl Into<Register>,
        src: impl Into<Variable>,
    ) {
        self.actions.push(Action::Unary(op, P64, dest.into(), src.into()));
    }

    pub fn binary(
        &mut self,
        op: BinaryOp,
        dest: impl Into<Register>,
        src1: impl Into<Variable>,
        src2: impl Into<Variable>,
    ) {
        self.actions.push(Action::Binary(op, P64, dest.into(), src1.into(), src2.into()));
    }

    /** [`TEMP`] is corrupted if `dest == src`. */
    pub fn const_binary(
        &mut self,
        op: BinaryOp,
        dest: impl Into<Register>,
        src: impl Into<Variable>,
        value: i64,
    ) {
        let dest = dest.into();
        let src = src.into();
        let temp = if src != dest.into() {
            dest
        } else if src != TEMP.into() {
            TEMP
        } else {
            panic!("dest and src cannot both be TEMP");
        };
        self.const_(temp, value);
        self.binary(op, dest, src, temp);
    }

    /** [`TEMP`] is corrupted if `dest == addr.0`. */
    pub fn load(
        &mut self,
        dest: impl Into<Register>,
        addr: (impl Into<Variable>, i64),
        width: Width,
        am: AliasMask,
    ) {
        let dest = dest.into();
        self.const_binary(Add, dest, addr.0, addr.1);
        self.actions.push(Action::Load(dest, (dest.into(), width), am));
    }

    /** [`TEMP`] is corrupted. */
    pub fn store(
        &mut self,
        src: impl Into<Variable>,
        addr: (impl Into<Variable>, i64),
        width: Width,
        am: AliasMask,
    ) {
        self.const_binary(Add, TEMP, addr.0, addr.1);
        self.actions.push(Action::Store(TEMP, src.into(), (TEMP.into(), width), am));
    }

    /** [`TEMP`] is corrupted. */
    pub fn array_load(
        &mut self,
        dest: impl Into<Register>,
        addr: (impl Into<Variable>, impl Into<Variable>),
        width: Width,
        am: AliasMask,
    ) {
        self.const_binary(Lsl, TEMP, addr.1, width as i64);
        self.binary(Add, TEMP, addr.0, TEMP);
        self.actions.push(Action::Load(dest.into(), (TEMP.into(), width), am));
    }

    /** [`TEMP`] is corrupted. */
    pub fn array_store(
        &mut self,
        src: impl Into<Variable>,
        addr: (impl Into<Variable>, impl Into<Variable>),
        width: Width,
        am: AliasMask,
    ) {
        self.const_binary(Lsl, TEMP, addr.1, width as i64);
        self.binary(Add, TEMP, addr.0, TEMP);
        self.actions.push(Action::Store(TEMP, src.into(), (TEMP.into(), width), am));
    }

    /** [`TEMP`] is corrupted. */
    pub fn pop(
        &mut self,
        dest: impl Into<Register>,
        sp: impl Into<Register>,
        am: AliasMask,
    ) {
        let dest = dest.into();
        let sp = sp.into();
        self.load(dest, (sp, 0), Eight, am);
        self.const_binary(Add, sp, sp, 8);
    }

    /** [`TEMP`] is corrupted. */
    #[allow(unused)]
    pub fn push(
        &mut self,
        src: impl Into<Variable>,
        sp: impl Into<Register>,
        am: AliasMask,
    ) {
        let src = src.into();
        let sp = sp.into();
        self.const_binary(Sub, sp, sp, 8);
        self.store(src, (sp, 0), Eight, am);
    }

    #[allow(unused)]
    pub fn debug(&mut self, src: impl Into<Variable>) {
        self.actions.push(Action::Debug(src.into()));
    }

    /**
     * If `condition` is not `expected`, abort by running `if_fail`.
     * See also [`if_()`] which is more symmetrical.
     */
    pub fn guard(&mut self, condition: impl Into<Variable>, expected: bool, if_fail: EBB<T>) {
        let mut actions = Vec::new();
        std::mem::swap(&mut actions, &mut self.actions);
        self.guards.push(Guard {actions, condition: condition.into(), expected, if_fail});
    }

    /**
     * Consume this `Builder` and return the finished `EBB`.
     * Usually, you will prefer to call one of [`jump()`], [`index()`] or
     * [`if_()`] which call this.
     */
    pub fn ending(mut self, ending: Ending<T>) -> EBB<T> {
        let mut ret = EBB {actions: self.actions, ending};
        while let Some(Guard {actions, condition, expected, if_fail}) = self.guards.pop() {
            let switch = if expected {
                Switch::if_(condition, ret, if_fail)
            } else {
                Switch::if_(condition, if_fail, ret)
            };
            ret = EBB {actions, ending: Ending::Switch(switch)};
        }
        ret
    }

    /** Jump to `target`. Equivalent to `ending(Ending::Leaf(target))`. */
    pub fn jump(self, target: T) -> EBB<T> {
        self.ending(Ending::Leaf(target))
    }

    /**
     * Select a continuation based on `switch`.
     * Equivalent to `ending(Ending::Leaf(target))`.
     * Usually, you will prefer to call one of [`index()`] or [`if_()`] which
     * call this.
     */
    pub fn switch(self, switch: Switch<EBB<T>>) -> EBB<T> {
        self.ending(Ending::Switch(switch))
    }

    /**
     * Select one of `cases` based on `discriminant`.
     * Select `default_` if `discriminant` exceeds `cases.len()`.
     * Equivalent to `switch(Switch::new(discriminant, cases, default_))`.
     */
    pub fn index(
        self,
        discriminant: impl Into<Variable>,
        cases: Box<[EBB<T>]>,
        default_: EBB<T>,
    ) -> EBB<T> {
        self.switch(Switch::new(discriminant.into(), cases, default_))
    }

    /**
     * Select `if_true` if `condition` is non-zero, otherwise `if_false`.
     * Equivalent to `switch(Switch::new(condition, if_true, if_false))`.
     * See also `guard()` which favours one outcome.
     */
    #[allow(unused)]
    pub fn if_(
        self,
        condition: impl Into<Variable>,
        if_true: EBB<T>,
        if_false: EBB<T>,
    ) -> EBB<T> {
        self.switch(Switch::if_(condition.into(), if_true, if_false))
    }
}
