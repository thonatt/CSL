#pragma once

#include "StringHelpers.h"

namespace csl {

inline stringPtr makeStringPtr(const std::string & s) {
	auto str_ptr = std::make_shared<std::string>(s);
	//std::cout << " str_cv : " <<  (int)(bool)str_ptr << " " << s << std::endl;
	return str_ptr;
}

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
	MULTIPLE_INITS = 1 << 3,
	PARENTHESIS = 1 << 4,
	MAIN_BLOCK = 1 << 5
};

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

	OperatorBase(uint _flags = 0) : flags(_flags) {}
	virtual ~OperatorBase() = default;

	virtual std::string str(int trailing) const {
		return "no_str";
	}

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
	virtual ~Precedence() = default;
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
		: OperatorBase(_flags), ctor_status(_status) { }

	virtual ~ConstructorBase() = default;

	CtorPosition position() const {
		if (flags & MAIN_BLOCK) {
			return CtorPosition::MAIN_BLOCK;
		} else {
			return CtorPosition::INSIDE_BLOCK;
		}
	}

	void setTemp() {
		if (status() == INITIALISATION ) { // || status() == DECLARATION || status() == FORWARD ) {
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

	virtual std::string obj_name() const {
		return "base_ctor_obj_name";
	}

	virtual Ex firstArg() { return {}; }

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

	virtual ~Constructor() = default;

	virtual std::string obj_name() const {
		return *obj_name_ptr;
	}

	std::string lhs_type_str(int trailing) const {
		if (flags & MULTIPLE_INITS) {
			return "";
		}
		return getTypeStr<T>(trailing);
	}

	virtual std::string rhs_type_str() const {
		return TypeStrRHS<T>::str();
	}

	virtual std::string lhs_str(int trailing) const {
		return DeclarationStr<T>::str(obj_name(), trailing); // lhs_type_str(trailing) + " " + obj_name();
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
		return is_same_v<T, Bool>;
	}

	virtual Ex firstArg() {
		if (N > 0) {
			return ArgsCall<N>::args[0];
		} else {
			return {};
		}
	}

	stringPtr obj_name_ptr;
};

//template<typename T, uint N, uint M>
//struct Constructor<Array<T, N>, M> : Constructor<T, M> {
//
//	template<typename ... Args>
//	Constructor(stringPtr _obj_name_ptr, CtorStatus _status, uint flags, const Args & ...  _args)
//		: Constructor<T, M>(_obj_name_ptr, _status, flags, _args...) {
//	}
//
//	virtual std::string lhs_str(int trailing) const {
//		return Constructor<T, M>::lhs_str(trailing) + TypeStr< Array<T, N> >::array_str();
//	}
//
//	virtual std::string rhs_type_str() const {
//		return getTypeStr<T>() + TypeStr< Array<T, N> >::array_str();
//	}
//};

struct MemberAccessor : Precedence<FIELD_SELECTOR> {
	MemberAccessor(const Ex & _obj, const std::string & _member_str)
		: obj(_obj), member_str(_member_str) {
	}
	std::string str(int trailing) const {
		std::string obj_str;
		if (obj_is_temp) {
			obj_str = OperatorBase::checkForParenthesis(obj);
		} else {
			if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(obj)) {
				obj_str = ctor->obj_name();
			}
		}
		return obj_str + "." + member_str;
	}

	void make_obj_tmp() {
		if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(obj)) {
			ctor->setTemp();
			ctor->disable();
		}
		obj_is_temp = true;
	}

	Ex obj;
	std::string member_str;
	bool obj_is_temp = false;
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
		// std::to_string(i);
		//s.erase(s.find_last_not_of('0') + 1, std::string::npos);
		//s.erase(s.find_last_not_of('.') + 1, std::string::npos);
		//return s;
	
		std::stringstream ss;
		ss << std::setprecision(8) << std::showpoint << i << LitteralSufffixStr<Infos<T>::scalar_type>::str();
		std::string out = ss.str();
		return out;
		//return ss.str();
		//return std::to_string(i); 
	}

	T i;
};

template<> struct Litteral<bool> : OperatorBase {
	Litteral(const bool & _b) : b(_b) {}
	virtual std::string str(int trailing) const { return b ? "true" : "false"; }
	bool b;
};

} //namespace csl
