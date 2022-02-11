#[allow(unused)]
#[derive(Debug, Copy, Clone, Hash, Eq, PartialEq)]
#[repr(u8)]
pub enum Op {
    CallI    = 0x0,
    PushI    = 0x1,
    PushrelI = 0x2,
    JumpI    = 0x3,
    JumpzI   = 0x4,
    Trap     = 0x5,
}

#[allow(non_camel_case_types)]
#[allow(unused)]
pub enum Insn {
    Nop            = 0x00,
    Not            = 0x01,
    And            = 0x02,
    Or             = 0x03,
    Xor            = 0x04,
    LShift         = 0x05,
    RShift         = 0x06,
    ARShift        = 0x07,
    Pop            = 0x08,
    Dup            = 0x09,
    Set            = 0x0a,
    Swap           = 0x0b,
    Jump           = 0x0c,
    JumpZ          = 0x0d,
    Call           = 0x0e,
    Ret            = 0x0f,
    Load           = 0x10,
    Store          = 0x11,
    Load1          = 0x12,
    Store1         = 0x13,
    Load2          = 0x14,
    Store2         = 0x15,
    Load4          = 0x16,
    Store4         = 0x17,
    Neg            = 0x18,
    Add            = 0x19,
    Mul            = 0x1a,
    DivMod         = 0x1b,
    UDivMod        = 0x1c,
    Eq             = 0x1d,
    Lt             = 0x1e,
    ULt            = 0x1f,
    PushR          = 0x20,
    PopR           = 0x21,
    DupR           = 0x22,
    Catch          = 0x23,
    Throw          = 0x24,
    Break          = 0x25,
    Word_Bytes     = 0x26,
    Get_M0         = 0x27,
    Get_MSize      = 0x28,
    Get_SSize      = 0x29,
    Get_SP         = 0x2a,
    Set_SP         = 0x2b,
    Get_DSize      = 0x2c,
    Get_DP         = 0x2d,
    Set_DP         = 0x2e,
    Get_Handler_SP = 0x2f,
}