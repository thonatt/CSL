#pragma once

#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <sstream> 

#include <algorithm> //for std::replace
#include <iomanip> // for std::setprecision

#include "StringHelpers.h"
#include "FunctionHelpers.h"

struct MainListener;
MainListener & listen();

using stringPtr = std::shared_ptr<std::string>;

//#define EX(type,var) getE(std::forward<type>(var))

#define EX(type, var) getExp(std::forward<type>(var))

stringPtr makeStringPtr(const std::string & s) {
	auto str_ptr = std::make_shared<std::string>(s);
	//std::cout << " str_cv : " <<  (int)(bool)str_ptr << " " << s << std::endl;
	return str_ptr;
}


struct OperatorBase;
using Ex = std::shared_ptr<OperatorBase>;

//enum NamedObjectTracking { NOT_TRACKED, TRACKED };
//enum NamedObjectInit { NO_INIT, INIT };

enum NamedObjectFlags : uint {
	IS_USED = 1 << 1,
	IS_INIT = 1 << 2,
	IS_TRACKED = 1 << 3,
	ALWAYS_EXP = 1 << 4
};

enum CtorStatus { DECLARATION, INITIALISATION, TEMP, FORWARD };
enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN, IN_BETWEEN_NOSPACE, BEHIND };
//enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };
//enum CtorTypeDisplay { DISPLAY, HIDE};
enum class CtorPosition { MAIN_BLOCK, INSIDE_BLOCK };

enum OpFlags : uint {
	DISABLED = 1 << 1,
	DISPLAY_TYPE = 1 << 2,
	MULTIPLE_INITS = 1 <<3,
	PARENTHESIS = 1 << 4,
	MAIN_BLOCK = 1 << 5
};

class NamedObjectBase;

template<typename T>
struct NamedObjectInit {
	NamedObjectInit(const Ex & _exp, const std::string & s) : exp(_exp), name(s) {}
	Ex exp;
	std::string name;
};

template<typename T, typename ... Args>
Ex createInit(const stringPtr & name, CtorStatus status, uint ctor_flags, const Args &... args);

template<typename T, typename ... Args>
Ex createDeclaration(const stringPtr & name, uint ctor_flags, const Args &... args);

template<typename T>
class NamedObject;

enum OperatorPrecedence {

	ALIAS = 1,
	INITIALIZER_LIST = 1,

	ARRAY_SUBSCRIPT = 2,
	FUNCTION_CALL = 2,
	FIELD_SELECTOR = 2,
	MEMBER_SELECTOR = 2,
	SWIZZLE = 2,
	POSTFIX = 2,
	
	PREFIX = 3,
	UNARY = 3,
	NEGATION = 3,
	ONES_COMPLEMENT = 3,

	MULTIPLY = 4,
	DIVISION = 4,
	MODULO = 4,

	ADDITION = 5,
	SUBSTRACTION = 5,

	BITWISE_SHIFT = 6,

	RELATIONAL = 7,

	EQUALITY = 8,

	BITWISE_AND = 9,

	BITWISE_XOR = 10,

	BITWISE_OR = 11,

	LOGICAL_AND = 12,

	LOGICAL_XOR = 13,

	LOGICAL_OR = 14,

	TERNARY = 15,

	ASSIGNMENT = 16,

	SEQUENCE = 17
};

struct OperatorBase;
using Ex = std::shared_ptr<OperatorBase>;

struct OperatorBase {
	//OperatorBase(stringPtr _op_str_ptr = {}) : op_str_ptr(_op_str_ptr) {
	//	if (!op_str_ptr) {
	//		op_str_ptr = std::make_shared<std::string>(" >no_str< ");
	//	}
	//}

	OperatorBase(uint _flags = 0) : flags(_flags)
	{

	}

	virtual std::string str(int trailing) const {
		return "no_str";
	}

	//std::string op_str() const {
	//	return *op_str_ptr;
	//}

	std::string explore() const {
		return "no_exploration";
	}

	virtual uint rank() const {
		return 0;
	}

	const bool inversion(Ex other) const {
		return rank() < other->rank();
	}

	std::string checkForParenthesis(Ex exp) const {
		if (inversion(exp)) {
			//return exp->str();
			return "(" + exp->str(0) + ")";
		} 
		return exp->str(0);
	}

	void disable() {
		flags = flags | DISABLED;
	}

	bool disabled() const {
		return flags & DISABLED;
	}

	//stringPtr op_str_ptr;
	uint flags = 0;
};

template<typename Operator, typename ... Args>
Ex createExp(const Args &... args) {
	return std::static_pointer_cast<OperatorBase>(std::make_shared<Operator>(args...));
}

struct NamedOperator {

	NamedOperator(const std::string & str) : operator_str(str) {}

	std::string op_str() const { return operator_str; }

	std::string operator_str;
};

template<OperatorPrecedence precedence>
struct Precedence : OperatorBase {
	//Precedence(stringPtr _op_str_ptr = {}) : OperatorBase(_op_str_ptr) {}

	virtual uint rank() const { return (uint)precedence; }
};

struct Alias : Precedence<ALIAS> {
	Alias(stringPtr _obj_str_ptr) : obj_str_ptr(_obj_str_ptr) {}
	//Alias( const std::string & s) : obj_str_ptr(makeStringPtr(s)) {}
	std::string str(int trailing) const { return *obj_str_ptr; }
	//virtual std::string str() const { return "Alias[" + *obj_str_ptr + "]"; }
	
	stringPtr obj_str_ptr;
}; 

template<OperatorPrecedence precedence>
struct MiddleOperator : Precedence<precedence>, NamedOperator {
	MiddleOperator(const std::string & op_str, Ex _lhs, Ex _rhs)
		: NamedOperator(op_str), lhs(_lhs), rhs(_rhs) {
		//std::cout << "middle op : " << op_str << " " << *OperatorBase::op_str_ptr << std::endl;
 	}

	std::string str(int trailing) const {
		return OperatorBase::checkForParenthesis(lhs) + NamedOperator::op_str() + OperatorBase::checkForParenthesis(rhs);
	}

	Ex lhs, rhs;
};

template<uint N>
struct ArgsCall {

	template<typename ... Args>
	ArgsCall(const Args & ... _args) : args{ _args... } {}

	std::string args_str_body() const {
		std::string out = "";
		for (uint i = 0; i < N; ++i) {
			out += args[i]->str(0) + ((i == N - 1) ? "" : ", ");
		}
		return out;
	}

	std::string args_str() const {
		return "(" + args_str_body() + ")";
	}

	std::array<Ex, N> args;
	//std::vector<Ex> args;
};

template<uint N>
struct FunctionCall : Precedence<FUNCTION_CALL>, NamedOperator, ArgsCall<N> {

	template<typename ... Args>
	FunctionCall(const std::string & s, const Args & ... _args)
		: NamedOperator(s), ArgsCall<N>(_args...) {
	}

	std::string str(int trailing) const {
		return op_str() + ArgsCall<N>::args_str();
	}
};

template<typename ... Args>
Ex createFCallExp(const std::string & f_name, const Args & ... args) {
	return createExp<FunctionCall<sizeof...(Args)>>(f_name, args...);
}

struct ConstructorBase : OperatorBase {
	ConstructorBase(CtorStatus _status = INITIALISATION, uint _flags = 0) 
		: ctor_status(_status), OperatorBase(_flags) { }

	CtorPosition position() const {
		if (flags & MAIN_BLOCK) {
			return CtorPosition::MAIN_BLOCK;
		} else {
			return CtorPosition::INSIDE_BLOCK;
		}
	}

