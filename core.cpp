#include "core.h"


//String representation of heap memory
#define toStrGlobal(TYPE, FUNCTION) \
core->type.stringGlobal[ValueType::TYPE] = FUNCTION;

//String representation of stack memory
#define toStrLocal(TYPE, FUNCTION) \
core->type.stringLocal[ValueType::TYPE] = FUNCTION;

//Destructor
#define destr(TYPE, FUNCTION) \
core->type.destructor[ValueType::TYPE] = FUNCTION;

//Pointer in data
#define data_pointer(TYPE, NAME, POINTER) \
	ptr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*)); \
	if (!ptr) \
	throw std::bad_alloc(); \
	*ptr = ValueType::TYPE; \
	*(void*)(ptr + 1) = POINTER; \
	data[T(NAME)] = ptr;

//Function pointer in data
#define data_1(TYPE, NAME, POINTER) \
	ptr = (ValueType*)malloc(sizeof(ValueType) + 1); \
	if (!ptr) \
	throw std::bad_alloc(); \
	*ptr = ValueType::TYPE; \
	*(Procedure*)(ptr + 1) = POINTER; \
	data[T(NAME)] = ptr;

//Unary function
#define uFun(LEFT_TYPE, FUNCTION, MODIFIER) \
(*table)[ValueType::LEFT_TYPE] = Operation{FUNCTION, (ValueType::all), MODIFIER};

//Unary function with return type
#define uFun(LEFT_TYPE, FUNCTION, MODIFIER, RETURN_TYPE) \
(*table)[ValueType::LEFT_TYPE] = Operation{FUNCTION, ValueType::RETURN_TYPE, MODIFIER};

//Unary function interface
//#define uFun(LEFT_TYPE, FUNCTION) \
(*table)[ValueType::LEFT_TYPE] = FUNCTION;

//Binary function
#define bFun(LEFT_TYPE, RIGHT_TYPE, FUNCTION, MODIFIER, RETURN_TYPE) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = Operation{FUNCTION, ValueType::RETURN_TYPE, MODIFIER};

//Binary function interface
#define bFunInterface(LEFT_TYPE, LEFT_CLASS, RIGHT_TYPE, RIGHT_CLASS, RESULT_TYPE, RESULT_CLASS, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = binaryFunctionInterface<LEFT_CLASS, RIGHT_CLASS, RESULT_CLASS, ValueType::RESULT_TYPE, FUNCTION>;

//Binary function interface with matching ValueType and typename
#define bFunInterfaceMatch(LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNCTION) \
(*table)[ValueTypeBinary(ValueType::LEFT_TYPE, ValueType::RIGHT_TYPE)] = binaryFunctionInterface<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, ValueType::RESULT_TYPE, FUNCTION>;

#define bFunInterfaceNameRefDowncast(TYPE) \
	bFun(name, name, findValueR0R2, bMod_N(), none); \
	bFun(name, TYPE, findValueR2, bMod_N(), none); \
	bFun(TYPE, name, findValueR0, bMod_N(), none); \
	bFun(reference, reference, getValueR0R2, bMod_N(), none); \
	bFun(reference, TYPE, getValueR2, bMod_N(), none); \
	bFun(TYPE, reference, getValueR0, bMod_N(), none); \
	bFun(reference, name, findValueR0, bMod_N(), none); \
	bFun(name, reference, getValueR0, bMod_N(), none);

#define bFunInterfaceNameRefDowncastTricketry \
	bFunInterfaceNameRefDowncast(int8); \
	bFunInterfaceNameRefDowncast(int16); \
	bFunInterfaceNameRefDowncast(int32); \
	bFunInterfaceNameRefDowncast(int64); \
	bFunInterfaceNameRefDowncast(uint8); \
	bFunInterfaceNameRefDowncast(uint16); \
	bFunInterfaceNameRefDowncast(uint32); \
	bFunInterfaceNameRefDowncast(uint64); \
	bFunInterfaceNameRefDowncast(float32); \
	bFunInterfaceNameRefDowncast(float64);

#define bFunMath_ii(FUNCTION) \
	bFun(int64, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int32, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int32, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int32, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int32, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int16, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int16, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int16, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int16, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int8, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int8, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int8, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int8, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int8);

