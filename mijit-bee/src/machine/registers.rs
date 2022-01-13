#[repr(C)]
pub struct Registers {
    pub pc: *const i64,
    pub m0: *mut i64,
    pub msize: u64,
    pub s0: *mut i64,
    pub ssize: u64,
    pub sp: u64,
    pub d0: *mut i64,
    pub dsize: u64,
    pub dp: u64,
    pub handler_sp: u64,
}