	void setTemp() {
		if (status() == INITIALISATION) {
			ctor_status = TEMP;
		}
	}
	void setInit() {
		if (status() == FORWARD) {
			ctor_status = INITIALISATION;
		}
	}

	CtorStatus status() const {
		return ctor_status;
	}

	virtual bool is_bool_ctor() const {
		return false;
	}

	CtorStatus ctor_status;
};

template<typename T, uint N = 0>
struct Constructor : ArgsCall<N>, ConstructorBase {

	virtual uint rank() const {
		if (N == 0) {
			return (uint)FUNCTION_CALL;
		} 
		return ArgsCall<N>::args[0]->rank();
	}

	template<typename ... Args>
	Constructor(stringPtr _obj_name_ptr, CtorStatus _status, uint _flags, const Args & ...  _args)
		: ArgsCall<N>(_args...), ConstructorBase(_status, _flags), obj_name_ptr(_obj_name_ptr)
	{
	}

	std::string obj_name() const {
		return *obj_name_ptr;
	}

	std::string lhs_type_str(int trailing) const {
		if (flags & MULTIPLE_INITS) {
			return "";
		}
		return getTypeStr<T>(trailing);
	}

	virtual std::string rhs_type_str() const {
		return getTypeStr<T>();
	}

	virtual std::string lhs_str(int trailing) const {
		return lhs_type_str(trailing) + " " + obj_name();
	}

	std::string rhs_str() const {
		std::string str;
		if (flags & DISPLAY_TYPE) {
			str += rhs_type_str();
		}
		if (flags & PARENTHESIS) {
			str += ArgsCall<N>::args_str();
		} else {
			str += ArgsCall<N>::args_str_body();
		}
		return str;
	}

	virtual std::string str(int trailing) const {
		if (status() == INITIALISATION) {
			return lhs_str(trailing) + " = " + rhs_str();
		} else if (status() == DECLARATION) {
			return lhs_str(trailing);
		} else if (status() == FORWARD) {
			return ArgsCall<N>::args[0]->str(0);
		} else {
			return rhs_str();
		}
	}

	virtual bool is_bool_ctor() const {
		return IsSameType<T, Bool>;
	}

	stringPtr obj_name_ptr;
};

template<typename T, uint N, uint M>
struct Constructor<Array<T, N>, M> : Constructor<T,M> {
	
	template<typename ... Args>
	Constructor(stringPtr _obj_name_ptr, CtorStatus _status, uint flags, const Args & ...  _args)
		: Constructor<T,M>(_obj_name_ptr, _status, flags, _args...) {
	}

	virtual std::string lhs_str(int trailing) const {
		return Constructor<T,M>::lhs_str(trailing) + TypeStr< Array<T, N> >::array_str();
	}

	virtual std::string rhs_type_str(int trailing) const {
		return getTypeStr<T>(trailing) + TypeStr< Array<T, N> >::array_str();
	}
};

struct MemberAccessor : Precedence<FIELD_SELECTOR> {
	MemberAccessor(const Ex & _obj, stringPtr member_str)
		: member_str_ptr(member_str), obj(_obj) {
	}
	std::string str(int trailing) const {
		return OperatorBase::checkForParenthesis(obj) + "." + *member_str_ptr;
	}
	Ex obj;
	stringPtr member_str_ptr;
};

template<uint N>
struct MemberFunctionAccessor : FunctionCall<N> {
	template<typename ...Args>
	MemberFunctionAccessor(const Ex & _obj, const std::string & fun_name, const Args & ... _args)
		: FunctionCall<N>(fun_name, _args...), obj(_obj) {
	}

	std::string str(int trailing) const {
		return OperatorBase::checkForParenthesis(obj) + "." + FunctionCall<N>::str(0);
	}

	Ex obj;
};

struct ArraySubscript : Precedence<ARRAY_SUBSCRIPT> {
	ArraySubscript(Ex _obj, Ex _arg) : obj(_obj), arg(_arg) {
	}

	std::string str(int trailing) const {
		//std::cout << " array_sub" << std::endl;
		return OperatorBase::checkForParenthesis(obj) + "[" + arg->str(0) + "]";
	}

	Ex obj, arg;
};

struct PrefixUnary : Precedence<PREFIX>, NamedOperator {

	PrefixUnary(const std::string & op_str, Ex _obj)
		: NamedOperator(op_str), obj(_obj) {
	}

	std::string str(int trailing) const {
		return op_str() + OperatorBase::checkForParenthesis(obj);
	}

	Ex obj;
};

struct PostfixUnary : Precedence<POSTFIX>, NamedOperator {
	
	PostfixUnary(const std::string & op_str, Ex _obj)
		: NamedOperator(op_str), obj(_obj) {
	}

	std::string str(int trailing) const {
		return OperatorBase::checkForParenthesis(obj) + op_str();
	}

	Ex obj;
};

struct Ternary : Precedence<TERNARY> {
	Ternary(Ex _condition, Ex _first, Ex _second)  
		: condition(_condition), first(_first), second(_second) {
	}

	std::string str(int trailing) const {
		return OperatorBase::checkForParenthesis(condition) + " ? " + OperatorBase::checkForParenthesis(first) + " : " + OperatorBase::checkForParenthesis(second);
	}

	Ex condition, first, second;
};

template<uint N>
struct InitializerList : Precedence<INITIALIZER_LIST>, ArgsCall<N>
{
	template<typename ... Args>
	InitializerList(const Args & ... args) : ArgsCall<N>(args...) 
	{ 
	}

	virtual std::string str(int trailing) const {
		return "{ " + ArgsCall<N>::args_str_body() + " }";
	}
};

struct EmptyExp : OperatorBase {
	static Ex get() { return std::static_pointer_cast<OperatorBase>(std::make_shared<EmptyExp>()); }

	std::string str(int trailing) const {
		return "";
	}
};

template<typename T>
struct Litteral : OperatorBase {
	Litteral(const T & _i) : i(_i) {}

	virtual std::string str(int trailing) const {
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << i;
		return ss.str();
		//return std::to_string(i); 
	}
	T i;
};

template<> struct Litteral<bool> : OperatorBase {
	Litteral(const bool & _b) : b(_b) {}
	virtual std::string str(int trailing) const { return b ? "true" : "false"; }
	bool b;
};

template<typename T> Ex getE(T && t) {
	return std::forward<T>(t).getEx();
}

class NamedObjectBase {
public:
	NamedObjectBase(const std::string & _name = "", uint _flags = IS_USED | IS_TRACKED)
		: flags(_flags)
	{
		namePtr = std::make_shared<std::string>(_name);
		//std::cout << " end check" << std::endl;
	}

	~NamedObjectBase();

	bool isUsed() const {
		return flags & IS_USED;
	}

	bool isTracked() const {
		return flags & IS_TRACKED;
	}

	void setNotUsed() {
		flags = flags & ~(IS_USED);
	}

	Ex alias() const;

	Ex getEx() &;
	Ex getEx() const &;
	Ex getEx() && ;
	Ex getEx() const &&;

	Ex getExRef();
	Ex getExRef() const;
	Ex getExTmp();
	Ex getExTmp() const;

	void checkDisabling();


	static std::string typeStr() { return "dummyT"; }

	//mutable bool isUsed = true;
	//NamedObjectBase * parent = nullptr;
	//NamedObjectTracking tracked = TRACKED;

protected:
	stringPtr namePtr;
	Ex exp;
	mutable uint flags;

public:

	stringPtr strPtr() const {
		return namePtr;
	}

	std::string str() const {
		return *strPtr();
	}

};

inline Ex NamedObjectBase::getEx() &
{
	return getExRef();
}

inline Ex NamedObjectBase::getEx() const &
{
	return getExRef();
}

inline Ex NamedObjectBase::getEx() &&
{
	return getExTmp();
}

inline Ex NamedObjectBase::getEx() const &&
{
	return getExTmp();
}

inline Ex NamedObjectBase::getExRef()
{
	if (flags & ALWAYS_EXP) {
		return getExTmp();
	}
	flags = flags | IS_USED;
	if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
		ctor->setInit();
	}
	return alias();
}

inline Ex NamedObjectBase::getExTmp()
{
	flags = flags | IS_USED;
	if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
		ctor->setTemp();
		ctor->disable();
	}
	return exp;
}

inline Ex NamedObjectBase::getExRef() const
{
	if (flags & ALWAYS_EXP) {
		return getExTmp();
	}
	flags = flags | IS_USED;
	return alias();
}

inline Ex NamedObjectBase::getExTmp() const
{
	flags = flags | IS_USED;
	if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
		ctor->setTemp();
		ctor->disable();
	}
	return exp;
}

NamedObjectBase::~NamedObjectBase() {
	if (!isUsed()) {
		//std::cout << " ~ setTemp " << exp->str() << std::endl;
		if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
			ctor->setTemp();
		}
	}
	//should check inside scope

}

inline void NamedObjectBase::checkDisabling()
{
	if (!(flags & IS_TRACKED)) {
		//std::cout << "disabling " << str() << std::endl;
		exp->disable();
	}
}


template<typename T>
class NamedObject : public NamedObjectBase {

public:

	static std::string typeStr() { return "dummyNameObjT"; }

public:
	NamedObjectInit<T> operator<<(const std::string & s) const && {
		return { getExTmp() , s };
	}

protected:
	NamedObject(const std::string & _name = "", uint _flags = IS_TRACKED)
		: NamedObjectBase(_name, _flags)
	{
		checkName();

		exp = createDeclaration<T>(NamedObjectBase::strPtr(), 0);

		checkDisabling();
	}

	NamedObject(
		const Ex & _ex,
		uint ctor_flags = 0,
		uint obj_flags = IS_TRACKED,
		const std::string & s = ""
	) : NamedObjectBase(s, obj_flags)
	{
		checkName();
		exp = createInit<T>(strPtr(), FORWARD, ctor_flags, _ex);
		checkDisabling();
	}

	template<typename ... Args>
	NamedObject(
		uint ctor_flags,
		uint obj_flags,
		const std::string & s,
		const Args &... args
	) : NamedObjectBase(s, obj_flags)
	{
		checkName();
		exp = createInit<T>(strPtr(), INITIALISATION, ctor_flags, args ...);
		checkDisabling();
	}

	NamedObject(const NamedObjectInit<T> & obj_init) : NamedObjectBase(obj_init.name, IS_TRACKED | IS_USED)
	{
		checkName();

		exp = createInit<T>(strPtr(), INITIALISATION, 0, obj_init.exp);
	}

	void checkName()
	{
		if (*namePtr == "") {
			namePtr = std::make_shared<std::string>(getTypeNamingStr<T>() + "_" + std::to_string(counter));
			std::replace(namePtr->begin(), namePtr->end(), ' ', '_');

			//std::cout << "created " << str() << std::endl;
			++counter;
		}
	}

	static int counter;

public:

};
template<typename T> int NamedObject<T>::counter = 0;

template<typename T>
Ex getExp(T && t)
{
	return std::forward<T>(t).getEx();
}

//template<bool temp, typename T> Ex getExpForced(const T &t) {
//	return getExp<T, temp>(t);
//}

Ex NamedObjectBase::alias() const {
	return createExp<Alias>(strPtr());
}

template<> Ex getExp<bool>(bool && b) {
	return createExp<Litteral<bool>>(b);
}
template<> Ex getExp<bool&>(bool & b) {
	return createExp<Litteral<bool>>(b);
}

template<> Ex getExp<int>(int && i) {
	return createExp<Litteral<int>>(i);
}
template<> Ex getExp<int&>(int & i) {
	return createExp<Litteral<int>>(i);
}


template<> Ex getExp<uint>(uint && i) {
	return createExp<Litteral<uint>>(i);
}
template<> Ex getExp<uint&>(uint & i) {
	return createExp<Litteral<uint>>(i);
}

template<> Ex getExp<float>(float && d) {
	return createExp<Litteral<float>>(d);
}
template<> Ex getExp<float&>(float & d) {
	return createExp<Litteral<float>>(d);
}

template<> Ex getExp<double>(double && d) {
	return createExp<Litteral<double>>(d);
}
template<> Ex getExp<double&>(double & d) {
	return createExp<Litteral<double>>(d);
}

/////////////////////////////////////////////////////////////////////////

enum StatementOptions : uint {
	SEMICOLON = 1 << 0,
	COMMA = 1 << 1,
	NOTHING = 1 << 2,
	ADD_SPACE = 1 << 3,
	NEW_LINE = 1 << 4,
	IGNORE_DISABLE = 1 << 5,
	IGNORE_TRAILING = 1 << 6,
	DEFAULT = SEMICOLON | NEW_LINE
};

struct InstructionBase {
	using Ptr = std::shared_ptr<InstructionBase>;

	static std::string instruction_begin(int trailing, uint opts = 0) {
		std::string out;
		if ((opts & IGNORE_TRAILING) == 0) {
			for (int t = 0; t < trailing; ++t) {
				out += "   ";
			}
		}
		return out;
	}

	static std::string instruction_end(uint opts) {
		std::string out;
		if (opts & SEMICOLON) {
			out += ";";
		} else if (opts & COMMA) {
			out += ",";
		}
		if (opts & ADD_SPACE) {
			out += " ";
		} else if (opts & NEW_LINE) {
			out += "\n";
		}
		return out;
	}

	virtual void str(std::stringstream & stream, int & trailing, uint otps) { }
	virtual void cout(int & trailing, uint otps = DEFAULT) {}
	virtual void explore() {}
};

struct Block {
	using Ptr = std::shared_ptr<Block>;
	Block(const Block::Ptr & _parent = {}) : parent(_parent) {}

	virtual void push_instruction(const InstructionBase::Ptr & i) {
		instructions.push_back(i);
	}

	//template<typename S>
	//void push_statement(const S & s) {
	//	push_instruction(std::static_pointer_cast<InstructionBase>(s));
	//}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) { 
		for (const auto & inst : instructions) {
			inst->str(stream, trailing, opts);
		}
	}

	virtual void cout(int & trailing, uint opts = DEFAULT) {
		for (const auto & inst : instructions) {
			inst->cout(trailing);
		}
	}
	virtual void explore() {
		for (const auto & inst : instructions) {
			inst->explore();
		}
	}
	std::vector<InstructionBase::Ptr> instructions;
	Block::Ptr parent;
};

struct MainBlock : Block {
	using Ptr = std::shared_ptr<MainBlock>;
	int dummy;
};

struct ReturnBlockBase : Block {
	using Ptr = std::shared_ptr<ReturnBlockBase>;
	using Block::Block;

	virtual RunTimeInfos getType() const {
		return getRunTimeInfos<void>();
	}

	virtual bool same_return_type(size_t other_type_hash) const {
		return false;
	}

	bool hasReturnStatement = false;
};