#define bFunMath_uu(FUNCTION) \
	bFun(uint64, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint64, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint64, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint64, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint32, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint32, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), uint32); \
	bFun(uint32, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), uint32); \
	bFun(uint32, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), uint32); \
	bFun(uint16, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint16, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), uint32); \
	bFun(uint16, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), uint16); \
	bFun(uint16, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), uint16); \
	bFun(uint8, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), uint64); \
	bFun(uint8, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), uint32); \
	bFun(uint8, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), uint16); \
	bFun(uint8, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), uint8);

#define bFunMath_ui(FUNCTION) \
	bFun(uint64, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint64, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint64, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint64, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint32, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint32, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(uint32, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(uint32, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(uint16, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint16, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(uint16, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(uint16, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(uint8, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(uint8, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(uint8, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(uint8, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), int8); \

#define bFunMath_iu(FUNCTION) \
	bFun(int64, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int64, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int32, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int32, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int32, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int32, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int16, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int16, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int16, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int16, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int8, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), int64); \
	bFun(int8, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), int32); \
	bFun(int8, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), int16); \
	bFun(int8, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), int8);

#define bFunMath_ff(FUNCTION) \
	bFun(float32, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32);

#define bFunMath_dd(FUNCTION) \
	bFun(float64, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \

#define bFunMath_df(FUNCTION) \
	bFun(float64, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float64);

#define bFunMath_fd(FUNCTION) \
	bFun(float32, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64);

#define bFunMath_if(FUNCTION) \
	bFun(int64, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(int32, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(int16, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(int8, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \

#define bFunMath_fi(FUNCTION) \
	bFun(float32, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \

#define bFunMath_uf(FUNCTION) \
	bFun(uint64, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(uint32, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(uint16, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(uint8, float32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \

#define bFunMath_fu(FUNCTION) \
	bFun(float32, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \
	bFun(float32, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), float32); \

#define bFunMath_id(FUNCTION) \
	bFun(int64, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(int32, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(int16, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(int8, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \

#define bFunMath_di(FUNCTION) \
	bFun(float64, int64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, int32, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, int16, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, int8, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \

#define bFunMath_ud(FUNCTION) \
	bFun(uint64, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(uint32, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(uint16, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(uint8, float64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \

#define bFunMath_du(FUNCTION) \
	bFun(float64, uint64, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, uint32, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, uint16, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \
	bFun(float64, uint8, FUNCTION, bMod_N(Md, Lvd, Rvd), float64); \

#define bFunMath_all(FUNCTION) \
	bFunMath_ii(FUNCTION ## _ii) \
	bFunMath_iu(FUNCTION ## _iu) \
	bFunMath_if(FUNCTION ## _if) \
	bFunMath_id(FUNCTION ## _id) \
	bFunMath_ui(FUNCTION ## _ui) \
	bFunMath_uu(FUNCTION ## _uu) \
	bFunMath_uf(FUNCTION ## _uf) \
	bFunMath_ud(FUNCTION ## _ud) \
	bFunMath_fi(FUNCTION ## _fi) \
	bFunMath_fu(FUNCTION ## _fu) \
	bFunMath_ff(FUNCTION ## _ff) \
	bFunMath_fd(FUNCTION ## _fd) \
	bFunMath_di(FUNCTION ## _di) \
	bFunMath_du(FUNCTION ## _du) \
	bFunMath_df(FUNCTION ## _df) \
	bFunMath_dd(FUNCTION ## _dd) \

namespace Core {
	ValueType none = ValueType::none;

	Module* initCore() {
		//if (Core::core != 0)
		//	return *Core::core;
		Module* core = new Module();
		core->type.count = 45;
		core->type.id = Table<String, ValueType>({	//type.name : typeID
			{T("int64"), ValueType::int64},
			{T("int32"), ValueType::int32},
			{T("int16"), ValueType::int16},
			{T("int8"), ValueType::int8},
			{T("uint64"), ValueType::uint64},
			{T("uint32"), ValueType::uint32},
			{T("uint16"), ValueType::uint16},
			{T("uint8"), ValueType::uint8},
			{T("float128"), ValueType::float128},
			{T("float64"), ValueType::float64},
			{T("float32"), ValueType::float32},
			{T("string"), ValueType::string},
			{T("name"), ValueType::name},
			{T("array"), ValueType::tuple},
			{T("expression"), ValueType::expression},
		});
		core->type.name = Table<ValueType, String>({	//typeID : type.name
			{ValueType::int64, T("int64")},
			{ValueType::int32, T("int32")},
			{ValueType::int16, T("int16")},
			{ValueType::int8, T("int8")},
			{ValueType::uint64, T("uint64")},
			{ValueType::uint32, T("uint32")},
			{ValueType::uint16, T("uint16")},
			{ValueType::uint8, T("uint8")},
			{ValueType::float128, T("float128")},
			{ValueType::float64, T("float64")},
			{ValueType::float32, T("float32")},
			{ValueType::string, T("string")},
			{ValueType::name, T("name")},
			{ValueType::tuple, T("array")},
			{ValueType::expression, T("expression")},
		});
		core->type.size = new uint8[48]{
			0,
			(uint8)-1,
			1,
			1,
			0, 0,
			1, 2, 4, 8,
			1, 2, 4, 8,
			2, 4, 8, 16,
			(uint8)-1, (uint8)-1, (uint8)-1, (uint8)-1,
			(uint8)-1, (uint8)-1, (uint8)-1, (uint8)-1,
			(uint8)-1,
			8, 8, 8,
			8, 8, 8,
			8, 8, 8,
			8, 8, 8,
			(uint8)-1,
			8,
			8,
			8,
			8, 8,
			8,
			8,
			8
		};
		/*core->type.size = Table<ValueType, size_t>({	//typeID : type.size
			{ValueType::int64, 8},
			{ValueType::int32, 4},
			{ValueType::int16, 2},
			{ValueType::int8, 1},
			{ValueType::uint64, 8},
			{ValueType::uint32, 4},
			{ValueType::uint16, 2},
			{ValueType::uint8, 1},
			{ValueType::float128, 16},
			{ValueType::float64, 8},
			{ValueType::float32, 4},
			{ValueType::string, sizeof(void*)},
			{ValueType::name, sizeof(void*)},
			{ValueType::arr, 0},
			{ValueType::expression, sizeof(void*)},
			{ValueType::unprocedure, sizeof(void*)}, //TODO: void* for 'all'
			{ValueType::reference, sizeof(void*)},
			{ValueType::pointer, sizeof(void*)},
		});*/
		core->type.stringGlobal = Table<ValueType, ToStringGlobal>();
		{
			toStrGlobal(all, toStringGlobal);
		}
		core->type.stringLocal = Table<ValueType, ToStringLocal>();
		{
			toStrLocal(all, toStringLocal);
			//toStrLocal(tuple, toStringLocalArray);
		}
		core->type.destructor = Table<ValueType, DestructorProcedure>();
		{
			destr(all, (DestructorProcedure)doNothing); //TODO: Fill with string, table and stuff
			destr(tuple, (DestructorProcedure)destructorArr);
			destr(autoptr, (DestructorProcedure)free);
		}
		core->type.destructorGlobal = Table<ValueType, Destructor>();
		{
			destr(all, (DestructorProcedure)doNothing); //TODO: Fill with string, table and stuff
			destr(tuple, (DestructorProcedure)destructorArr);
			destr(autoptr, (DestructorProcedure)free);
		}
		core->op.prefixForward = Table<String, Table<ValueType, Operation>>();
		{
			Table<ValueType, Operation>* table;

			table = &core->op.prefixForward[T("//")];
			//uFun(all, ignore);
			
			table = &core->op.prefixForward[T("?")];
			uFun(all, conditional, uMod_N(), none);
			uFun(name, findValueR0, uMod_N(), none);
			uFun(string, conditionalTrue, uMod_N(), none);
			uFun(truth, conditionalTrue, uMod_N(), none);
			uFun(lie, conditionalFalse, uMod_N(), none);

			table = &core->op.prefixForward[T("?.")];
			//uFun(all, conditionalShort);
			
			table = &core->op.prefixForward[T("?|")];
			//uFun(all, loopWhile);

			table = &core->op.prefixForward[T("?:")];
			//uFun(all, conditionalSwitch);
			//uFun(tuple, conditionalSwitch);

			table = &core->op.prefixForward[T("?<")];
			//uFun(all, conditionalComparator);

			table = &core->op.prefixForward[T("?:<")];
			//uFun(all, conditionalSwitchPredicate);

			table = &core->op.prefixForward[T("!<<")];
			uFun(name, getReferenceR0, uMod_N(), none);
			uFun(all, print, uMod_N(Md, Ai), none);

			table = &core->op.prefixForward[T("!>>")];
			uFun(name, getReferenceR0, uMod_N(), none);
			uFun(reference, scan, uMod_N(), none);

			table = &core->op.prefixForward[T(",")];
			uFun(all, commaPrefix, uMod_N(), none);

			table = &core->op.prefixForward[T("+")];
			uFun(name, findValueR0, uMod_N(), none);
			uFun(type, instantiate, uMod_N(), none);
			uFun(table, instantiateObject, uMod_N(), none);

			//table = &core->op.prefix[T("-")];
			//u uFun(int64, (negate<int64>));
			//uFun(float64, (negate<float64>));

			table = &core->op.prefixForward[T("&")];
			uFun(name, (getReferenceR1), uMod_N(), none);

			table = &core->op.prefixForward[T("*")];
			uFun(name, findValueR0, uMod_N(), none);

			table = &core->op.prefixForward[T("*&")];
			uFun(name, (findValueR0), uMod_N(), none);
			uFun(reference, (getValueR0), uMod_N(), none);

			table = &core->op.prefixForward[T(">")];
			uFun(all, allArrayInclusive, uMod_N(), none);

			table = &core->op.prefixForward[T("^")];
			uFun(int64, getNamespace, uMod_N(), none);

			table = &core->op.prefixForward[T("@")];
			uFun(int64, atContextByIndex, uMod_N(), none);
			uFun(name, atContextByName, uMod_N(), none);
			uFun(string, atContextByName, uMod_N(), none);
			uFun(tuple, renameArrayContext, uMod_N(), none);

			table = &core->op.prefixForward[T(".")];
			uFun(int64, contextMethod, uMod_N(), none);

			table = &core->op.prefixForward[T("::")];
			uFun(string, getNamespaceEntry, uMod_N(), none);
			//uFun(tuple, getApplication);

			table = &core->op.prefixForward[T("::<")];
			uFun(string, getNamespacePrefix, uMod_N(), none);

			table = &core->op.prefixForward[T("::>")];
			uFun(string, getNamespacePostfix, uMod_N(), none);

			table = &core->op.prefixForward[T("::.^")];
			uFun(string, getNamespaceBinaryRight, uMod_N(), none);

			table = &core->op.prefixForward[T("::^.")];
			uFun(string, getNamespaceBinaryLeft, uMod_N(), none);
		}
		core->op.postfixForward = Table<String, Table<ValueType, Operation>>();
		{
			Table<ValueType, Operation>* table;


			table = &core->op.postfixForward[T("")];
			uFun(name, findValueR2, uMod_N(), none);
			uFun(reference, getValueR2, uMod_N(), none);
			uFun(expression, callFunction, uMod_N(), none);


			table = &core->op.postfixForward[T(",")];
			uFun(all, commaPostfix, uMod_N(), none);
			
			table = &core->op.postfixForward[T("*")];
			//uFun(all, getValueProcedure);

			table = &core->op.postfixForward[T(">")];
			uFun(all, allArrayExclusive, uMod_N(), none);

			table = &core->op.postfixForward[T("^")];
			uFun(all, allGroupExclusive, uMod_N(), none);

			table = &core->op.postfixForward[T(":")];
			uFun(table, allContext, uMod_N(), none);
			uFun(name, allContext, uMod_N(), none);
			//uFun(type, allocDynamicArrayStack);

			table = &core->op.postfixForward[T("*:")];
			//uFun(dict, allocDynamicArrayHeap);

			table = &core->op.postfixForward[T(".")];
			uFun(name, findValueR2, uMod_N(), none);
			uFun(reference, getValueR2, uMod_N(), none);
			uFun(expression, callFunction, uMod_N(), none);


			table = &core->op.postfixForward[T(">>!")];
			uFun(name, getReferenceR0, uMod_N(), none);
			uFun(all, print, uMod_N(Md, Ai), none);

			table = &core->op.postfixForward[T("<<!")];
			uFun(name, getReferenceR0, uMod_N(), none);
			uFun(reference, scan, uMod_N(), none);
		}
		core->op.binaryForward = Table<String, Table<ValueTypeBinary, Operation>>();
		{
			Table<ValueTypeBinary, Operation>* table;

			table = &core->op.binaryForward[T("")]; //Null coalesceing
			bFun(name, all, (findValueR2), bMod_N(), none); //tuple
			bFun(reference, tuple, (findValueR2), bMod_N(), none);

			//bFun(unprocedure, all, invokeProcedure, bMod_N(), none); //tuple
			//bFun(parameter_pattern, tuple, invokeFunction, bMod_N(), none);
			//bFun(unfunction, tuple, invokeNativeFunction, bMod_N(), none);

			bFun(native_function, tuple, invokeNativeFunction, bMod_N(), none);
			//bFun(type, all, cast, bMod_N(Md, Lrd, Rrd), none);

			bFun(type, name, declareVariable, bMod_N(), none);

			bFun(string, string, concatenate, bMod_N(Mi, Lii, Ri), none);

			//bFun(type, none, allocStack);


			table = &core->op.binaryForward[T("?,")]; //Null coalesceing

			table = &core->op.binaryForward[T("?=")]; //Null conditional assignment

			table = &core->op.binaryForward[T("?.")]; //Null conditional method call

			//table = &core->op.binaryForward[T("@")];
			//bFun(unfunction, tuple, callWithContext, bMod_N(), none);

			table = &core->op.binaryForward[T(",")];
			bFun(all, all, commaBinary, bMod_N(), none);

			table = &core->op.binaryForward[T("=")];
			bFun(name, all, assignToName, bMod_N(), none);
			bFun(reference, all, assignToReference, bMod_N(), none);
			//bFun(pointer, all, assignToName);

			table = &core->op.binaryForward[T("<=")];
			bFun(name, all, assignToName, bMod_N(), none);
			bFun(reference, all, assignToReference, bMod_N(), none);

			table = &core->op.binaryForward[T(":<=")];
			//bFun(name, name, findValueR0);
			bFun(name, all, declareAssign, bMod_N(), none);

			table = &core->op.binaryForward[T(":=")];
			bFun(name, all, declareAssign, bMod_N(), none);

			table = &core->op.binaryForward[T(".")];
			bFun(int64, int64, (createFloat<int64, int64, float64, ValueType::float64>), bMod_N(), none);
			bFun(all, name, findValueR0, bMod_N(), none);
			bFun(all, name, getTableEntry, bMod_N(), none);
			//bFun(all, unprocedure, callThis, bMod_N(), none);

			table = &core->op.binaryForward[T("::")];
			bFun(name, name, findValueR2, bMod_N(), none);
			bFun(name, string, findValueR2, bMod_N(), none);
			bFun(table, name, getTableEntry, bMod_N(), none);
			//bFun(table, tuple, getTable);
			bFun(table, string, getTableEntry, bMod_N(), none);
			//bFun(type, int64, allocArrayStack);

			table = &core->op.binaryForward[T("::<")];
			bFun(name, name, findValueR0R2, bMod_N(), none);
			bFun(table, string, getTablePrefix, bMod_N(), none);

			table = &core->op.binaryForward[T("::>")];
			bFun(name, name, findValueR0R2, bMod_N(), none);
			bFun(table, string, getTablePostfix, bMod_N(), none);

			table = &core->op.binaryForward[T("::.^")];
			bFun(name, name, findValueR0R2, bMod_N(), none);
			bFun(table, string, getTableBinaryRight, bMod_N(), none);

			table = &core->op.binaryForward[T("::^.")];
			bFun(name, name, findValueR0R2, bMod_N(), none);
			bFun(table, string, getTableBinaryLeft, bMod_N(), none);

			table = &core->op.binaryForward[T("*:")];
			//bFun(type, int64, allocArrayHeap);

			table = &core->op.binaryForward[T("||")];
			bFun(string, string, concatenate, bMod_N(Mi, Lii, Ri), none);


			table = &core->op.binaryForward[T("+")];
			bFunInterfaceNameRefDowncastTricketry;
			bFunMath_all(add);

			table = &core->op.binaryForward[T("-")];
			bFunInterfaceNameRefDowncastTricketry;
			bFunMath_all(sub);


			table = &core->op.binaryForward[T("*")];
			bFunInterfaceNameRefDowncastTricketry;
			bFunMath_all(mul);

			table = &core->op.binaryForward[T("/")];
			bFunInterfaceNameRefDowncastTricketry;
			bFunMath_ii(div_ii);
			bFunMath_iu(div_iu);
			bFunMath_if(div_if);
			bFunMath_id(div_id);
			bFunMath_ui(div_ui);
			bFunMath_uu(div_uu);
			bFunMath_uf(div_uf);
			bFunMath_ud(div_ud);
			bFunMath_fi(div_fi);
			bFunMath_fu(div_fu);
			bFunMath_ff(div_ff);
			bFunMath_fd(div_fd);
			bFunMath_di(div_di);
			bFunMath_du(div_du);
			bFunMath_df(div_df);
			bFunMath_dd(div_dd);

			table = &core->op.binaryForward[T("/.")];
			bFunInterfaceNameRefDowncastTricketry;
			bFunMath_ii(divi_ii);
			bFunMath_iu(divi_iu);
			bFunMath_ui(divi_ui);
			bFunMath_uu(divi_uu);

			table = &core->op.binaryForward[T("%")];
			bFunMath_ii(mod_ii);
			bFunMath_iu(mod_iu);
			bFunMath_ui(mod_ui);
			bFunMath_uu(mod_uu);

			table = &core->op.binaryForward[T("^")];
			bFunMath_ii(bitOR);
			bFunMath_iu(bitOR);
			bFunMath_if(bitOR);
			bFunMath_id(bitOR);
			bFunMath_ui(bitOR);
			bFunMath_uu(bitOR);
			bFunMath_uf(bitOR);
			bFunMath_ud(bitOR);
			bFunMath_fi(bitOR);
			bFunMath_fu(bitOR);
			bFunMath_ff(bitOR);
			bFunMath_fd(bitOR);
			bFunMath_di(bitOR);
			bFunMath_du(bitOR);
			bFunMath_df(bitOR);
			bFunMath_dd(bitOR);

			table = &core->op.binaryForward[T("\\/")]; //bitwise 
			bFunMath_ii(bitOR);
			bFunMath_iu(bitOR);
			bFunMath_if(bitOR);
			bFunMath_id(bitOR);
			bFunMath_ui(bitOR);
			bFunMath_uu(bitOR);
			bFunMath_uf(bitOR);
			bFunMath_ud(bitOR);
			bFunMath_fi(bitOR);
			bFunMath_fu(bitOR);
			bFunMath_ff(bitOR);
			bFunMath_fd(bitOR);
			bFunMath_di(bitOR);
			bFunMath_du(bitOR);
			bFunMath_df(bitOR);
			bFunMath_dd(bitOR);

			table = &core->op.binaryForward[T("/\\")]; //bitwise
			bFunMath_ii(bitAND);
			bFunMath_iu(bitAND);
			bFunMath_if(bitAND);
			bFunMath_id(bitAND);
			bFunMath_ui(bitAND);
			bFunMath_uu(bitAND);
			bFunMath_uf(bitAND);
			bFunMath_ud(bitAND);
			bFunMath_fi(bitAND);
			bFunMath_fu(bitAND);
			bFunMath_ff(bitAND);
			bFunMath_fd(bitAND);
			bFunMath_di(bitAND);
			bFunMath_du(bitAND);
			bFunMath_df(bitAND);
			bFunMath_dd(bitAND);

			table = &core->op.binaryForward[T("\\\\/")]; //logical
		}
		core->op.prefixGeneral = Table<ValueType, Operation>();
		{
			Table<ValueType, Operation>* table = &core->op.prefixGeneral;

			uFun(object, callObjectPrefix, uMod_N(Mi, Ai), none);
		}
		core->op.postfixGeneral = Table<ValueType, Operation>();
		{
			Table<ValueType, Operation>* table = &core->op.postfixGeneral;

			uFun(object, callObjectPostfix, uMod_N(), none);
		}
		core->op.binaryGeneral = Table<ValueTypeBinary, Operation>();
		{
			Table<ValueTypeBinary, Operation>* table = &core->op.binaryGeneral;

			bFun(object, all, callObjectBinaryLeft, bMod_N(), none);
			bFun(all, object, callObjectBinaryRight, bMod_N(), none);
		}
		core->op.coalescing = Table<ValueTypeBinary, Operation>();
		{
			Table<ValueTypeBinary, Operation>* table = &core->op.coalescing;

			bFun(name, all, (findValueR1), bMod_N(), none); //tuple
			bFun(reference, tuple, (findValueR1), bMod_N(), none);
			//bFun(unprocedure, all, invokeProcedure, bMod_N(), none); //tuple
			//bFun(parameter_pattern, tuple, invokeFunction, bMod_N(), none);
			bFun(native_function, all, invokeNativeFunctionUnary, bMod_N(), none);
			bFun(native_function, tuple, invokeNativeFunction, bMod_N(), none);
			
			//bFun(type, all, cast, bMod_N(Md, Lrd, Rrd), none);

			bFun(type, name, declareVariable, bMod_N(), none);

			bFun(string, string, concatenate, bMod_N(Mi, Lii, Ri), none);

			//bFun(type, none, allocStack);
		}

		core->context.onEnter = Table<ValueType, Operation>();
		{
			Table<ValueType, Operation>* table = &core->context.onEnter;

			uFun(all, doNothing, uMod_N(), none);
			uFun(reference, onEnterContextReference, uMod_N(), none);
			uFun(table, onEnterContextNamespace, uMod_N(), none);
		}

		core->context.onExit = Table<ValueType, Operation>();
		{
			Table<ValueType, Operation>* table = &core->context.onExit;

			uFun(all, doNothing, uMod_N(), none);
			uFun(table, onExitContextNamespace, uMod_N(), none);
		}

		return core;
	}

	Table<String, ValueType*> initCoreData() {
		Table<String, ValueType*> data = Table<String, ValueType*>();
		ValueType* ptr;

		auto allocOperator = [](ValueType valueType, void* ptr, OperationModifier mod, ValueType ret) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + sizeof(Operation));
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(Operation*)(valuePtr + 1) = Operation{ ptr, ret, mod };
			return valuePtr;
		};

		auto allocPtr = [](ValueType valueType, void* ptr) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + sizeof(void*));
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(void**)(valuePtr + 1) = ptr;
			return valuePtr;
		};
		auto alloc8 = [](ValueType valueType, uint64 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 8);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint64*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc4 = [](ValueType valueType, uint32 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 4);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint32*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc2 = [](ValueType valueType, uint16 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 2);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint16*)(valuePtr + 1) = value;
			return valuePtr;
		};
		auto alloc1 = [](ValueType valueType, uint8 value) {
			auto valuePtr = (ValueType*)malloc(sizeof(ValueType) + 1);
			if (!valuePtr)
				throw std::bad_alloc();
			*valuePtr = valueType;
			*(uint8*)(valuePtr + 1) = value;
			return valuePtr;
		};

		data["int64"] = alloc1(ValueType::type, (uint8)ValueType::int64);
		data["int32"] = alloc1(ValueType::type, (uint8)ValueType::int32);
		data["int16"] = alloc1(ValueType::type, (uint8)ValueType::int16);
		data["int8"] = alloc1(ValueType::type, (uint8)ValueType::int8);
		data["uint64"] = alloc1(ValueType::type, (uint8)ValueType::uint64);
		data["uint32"] = alloc1(ValueType::type, (uint8)ValueType::uint32);
		data["uint16"] = alloc1(ValueType::type, (uint8)ValueType::uint16);
		data["uint8"] = alloc1(ValueType::type, (uint8)ValueType::uint8);
		data["float32"] = alloc1(ValueType::type, (uint8)ValueType::float32);
		data["float64"] = alloc1(ValueType::type, (uint8)ValueType::float64);
		
		data["string"] = alloc1(ValueType::type, (uint8)ValueType::string);
		data["group"] = alloc1(ValueType::type, (uint8)ValueType::tuple);
		data["table"] = alloc1(ValueType::type, (uint8)ValueType::table);
		data["auto"] = alloc1(ValueType::type, (uint8)ValueType::table);
		

		data["print"] = allocOperator(ValueType::native_operator, print, uMod_N(Md, Ai), ValueType::none);
		//data["scan"] = allocPtr(ValueType::unprocedure, scan);
		//data["if"] = allocPtr(ValueType::unprocedure, conditional);
		//data["dll_load"] = allocPtr(ValueType::unprocedure, loadLibrary);
		//data["dll_free"] = allocPtr(ValueType::unprocedure, freeLibrary);

		//data_pointer(unprocedure, "print", print);
		//data_pointer(unprocedure, "scan", scan);
		//data_pointer(unprocedure, "if", conditional); //Should be pattern matching for truth and lie types.
		//data_pointer(unprocedure, "dll_load", loadLibrary);
		//data_pointer(unprocedure, "dll_free", freeLibrary);

		return data;
	}

	void ignore() {
		//TODO: add script to 
	}


	void callThis() {
		g_val_mem.max_index = g_stack_instruction.get_r(1).shift; //We know that the right value is procedure pointer, so no special destructor should be.
		//g_memory.add<void*>(*g_memory.get<void**>(g_stack_instruction.get_r(0).shift));
		g_val_mem.add<void*>(g_val_mem.get<void*>(g_stack_instruction.get_r(0).shift));

		Array<Instruction>* tuple = new Array<Instruction>();
		Instruction instruction = g_stack_instruction.get_r(2);
		instruction.shift -= g_val_mem.max_index + sizeof(void*);
		tuple->add(instruction);

		instruction = g_stack_instruction.get_r(0);
		//instruction.shift = g_memory.max_index - (sizeof(void*) + 1);	//Fixes shift of procedure after it's moving
		instruction.shift -= sizeof(charT);
		g_stack_instruction.at_r(2) = instruction;
		g_stack_instruction.at_r(1) = Instruction::pos(InstructionType::start_array, g_val_mem.max_index);
		g_val_mem.add<Array<Instruction>*>(tuple);
		g_stack_instruction.at_r(0) = Instruction::atom(InstructionType::ignore_array_start);
	}

	void contextMethod() {

	}


	void contextAtIndex() {
		Instruction top = g_stack_context.get_r(0);
		if (top.value == ValueType::tuple) {
			Array<Instruction> arr = g_val_mem.get<Array<Instruction>>(top.shift);
			//arr.max_index;

			//Instruction instr = g_stack_array.at(g_context.shift).at(g_memory.at<int64>(g_stack_instruction.at_r(1).shift));
			//if(instr.value == ValueType::
		}
	}

	void contextAtName() {	//TODO: delete or think whether it's still needed.
		if (g_stack_context.get(0).value == ValueType::table) {
			auto var = &(*g_val_mem.at<Table<String, ValueType*>*>(g_stack_context.get(0).shift))[String(g_val_mem.at<charT*>(g_stack_instruction.at_r(0).shift))];
			g_val_mem.max_index = g_stack_instruction.at_r(1).shift;
			g_val_mem.add<ValueType**>(var);
		}
	}

	void concatenate(Instruction& instruction_r2, Instruction instruction_r0, Instruction instruction_r1) {
		instruction_r2.modifier += instruction_r0.modifier;
		g_op_mem.max_index = instruction_r1.shift;
		g_stack_instruction.max_index -= 2;
	}


	void getNamespace() {}
	void atContextByIndex() {}
	void atContextByName() {}

	void callWithContext() {}
	void renameArrayContext() {}


	template<typename _TypeLeft, typename _TypeRight, typename _TypeResult, ValueType type, _TypeResult (*function) (_TypeLeft, _TypeRight)>
	void binaryFunctionInterface() {
		_TypeLeft left = g_val_mem.get<_TypeLeft>(g_stack_instruction.get_r(2).shift);
		_TypeRight right = g_val_mem.get<_TypeRight>(g_stack_instruction.get_r(0).shift);

		_TypeResult result = ((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right);

		Instruction resultInstruction = g_stack_instruction.get_r(2);
		resultInstruction.value = type;

		g_memory_delete_span_r(3);
		g_val_mem.add<_TypeResult>(result);

		g_stack_instruction.add(resultInstruction);

		/*g_memory.max_index -= sizeof(_TypeLeft);
		g_memory.max_index -= sizeof(void*);
		g_memory.max_index -= sizeof(_TypeRight);

		g_memory.add<_TypeResult>(((_TypeResult(*) (_TypeLeft, _TypeRight))function)(left, right));

		g_stack_instruction.max_index -= 2;
		g_stack_instruction.at_r(0).value = type;*/
	}


	void getChild() {
		/*String* right = *(String**)(memory.data + g_stack_instruction.at_r(0).shift);
		g_stack_instruction.max_index -= 2;
		String* left = *(String**)(memory.data + g_stack_instruction.at_r(0).shift);

		auto ptr = data.at_r(0)[*left];

		if (*ptr == ValueType::dict)
		{
			ptr = (**(std::unordered_map<String, ValueType*>**)(ptr + 1))[*right];
			//++ptr;
			//std::unordered_map<String, ValueType*>* map = *(std::unordered_map<String, ValueType*>**)ptr;
			//ptr = (*map)[*right];

			g_stack_instruction.at_r(0).value = *ptr;

			memory.currentSize -= sizeof(void*) + sizeof(void*) + sizeof(void*);
			for (int i = 0; i < specification.type.size[*ptr]; ++i)
				*(memory.data + memory.currentSize + i) = *((uint8*)ptr + sizeof(ValueType) + i);
			memory.currentSize += specification.type.size[*ptr];

			delete left,
			delete right;
		}*/
	}
}