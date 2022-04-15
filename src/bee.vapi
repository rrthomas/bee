[CCode (cheader_filename = "bee/bee.h")]
namespace Bee {
	// Basic types
	[SimpleType]
	[CCode (cname = "bee_word_t")]
	[IntegerType (rank = 8)]
	public struct word {
		public const word MIN;
		public const word MAX;

		[CCode (cname = "g_strdup_printf", instance_pos = -1)]
		public string to_string (string format = "%li");

		public const int BYTES;

		[CCode (cname = "BEE_WORD_BIT")]
		public const int BITS;

	}

	[SimpleType]
	[CCode (cname = "bee_uword_t")]
	[IntegerType (rank = 9)]
	public struct uword {
		[CCode (cname = "0UL")]
		public const uword MIN;
		public const uword MAX;

		[CCode (cname = "g_strdup_printf", instance_pos = -1)]
		public string to_string (string format = "%lu");

		[CCode (cname = "BEE_WORD_BYTES")]
		public const int BYTES;

		[CCode (cname = "BEE_WORD_BIT")]
		public const int BITS;
	}

	// Error codes
	public enum Error {
		OK,
		INVALID_OPCODE,
		STACK_UNDERFLOW,
		STACK_OVERFLOW,
		UNALIGNED_ADDRESS,
		INVALID_LIBRARY,
		INVALID_FUNCTION,
		BREAK
	}

	// Stack access
	public int check_stack(uword ssize, uword sp, uword pushes, uword pops);
	public int pop_stack(word *s0, uword ssize, ref uword sp, ref word val_ptr);
	public int push_stack(word *s0, uword ssize, ref uword sp, word val);

	// Default stacks size in words
	public const int DEFAULT_STACK_SIZE;

	// Bee state
	[Compact]
	[CCode (cname = "bee_state", destroy_function = "bee_destroy")]
	public class State {
		word *pc;
		word *s0;
		uword ssize;
		uword sp;
		word *d0;
		uword dsize;
		uword dp;
		uword handler_sp;

		[CCode (cname = "bee_init")]
		State(word *pc, uword stack_size, uword return_stack_size);
		public word run();

		public int check_stack(uword pushes, uword pops) {
			return Bee.check_stack(this.ssize, this.sp, pushes, pops);
		}

		public word pop_stack() {
			word val = 0;
			Bee.pop_stack(this.s0, this.ssize, ref this.sp, ref val);
			return val;
		}

		public void push_stack(word val) {
			Bee.push_stack(this.s0, this.ssize, ref this.sp, val);
		}

		public word pop_data() {
			word val = 0;
			Bee.pop_stack(this.d0, this.dsize, ref this.dp, ref val);
			return val;
		}

		public void push_data(word val) {
			Bee.push_stack(this.d0, this.dsize, ref this.dp, val);
		}
	}

	public void register_args(string[] args);
}