template<typename ReturnType>
struct ReturnBlock : ReturnBlockBase {
	using Ptr = std::shared_ptr<ReturnBlock<ReturnType>>;

	ReturnBlock(const Block::Ptr & _parent = {} ) : ReturnBlockBase(_parent) {}
	
	virtual RunTimeInfos getType() const {
		return getRunTimeInfos<ReturnType>();
	}

	virtual bool same_return_type(size_t other_type_hash) const {
		//std::cout << "this : " << typeid(ReturnType).hash_code() << std::endl;
		//std::cout << " other : " << other_type_hash << std::endl;
		return typeid(ReturnType).hash_code() == other_type_hash;
	}

	virtual void str(std::stringstream & stream, int & trailing, uint otps);

	//virtual void cout(int & trailing, uint opts = DEFAULT);
};

//struct IfBlockT : Block {
//	using Ptr = std::shared_ptr<IfBlockT>;
//	using Block::Block;
//};

struct Statement : InstructionBase {
	using Ptr = std::shared_ptr<Statement>;
	
	Statement(const Ex & e) : ex(e) {}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) { 
		if ((opts & IGNORE_DISABLE) || !ex->disabled()) {
			stream <<
				((opts & NEW_LINE) ? instruction_begin(trailing, opts) : "")
				<< ex->str(trailing)
				<< instruction_end(opts);
		}
	}

	virtual void cout(int & trailing, uint opts = SEMICOLON & NEW_LINE) {
		if ( (opts & IGNORE_DISABLE) || !ex->disabled()) {
			std::cout <<
				( (opts & NEW_LINE) ? instruction_begin(trailing, opts) : "" )
				<< ex->str(trailing) 
				<< instruction_end(opts);
		}
	}

	void explore() {
		ex->explore();
	}
	Ex ex;
};

InstructionBase::Ptr toInstruction(const Ex & e) {
	auto statement = std::make_shared<Statement>(e);
	return std::dynamic_pointer_cast<InstructionBase>(statement);
}

struct EmptyStatement : Statement {

	EmptyStatement(uint _flags = 0) : flags(_flags), Statement(Ex()) {}

	static InstructionBase::Ptr create(uint _flags = 0) { 
		return std::static_pointer_cast<InstructionBase>(std::make_shared<EmptyStatement>(_flags)); 
	}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		if (flags != 0) {
			stream << Statement::instruction_begin(trailing, flags) << Statement::instruction_end(flags);
		} else {
			stream << Statement::instruction_begin(trailing, opts) << Statement::instruction_end(opts);
		}
	}

	virtual void cout(int & trailing, uint opts = 0) {
		if (flags != 0) {
			std::cout << Statement::instruction_begin(trailing, flags) << Statement::instruction_end(flags);
		} else {
			std::cout << Statement::instruction_begin(trailing, opts) << Statement::instruction_end(opts);
		}
		
	}

	uint flags;
};

struct ForArgsBlock : Block {
	using Ptr = std::shared_ptr<ForArgsBlock>;
	using Block::Block;

	enum Status { INIT, CONDITION, LOOP };

	Status status = INIT;
	Ex stacked_condition;

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		
		std::vector<InstructionBase::Ptr> inits, conditions, loops;
		Status status = INIT;
		auto it = instructions.begin();

		while (it != instructions.end()) {
			if (auto statement = std::dynamic_pointer_cast<Statement>(*it)) {
				if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(statement->ex)) {
					//std::cout << "for : " << ctor->str() << " " << ctor->ctor_status << std::endl;
					if (status == INIT) {
						if (ctor->ctor_status == INITIALISATION) {
							//std::cout << "init : " << ctor->str() << " " << ctor->ctor_status << std::endl;
							if (inits.size() > 0) {
								ctor->flags = ctor->flags | MULTIPLE_INITS;
							}
							inits.push_back(*it);
						} else if (ctor->is_bool_ctor()) {
							//std::cout << "init2 : " << ctor->str() << " " << ctor->ctor_status << std::endl;
							status = CONDITION;
						} else if(ctor->ctor_status == FORWARD){
							//std::cout << "init3 : " << ctor->str() << " " << ctor->ctor_status << std::endl;
							status = LOOP;
						}
					}

					if (status == CONDITION) {
						if (ctor->is_bool_ctor()) {
							conditions.push_back(*it);
						} else {
							status = LOOP;
						}
					}

					if (status == LOOP) {
						loops.push_back(*it);
					}
				}
			}
			++it;
		}
		
		if (inits.empty()) {
			inits.push_back(EmptyStatement::create());
		}
		if (conditions.empty()) {
			if (stacked_condition) {
				conditions.push_back(toInstruction(stacked_condition));
			} else {
				conditions.push_back(EmptyStatement::create());
			}
		}
		if (loops.empty()) {
			loops.push_back(EmptyStatement::create());
		}

		for (int i = 0; i < (int)inits.size() - 1; ++i) {
			inits[i]->str(stream, trailing, IGNORE_TRAILING | COMMA);
		}
		inits.back()->str(stream, trailing, IGNORE_TRAILING | SEMICOLON | ADD_SPACE);

		conditions.back()->str(stream, trailing, IGNORE_TRAILING | SEMICOLON | ADD_SPACE);

		loops.back()->str(stream, trailing, IGNORE_TRAILING | NOTHING);
	}

};

struct ReturnStatement : Statement {
	using Ptr = std::shared_ptr<ReturnStatement>;

	ReturnStatement(const Ex & e) : Statement(e) {}

	virtual void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON & NEW_LINE) {
		stream << instruction_begin(trailing, opts) << internal_str() << instruction_end(opts);
	}

	std::string internal_str() const {
		std::string s = "return";
		std::string ex_str = ex->str(0);
		if (ex_str != "") {
			s += " " + ex_str;
		}
		return s;
	}

	void explore() {
		ex->explore();
	}
};

struct CommentInstruction : InstructionBase {
	CommentInstruction(const std::string & s) : comment(s) {}

	virtual void str(std::stringstream & stream, int & trailing, uint otps = DEFAULT) {
		stream << instruction_begin(trailing) << "//" << comment << std::endl;
	}
	std::string comment;
};

template<typename ReturnType>
void ReturnBlock<ReturnType>::str(std::stringstream & stream, int & trailing, uint opts)
{
	Block::str(stream, trailing, opts);
	if (!std::is_same<ReturnType, void>::value && !ReturnBlockBase::hasReturnStatement) {
		CommentInstruction("need return statement here").str(stream, trailing, opts);
	}
}

struct ForInstruction : InstructionBase {
	using Ptr = std::shared_ptr<ForInstruction>;
	
	ForInstruction() {
		args = std::make_shared<ForArgsBlock>();
		body = std::make_shared<Block>();
	}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		stream << instruction_begin(trailing, opts) << "for( ";
		args->str(stream, trailing, IGNORE_TRAILING );
		stream << "){\n";
		++trailing;
		body->str(stream, trailing, opts);
		--trailing;
		stream << instruction_begin(trailing, opts) << "}\n";
	}

	//Statement::Ptr init, condition, loop;
	ForArgsBlock::Ptr args;
	Block::Ptr body;
};

struct WhileInstruction : InstructionBase {
	using Ptr = std::shared_ptr<WhileInstruction>;

