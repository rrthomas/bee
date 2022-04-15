// glib traps.
//
// (c) Reuben Thomas 2022
//
// The package is distributed under the GNU General Public License version 3,
// or, at your option, any later version.
//
// THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
// RISK.

using GI;

using Bee;

void bind_thing(Bee.State S, BaseInfo info) {
	var infotype = info.get_type();
	print(@"type $(infotype)\n");
	switch (infotype) {
	case INVALID:
	case INVALID_0:
		break;
	case FUNCTION:
	{
		// ABI: on entry, one stack item per in/ref parameter; on exit,
		// one stack item for the return value and one for each ref/out
		// parameter.
		var fn = (FunctionInfo)info;
		print(@"symbol: $(fn.get_symbol())\n");
		print(@"flags: $(fn.get_flags())\n");
		print(@"args:\n");
		var n_args = fn.get_n_args();
		var in_args = new Argument[n_args];
		var out_args = new Argument[n_args];
		for (int i = n_args - 1; i >= 0; i--) {
			var arg = fn.get_arg(i);
			var val = S.pop_data();
			switch (arg.get_type().get_tag()) {
			case VOID:
				warn_if_fail(arg.get_ownership_transfer() == NOTHING);
				in_args[i].v_pointer = (void *)val;
				break;
			case BOOLEAN:
				in_args[i].v_boolean = (bool)val;
				break;
			case INT8:
				in_args[i].v_int8 = (int8)val;
				break;
			case UINT8:
				in_args[i].v_uint8 = (uint8)val;
				break;
			case INT16:
				in_args[i].v_int16 = (int16)val;
				break;
			case UINT16:
				in_args[i].v_uint16 = (uint16)val;
				break;
			case INT32:
				in_args[i].v_int32 = (int32)val;
				break;
			case UINT32:
				in_args[i].v_uint32 = (uint32)val;
				break;
			case INT64:
				in_args[i].v_int64 = (int64)val;
				break;
			case UINT64:
				in_args[i].v_uint64 = (uint64)val;
				break;
			case FLOAT:
				in_args[i].v_float = (float)val;
				break;
			case DOUBLE:
				in_args[i].v_double = (double)val;
				break;
			case UTF8:
			case FILENAME:
				in_args[i].v_string = (string)val;
				break;
			case UNICHAR:
				break;
			case GHASH:
			case GLIST:
			case GSLIST:
				break;
			case GTYPE:
				break;
			case INTERFACE:
				break;
			case ERROR:
				break;
			default:
				assert_not_reached();
			}
		}
		Argument ret = Argument() {};
		try {
			fn.invoke(in_args, out_args, ret);
			// FIXME: push ret
		} catch (GLib.Error e) {
			// FIXME: push error
		}
	}
	break;
	// public GI.PropertyInfo get_property ();
	// public GI.VFuncInfo get_vfunc ();
	// public bool invoke ([CCode (array_length_cname = "n_in_args", array_length_pos = 1.5)] GI.Argument[] in_args, [CCode (array_length_cname = "n_out_args", array_length_pos = 2.5)] GI.Argument[] out_args, GI.Argument return_value) throws GI.InvokeError;
	case CALLBACK:
		break;
	case STRUCT:
		break;
	case BOXED:
		break;
	case ENUM:
		break;
	case FLAGS:
		break;
	case OBJECT:
		break;
	case INTERFACE:
		break;
	case CONSTANT:
		break;
	case UNION:
		break;
	case VALUE:
		break;
	case SIGNAL:
		break;
	case VFUNC:
		break;
	case PROPERTY:
		break;
	case FIELD:
		break;
	case ARG:
	{
		var arg = (ArgInfo)info;
		print(@"name $(arg.get_name())\n");
		if (arg.is_return_value())
			print(@"return\n");
		if (arg.is_optional())
			print(@"optional\n");
		bind_thing(S, arg.get_type());
	}
		break;
	case TYPE:
	{
		var type = (GI.TypeInfo)info;
		if (type.is_pointer())
			print(@"pointer\n");
		// print(@"storage type $(type.get_storage_type())"); FIXME: needs gobject-introspection-1.0 >= 1.64.1
		print(@"tag $(type.get_tag())\n");
	}
		break;
	case UNRESOLVED:
		break;
	}
}

string pop_string(Bee.State S) {
	var len = S.pop_data();
	unowned string strp = (string *)S.pop_data();
	return (string)strp.substring(0, len);
}

static bool initialized = false;
static unowned Typelib typelib;
static unowned string namespace_;

void bind_symbol(Bee.State S, string fn) {
	assert(initialized);
	var r = Repository.get_default();
	var info = r.find_by_name(namespace_, fn);
	if (info == null) {
		print(@"$(fn) does not exist in $(namespace_)\n");
		// FIXME: return an error
	}
	unowned string name, val;
	var iterator = AttributeIter() {};
	while (info.iterate_attributes(ref iterator, out name, out val)) {
		print(@"name $(name) value $(val)\n");
	}
	bind_thing(S, info);
}

bool open_glib() {
	var r = Repository.get_default();
	try {
		typelib = r.require("GLib", null, 0);
		namespace_ = r.load_typelib(typelib, 0);
	} catch (GLib.Error e) {
		warning(e.message);
		return false;
	}
	return true;
}

public word trap_glib(Bee.State S)
{
	int error = Bee.Error.OK;

	if (!initialized) {
		if (!open_glib())
			return Bee.Error.INVALID_LIBRARY; // Need a better error.
		initialized = true;
	}

	// Name of function to call with arguments underneath.
	// Returns return value, if applicable.
	// ( x*i c-addr u -- x | )
	var str = pop_string(S);
	message(@"GLib TRAP $(str)");
	bind_symbol(S, str);

	return error;
}