	WhileInstruction(const Ex & ex, const Block::Ptr & parent ) {
		condition = std::make_shared<Statement>(ex);
		body = std::make_shared<Block>(parent);
	}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		stream << instruction_begin(trailing, opts) << "while( ";
		condition->str(stream, trailing, NOTHING | IGNORE_DISABLE);
		stream << " ){\n";
		++trailing;
		body->str(stream, trailing, opts);
		--trailing;
		stream << instruction_begin(trailing, opts) << "}\n";
	}

	Statement::Ptr condition;
	Block::Ptr body;	
};

struct IfInstruction : InstructionBase {
	using Ptr = std::shared_ptr<IfInstruction>;

	struct IfBody {
		Block::Ptr body;
		Statement::Ptr condition;
	};

	IfInstruction(std::shared_ptr<IfInstruction> _parent = {}) : parent_if(_parent) {}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		const int numBodies = (int)bodies.size();
		for (int i = 0; i < numBodies; ++i) {
			if (i == 0) {
				stream << instruction_begin(trailing, opts) << "if( ";
				bodies[0].condition->str(stream, trailing, NOTHING | IGNORE_DISABLE);
				stream << " ) {\n";
			} else if(bodies[i].condition) {
				stream << "else if( ";
				bodies[i].condition->str(stream, trailing, NOTHING | IGNORE_DISABLE);
				stream << " ) {\n";
			} else {
				stream << "else {\n";
			}

			++trailing;
			bodies[i].body->str(stream, trailing, DEFAULT);
			--trailing;
			stream << instruction_begin(trailing, opts) << "}" << (i == numBodies - 1 ? "\n" : " ");
		}
	}

	std::vector<IfBody> bodies;
	IfInstruction::Ptr parent_if;
	bool waiting_for_else = false;
};

enum SeparatorRule { SEP_IN_BETWEEN, SEP_AFTER_ALL };

template<SeparatorRule s, int N, typename ... Ts> struct DisplayDeclaration;

template<SeparatorRule s, int N, typename T, typename ... Ts>
struct DisplayDeclaration<s, N, T, Ts...> {
	static std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
		return InstructionBase::instruction_begin(trailing) + T::typeStr() + " " + v[v.size() - N] + 
			 ( (s==SEP_AFTER_ALL || ( s==SEP_IN_BETWEEN && N!=1) ) ? separator : "" ) + 
			DisplayDeclaration<s, N - 1, Ts...>::str(v, trailing, separator);
	}
};

template<SeparatorRule s, typename ... T>
struct DisplayDeclaration<s, 0,T...> {
	static std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts>
std::string memberDeclarations(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
	return DisplayDeclaration<s, sizeof...(Ts), Ts...>::str(v, trailing, separator);
}

template<SeparatorRule s, int N, typename ... Ts>
struct DisplayDeclarationTuple {
	static std::string str(const std::tuple<Ts...> & v, const std::string & separator) {
		return std::tuple_element_t<sizeof...(Ts) - N,std::tuple<Ts...> >::typeStr() + " " + 
			std::get<sizeof...(Ts) - N>(v).str() +
			((s == SEP_AFTER_ALL || (s == SEP_IN_BETWEEN && N != 1)) ? separator : "") +
			DisplayDeclarationTuple<s, N - 1, Ts...>::str(v, separator);
	}
};
template<SeparatorRule s, typename ... Ts>
struct DisplayDeclarationTuple<s, 0, Ts...> {
	static std::string str(const std::tuple<Ts...> & v, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts>
std::string memberDeclarationsTuple(const std::tuple<Ts...> & v, const std::string & separator) {
	return DisplayDeclarationTuple<s, sizeof...(Ts), Ts...>::str(v, separator);
}

template<typename ... Args>
struct StructDeclaration : InstructionBase {
	template<typename ... Strings>
	StructDeclaration(const std::string & _name, const Strings &... _names) : 
		name(_name), member_names{ _names... } {}

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		stream << instruction_begin(trailing, opts) << "struct " << name << " {\n";
		++trailing;
		stream << memberDeclarations<SEP_AFTER_ALL,Args...>(member_names, trailing, ";\n");
		--trailing;
		stream << instruction_begin(trailing, opts) << "}\n";
	}

	std::vector<std::string> member_names;
	std::string name;
};

template<QualifierType qType, typename ... Args>
struct InterfaceDeclaration : InstructionBase {
	
	template<typename ... Strings>
	static std::string str(const std::string & name, int trailing, const Strings &... member_names) {
		std::string out;
		int member_trailing = trailing + 1;
		out += QualifierTypeStr<qType>::str() + " " + name + " { \n";
		out += memberDeclarations<SEP_AFTER_ALL, Args...>({ member_names... }, member_trailing, ";\n");
		out += Statement::instruction_begin(trailing) + "}";
		return out;
	}
	
};
//template<int N, typename ... Strings>
//std::array<std::string,N> fill_args_names(const Strings & ... _argnames) {
//	return std::array<std::string, N>{_argnames...};
//}

template <typename ReturnType, typename... Args, typename ... Strings>
void init_function_declaration( const std::string & fname, const std::function<void(Args...)>& f, const Strings & ... args_name);

template<typename T> struct FunctionReturnType;
template<typename Lambda> typename FunctionReturnType<decltype(&Lambda::operator())>::type functionFromLambda(const Lambda &func);

template<typename ...Args> struct ArgTypeList {};
template<typename Arg, typename Arg2, typename ...Args> struct ArgTypeList<Arg,Arg2,Args...> {
	using first = ArgTypeList<Arg>;
	using rest = ArgTypeList<Arg2,Args...>;
};

template <typename ...Ts> constexpr bool SameTypeList = false;

template <>
constexpr bool SameTypeList<ArgTypeList<>, ArgTypeList<> > = true;

template <typename Input, typename Target>
constexpr bool SameTypeList<ArgTypeList<Input>, ArgTypeList<Target> > = IsConvertibleTo<Input, Target>; //EqualMat<Arg, ArgM>;

template <typename LA, typename LB>
constexpr bool SameTypeList<LA, LB> =
SameTypeList<typename LA::first,typename LB::first> && SameTypeList<typename LA::rest, typename LB::rest>;

template <typename List, typename ReturnType, typename ... Args>
void checkArgsType(const std::function<ReturnType(Args...)>& f){
	static_assert(SameTypeList < List, ArgTypeList<Args...> >, "arg types do not match function signature, or no implicit conversion available");
}

//template<typename Lambda>

struct FunBase : NamedObject<FunBase> {
	FunBase( const std::string & s = "") : NamedObject<FunBase>(s, 0) {}
	static std::string typeStr(int trailing = 0) { return "function"; }
};

template<typename ReturnType, typename F_Type>
struct Fun : FunBase {
	//using FunctionType = std::result_of_t< plugType<ReturnType>(F_Type));

	template<typename ... Strings>
	Fun( const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FunBase(_name), f(_f) {
		init_function_declaration<ReturnType>(str(), functionFromLambda(_f), _argnames...);
	}

	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	ReturnType operator()(R_Args &&  ... args) {
		checkArgsType<ArgTypeList<CleanType<R_Args>...> >(functionFromLambda(f));
		return { createFCallExp(str(), EX(R_Args, args)...) };
	}

	F_Type f;
};

template<typename ReturnType, typename F_Type, typename ... Strings >
Fun<ReturnType, F_Type> makeFun( const std::string & name, const F_Type & f, const Strings & ...argnames) {
	return Fun<ReturnType, F_Type>(name, f, argnames...);
}
template<typename ReturnType, typename F_Type, typename ... Strings , typename = std::enable_if_t<!std::is_convertible<F_Type,std::string>::value > >
Fun<ReturnType, F_Type> makeFun(const F_Type & f, const Strings & ...argnames) {
	return Fun<ReturnType, F_Type>("", f, argnames...);
}

struct FunctionDeclarationBase : InstructionBase {
	virtual Block::Ptr getBody() { return {}; }
};

template<typename ReturnType>
struct FunctionDeclarationRTBase : FunctionDeclarationBase {
	FunctionDeclarationRTBase() {
		body = std::make_shared<ReturnBlock<ReturnType>>();
	}
	virtual Block::Ptr getBody() { return std::static_pointer_cast<Block>(body); }

	typename ReturnBlock<ReturnType>::Ptr body;
};

template<typename ReturnType, typename ... Args>
struct FunctionDeclarationArgs : FunctionDeclarationRTBase<ReturnType> {
	
	FunctionDeclarationArgs( const std::string & _name, const std::tuple<Args...> & _args) : name(_name), args(_args) {	
	}

	std::string name;
	std::tuple<Args...> args;
};

template<typename ReturnT, typename ... Args>
struct FunctionDeclaration : FunctionDeclarationArgs<ReturnT,Args...> {
	using Base = FunctionDeclarationArgs<ReturnT, Args...>;
	using Base::Base;

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		stream << InstructionBase::instruction_begin(trailing, opts) << ReturnT::typeStr() << " " << Base::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(Base::args, ", ") << ") \n";
		stream << InstructionBase::instruction_begin(trailing, opts) << "{" << std::endl;
		++trailing;
		Base::getBody()->str(stream, trailing, DEFAULT);
		--trailing;
		stream << InstructionBase::instruction_begin(trailing, opts) << "}" << std::endl;
	}
};

//specialization for ReturnT == void
template<typename ... Args>
struct FunctionDeclaration<void, Args...> : FunctionDeclarationArgs<void,Args...> {
	using Base = FunctionDeclarationArgs<void, Args...>;
	using Base::Base;

	virtual void str(std::stringstream & stream, int & trailing, uint opts) {
		stream << InstructionBase::instruction_begin(trailing, opts) << "void " << Base::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(Base::args, ", ") << ") \n";
		stream << InstructionBase::instruction_begin(trailing, opts) << "{" << std::endl;
		++trailing;
		Base::getBody()->str(stream, trailing, DEFAULT);
		--trailing;
		stream << InstructionBase::instruction_begin(trailing, opts)  << "}" << std::endl;
	}
};

struct ControllerBase {

	void queueEvent(const Ex & e) {

		if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(e)) {
			if (auto cBlock = std::dynamic_pointer_cast<MainBlock>(currentBlock)) {
				if (ctor->position() == CtorPosition::INSIDE_BLOCK) {
					//std::cout << " not valid outside block : " << e->str() << std::endl;
				}
			} else {
				if (ctor->status() == DECLARATION && ctor->position() == CtorPosition::MAIN_BLOCK) {
					//std::cout << " not valid inside block : " << e->str() << std::endl;
				}
			}
		}

		//if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(e)) {
		//	if (ctor->position == MAIN_BLOCK) {
		//		std::cout << "main_block : " << ctor->str() << std::endl;
		//	}
		//}

		currentBlock->push_instruction(toInstruction(e));

		//currentBlock->instructions.push_back(toInstruction(e));
	}

	Block::Ptr currentBlock;
};

struct ForController : virtual ControllerBase {

	struct EndFor {
		~EndFor();

		explicit operator bool() {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}

		bool first = true;
	};

	void begin_for() {
		current_for = std::make_shared<ForInstruction>();
		currentBlock->push_instruction(current_for);
	}

	void begin_for_args() {	
		current_for->body->parent = currentBlock;
		currentBlock = current_for->args;
	}

	void begin_for_body() {
		currentBlock = current_for->body;
	}

	virtual void end_for() {
		currentBlock = currentBlock->parent;
	}

	void stack_for_condition(const Ex & ex) {
		if (current_for) {
			current_for->args->stacked_condition = ex;
		}
	}

	ForInstruction::Ptr current_for;
};


struct IfController : virtual ControllerBase {

	struct BeginIf {
		operator bool() const { return true; }
		~BeginIf();
	};

	struct BeginElse {
		operator bool() const { return false; }
		~BeginElse();
	};

	void begin_if(const Ex & ex) {

		current_if = std::make_shared<IfInstruction>(current_if);
		
		current_if->bodies.push_back({ std::make_shared<Block>(currentBlock), std::make_shared<Statement>(ex) });
		currentBlock->push_instruction(current_if);
		currentBlock = current_if->bodies.back().body;
	}

	void begin_else() {
		current_if->bodies.push_back({ std::make_shared<Block>(currentBlock->parent), {} });
		currentBlock = current_if->bodies.back().body;
		delay_end_if();
	}

	void begin_else_if(const Ex & ex) {
		current_if->bodies.push_back({ std::make_shared<Block>(currentBlock->parent), std::make_shared<Statement>(ex) });
		currentBlock = current_if->bodies.back().body;
	}

	void end_if_sub_block() {
		if (current_if->waiting_for_else) {
			end_if();
			end_if_sub_block();
		} else {
			current_if->waiting_for_else = true;
		}
		
	}
	void end_if() {
		//std::cout << " end if " << std::endl;
		current_if = current_if->parent_if;
		currentBlock = currentBlock->parent;
	}
	void delay_end_if() {
		if (current_if) {
			current_if->waiting_for_else = false;
		}
	}
	void check_begin_if() {
		if (current_if && current_if->waiting_for_else) {
			end_if();
		}
	}
	void check_end_if() {
		if (current_if  && current_if->waiting_for_else) {
			end_if();
		}
	}

	IfInstruction::Ptr current_if;
	
};

struct WhileController : virtual ControllerBase {

	struct BeginWhile {
		operator bool() const { 
			//std::cout << " BeginWhile operator bool() const " << first << std::endl;
			if (first) {
				first = false;
				return true;
			}
			return false;
		}
		mutable bool first = true;
		~BeginWhile();
	};

	void begin_while(const Ex & ex) {
		//std::cout << " begin while " << std::endl;
		auto while_instruction = std::make_shared<WhileInstruction>(ex, currentBlock);
		currentBlock->push_instruction(while_instruction);
		currentBlock = while_instruction->body;
	}

	virtual void end_while() {
		//std::cout << " end while " << std::endl;
		currentBlock = currentBlock->parent;
	}
};

struct MainController : virtual ForController, virtual WhileController, virtual IfController {
	using Ptr = std::shared_ptr<MainController>;
	
	void add_blank_line(int n = 0) {
		check_end_if();
		if (currentBlock) {
			for (int i = 0; i < n; ++i) {
				currentBlock->push_instruction(EmptyStatement::create(NEW_LINE | IGNORE_TRAILING | NOTHING));
			}			
		}
	}

	virtual void begin_for_args() {
		check_end_if();
		ForController::begin_for_args();
	}

	virtual void end_for() {
		check_end_if();
		ForController::end_for();
	}

	virtual void end_while() {
		check_end_if();
		WhileController::end_while();
	}

	void handleEvent(const Ex & e) {
	
		//if (for_status != NONE) {
		//	if (feed_for(e)) {
		//		return;
		//	}
		//} 

		check_end_if();

		queueEvent(e);
	}
};

struct ShaderBase : MainController {
	using Ptr = std::shared_ptr<ShaderBase>;

	ShaderBase() {
		declarations = std::make_shared<MainBlock>();
		currentBlock = declarations;

		//add defaut main
		//functions.push_back(std::make_shared<Block>(createExp<>()))
	}

	virtual std::string header() const { return ""; }

	MainBlock::Ptr declarations;
	std::vector<InstructionBase::Ptr> structs; 
	std::vector<InstructionBase::Ptr> functions;
	

	std::string str() const {
		int trailing = 1;

		std::stringstream out;

		out << Statement::instruction_begin(trailing) << header() << "\n\n";

		for (const auto & struc : structs) {
			struc->str(out, trailing, DEFAULT);
			out << "\n";
		}

		declarations->str(out, trailing, DEFAULT);
		out << "\n";
		for (const auto & fun : functions) {
			fun->str(out, trailing, DEFAULT);
			out << "\n";
		}

		return out.str();
	}

	void explore() {
		declarations->explore();
		for (const auto & struc : structs) {
			//struc->cout();
		}
	}

	template<typename ...Args, typename ... Strings> 
	void add_struct(const std::string & name, const Strings & ... names) {
		auto struct_declaration = std::make_shared < StructDeclaration<Args...> >(name, names...);
		structs.push_back(std::static_pointer_cast<InstructionBase>(struct_declaration));
	}

	template<typename ReturnT, typename ...Args>
	void begin_function( const std::string & name, const std::tuple<Args...> & args) {
		auto function_declaration = std::make_shared < FunctionDeclaration<ReturnT,Args...> >(name, args );
		functions.push_back(std::static_pointer_cast<InstructionBase>(function_declaration));
		function_declaration->body->parent = currentBlock;
		//std::cout << "begin_function : " << (bool)(std::dynamic_pointer_cast<MainBlock>(currentBlock)) << std::endl;
		currentBlock = std::static_pointer_cast<Block>(function_declaration->body);
		//std::cout << " in_function : " << (bool)(std::dynamic_pointer_cast<MainBlock>(currentBlock)) << std::endl;
	}
	void end_function() {
		currentBlock = std::static_pointer_cast<FunctionDeclarationBase>(functions.back())->getBody()->parent;
		//std::cout << " end_function : " << (bool)(std::dynamic_pointer_cast<MainBlock>(currentBlock)) << std::endl;
	}

	ReturnBlockBase::Ptr get_return_block() {
		Block::Ptr cBlock = currentBlock;
		bool found_return_block = false;
		while (!found_return_block) {
			if (auto testblock = std::dynamic_pointer_cast<ReturnBlockBase>(cBlock)) {
				found_return_block = true;
			} else if (cBlock->parent) {
				cBlock = cBlock->parent;
			} else {
				break;
			}
		}
		return std::dynamic_pointer_cast<ReturnBlockBase>(cBlock);
	}

	void add_return_statement() {
		auto return_statement = std::make_shared<ReturnStatement>(EmptyExp::get());
		if (auto return_block = get_return_block()) {
			if (return_block->getType().is_void()) {
				currentBlock->push_instruction(return_statement);
			} else {
				currentBlock->push_instruction(
					std::make_shared<CommentInstruction>("unexpected " + return_statement->internal_str() + "; in non void function ")
				);
			}
		}
	}

	template<typename R_T>
	void add_return_statement(R_T && t) {
		using T = CleanType<R_T>;
		Ex ex = EX(R_T, t);
		auto return_statement = std::make_shared<ReturnStatement>(ex);

		if (auto return_block = get_return_block()) {
			bool same_type = return_block->same_return_type(typeid(T).hash_code());
			if (same_type || return_block->getType().isConvertibleTo(getRunTimeInfos<T>())) {
				currentBlock->push_instruction(return_statement);
				return_block->hasReturnStatement = true;
			} else {
				currentBlock->push_instruction(std::make_shared<CommentInstruction>("wrong result type in : return " + ex->str(0)));
			}
		}
	}
};

template<GLVersion version>
struct IShader : ShaderBase {
	using Ptr = std::shared_ptr<IShader>;

	virtual std::string header() const {
		return "#version " + gl_version_str<version>();
	}
};

template<GLVersion version>
struct ShaderWrapper {

	ShaderWrapper() {
		shader_ptr = std::make_shared<IShader<version>>();
		listen().currentShader = shader_ptr;
	}

	template<typename F_Type, typename ... Strings >
	void main(const F_Type & f, const Strings & ...argnames) {
		Fun<void, F_Type>("main", f, argnames...);
	}

	std::string str() {
		if (shader_ptr) {
			return shader_ptr->str();
		} else {
			return "";
		}
	}

	typename IShader<version>::Ptr shader_ptr;
};

struct MainListener {
	
	MainListener() {
		//shader = std::make_shared<TShader>();
		//currentShader = shader;
	}

	void add_blank_line(int n = 0){
		if (currentShader) {
			currentShader->add_blank_line(n);
		}
	}

	/////////////////////////////////////////////////

	void addEvent(const Ex & ex) {
		if (currentShader && isListening) {
			currentShader->handleEvent(ex);
		}
	}
	
	/////////////////////////////////////////////////

	//void add_return_statement(const Ex & ex) {
	//	if (currentShader) {
	//		currentShader->add_return_statement(ex);
	//	}
	//}

	void add_return_statement() {
		if (currentShader) {
			currentShader->add_return_statement();
		}
	}

	template<typename T>
	void add_return_statement(T && t) {
		if (currentShader) {
			currentShader->add_return_statement(std::forward<T>(t));
		}
	}

	/////////////////////////////////////////////////

	void begin_for() {
		if (currentShader) {
			currentShader->begin_for();
		}
	}

	void begin_for_args() {
		if (currentShader) {
			currentShader->begin_for_args();
		}
	}

	void begin_for_body() {
		if (currentShader) {
			currentShader->begin_for_body();
		}

	}
	void end_for() {
		if (currentShader) {
			currentShader->end_for();
		}
	}

	void stack_for_condition(const Ex & ex) {
		if (currentShader) {
			currentShader->stack_for_condition(ex);
		}
	}

	/////////////////////////////////////////////////

	template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
	void begin_if(B && b) {
		if (currentShader) {
			currentShader->begin_if(EX(B, b));
		}
	}
	void begin_else() {
		if (currentShader) {
			currentShader->begin_else();
		}
	}
	template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
	void begin_else_if(B && b) {
		if (currentShader) {
			currentShader->begin_else_if(EX(B, b));
		}
	}
	void end_if_sub_block() {
		if (currentShader) {
			currentShader->end_if_sub_block();
		}
	}
	void end_if() {
		if (currentShader) {
			currentShader->end_if();
		}
	}
	void check_begin_if() {
		if (currentShader) {
			currentShader->check_begin_if();
		}
	}
	void delay_end_if() {
		if (currentShader) {
			currentShader->delay_end_if();
		}
	}

	/////////////////////////////////////////////////

	template<typename R_B, typename = std::enable_if_t< EqualType<CleanType<R_B>, Bool> > >
	void begin_while(R_B && b) {
		if (currentShader) {
			currentShader->begin_while(getExp<R_B>(b));
		}
	}

	void end_while() {
		if (currentShader) {
			currentShader->end_while();
		}
	}

	/////////////////////////////////////////////////

	template<bool dummy, typename ...Args, typename ... Strings>
	void add_struct(const std::string & name, const Strings & ... names) {
		if (currentShader) {
			currentShader->add_struct<Args...>(name, names...);
		}
	}

	/////////////////////////////////////////////////

	template<typename ReturnT, typename ...Args>
	void begin_function( const std::string & name, const std::tuple<Args...> & args) {
		if (currentShader) {
			currentShader->begin_function<ReturnT, Args...>(name, args);
		}
	}
	void end_function() {
		if (currentShader) {
			currentShader->end_function();
		}
	}

	/////////////////////////////////////////////////

	void explore() {
		if (currentShader) {
			std::cout << "############## explore ############" << std::endl;
			currentShader->explore();
			std::cout << "###################################" << std::endl;
		}
	}

	bool & active() { return isListening; }

	//TShader::Ptr shader;
	ShaderBase::Ptr currentShader;
	bool isListening = true;
	static MainListener overmind;
};

MainListener MainListener::overmind = MainListener();

MainListener & listen() { return MainListener::overmind; }

void lineBreak(int n = 1) { listen().add_blank_line(n); }

//specialization of Fun when ReturnType == void
template<typename F_Type>
struct Fun<void, F_Type> : FunBase {
	template<typename ... Strings>
	Fun( const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FunBase(_name), f(_f) {
		init_function_declaration<void>(NamedObjectBase::str(), functionFromLambda(_f), _argnames...);
	}
	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	void operator()(R_Args &&  ... args) {
		checkArgsType<ArgTypeList<CleanType<R_Args>...> >(functionFromLambda(f));
		listen().addEvent(createFCallExp(NamedObjectBase::str(), EX(R_Args, args)...));
	}

	F_Type f;
};

//template<typename T, CtorTypeDisplay tRule, OperatorDisplayRule  dRule, ParenthesisRule pRule, CtorStatus status, typename ... Args>
//Ex createInit(const stringPtr & name, const Args &... args)
//{
//	auto ctor = std::make_shared<Ctor<T, dRule, pRule, tRule>>(name, (sizeof...(args) != 0), status);
//	Ex expr = std::make_shared<Exp>(std::static_pointer_cast<OpBase>(ctor), std::vector<Ex>{args...});
//	listen().addEvent(expr);
//	return expr;
//}

template<typename T, typename ... Args>
Ex createInit(const stringPtr & name, CtorStatus status, uint ctor_flags, const Args &... args) 
{
	//std::cout << "ctor : " << *name << " " << (bool)(ctor_flags & PARENTHESIS) << std::endl;
	auto ctor = std::make_shared<Constructor<T, sizeof...(args)>>(name, status, ctor_flags, args...);
	Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
	listen().addEvent(expr);
	return expr;
}

template<typename T>
struct CreateDeclaration {
	
	template<typename ... Args>
	static Ex run(
		const stringPtr & name,	CtorStatus status, uint flags, const Args &... args)
	{
		return createInit<T, Args...>(name, status, flags, args...);
	}
};

//template<QualifierType type, typename T, typename L>
//struct CreateDeclaration<Qualifier<type, T, L>> {
//	template<typename ... Args>
//	static Ex run(const stringPtr & name, const Args &... args) {
//		Ex e = createInit<Qualifier<type, T, L>, DISPLAY, USE_PARENTHESIS, DECLARATION, MAIN_BLOCK, Args...>(name, args...);
//		std::cout << "mb" << e->str() << std::endl;
//		return e;
//	}
//};

template<typename T, typename ... Args>
Ex createDeclaration(const stringPtr & name, uint flags, const Args &... args) {
	return CreateDeclaration<T>::run(name, DECLARATION, flags, args...);
}

//template<typename T>
//struct CreateArrayDeclaration {	
//	static Ex run(const stringPtr & name, int n) {
//		auto ctor = std::make_shared<Constructor<Array<T>>>(name, DECLARATION, n);
//		Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
//		listen().addEvent(expr);
//		return expr;
//	}
//};
//
//template<typename T>
//Ex createArrayDeclaration(const stringPtr & name, int n) {
//	return CreateArrayDeclaration<T>::run(name, n);
//}

//template<QualifierType type, typename T, typename L>
//struct CreateArrayDeclaration<Qualifier<type, T, L>> {
//
//	static Ex run(const stringPtr & name) {
//		auto ctor = std::make_shared<Constructor<Qualifier<type, T, L>,0,DISPLAY,USE_PARENTHESIS,MAIN_BLOCK>>(name, DECLARATION);
//		Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
//		std::cout << "mb" << expr->str() << std::endl;
//		listen().addEvent(expr);
//		return expr;
//	}
//};
//



//enum class Matrix_Track { UNTRACKED };


template<unsigned int S, typename... Args> struct TupleBuilder;

template<unsigned int S, typename... Args> std::tuple<Args...> getTuple(std::array<std::string, S> & names) {
	return TupleBuilder<S, Args...>::tup(names);
}
template<unsigned int S> struct TupleBuilder<S> {
	static std::tuple<> tup(std::array < std::string, S> & names) {
		return std::tuple<>();
	}
};

template<unsigned int S, typename Arg, typename... Args>
struct TupleBuilder<S, Arg, Args...> {
	static std::tuple<Arg, Args...> tup(std::array<std::string, S> & names) {
		return std::tuple_cat(
			std::tuple<Arg>( Arg( names[S - sizeof...(Args) - 1], 0 ) ),
			getTuple<S, Args...>(names)
		);
	}
};


template <typename ReturnType, typename... Args, typename ... Strings>
void init_function_declaration( const std::string & fname, const std::function<void(Args...)>& f, const Strings & ... args_name)
{
	static_assert(sizeof...(Strings) <= sizeof...(Args), "too many string arguments for function declaration");
	
	std::array<std::string, sizeof...(Args)> args_names_array{args_name...};
	
	std::tuple<Args...> args = getTuple<sizeof...(Args),Args...>(args_names_array);

	listen().begin_function<ReturnType, Args...>(fname, args);
	
	call_from_tuple(f, args);

	listen().end_function();
}

template<typename ...Ts> struct ReturnStT;

template<typename T>
struct ReturnStT<T> {
	static void return_statement(T && t) {
		//checkForTemp<T>(t);
		//listen().add_return_statement(getExp<T>(t));
		listen().add_return_statement(std::forward<T>(t));
	}
};

template<> struct ReturnStT<> {
	static void return_statement() {
		listen().add_return_statement();
	}
};

template<typename ...Ts>
void return_statement(Ts && ... ts) {
	ReturnStT<Ts...>::return_statement(std::forward<Ts>(ts)...);
}


#define GL_RETURN return_statement
#define GL_RETURN_TEST(...) if(false){ return __VA_ARGS__; } return_statement( __VA_ARGS__ )

ForController::EndFor::~EndFor() {
	listen().end_for();
}


#define GL_FOR(...) listen().begin_for(); listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
listen().begin_for_args(); __VA_ARGS__;  listen().begin_for_body(); \
for(ForController::EndFor csl_dummy_for; csl_dummy_for; )


IfController::BeginIf::~BeginIf() {
	listen().end_if_sub_block();
}
IfController::BeginElse::~BeginElse() {
	listen().end_if();
}

#define GL_IF(condition) listen().check_begin_if(); listen().begin_if(condition); if(IfController::BeginIf csl_begin_if = {})

#define GL_ELSE else {} listen().begin_else(); if(IfController::BeginElse csl_begin_else = {}) {} else 

#define GL_ELSE_IF(condition) else if(false){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(IfController::BeginIf csl_begin_else_if = {})


WhileController::BeginWhile::~BeginWhile() {
	listen().end_while();
}

#define GL_WHILE(condition) listen().begin_while(condition); for(WhileController::BeginWhile csl_begin_while = {}; csl_begin_while; )

//////////////////////////////////////

