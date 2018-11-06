#pragma once


#include <vector>
#include <array>
#include <memory>
#include <algorithm> //for std::replace

#include "StringHelpers.h"
#include "FunctionHelpers.h"

struct MainListener;
MainListener & listen();

using stringPtr = std::shared_ptr<std::string>;

stringPtr makeStringPtr(const std::string & s) {
	auto str_ptr = std::make_shared<std::string>(s);
	//std::cout << " str_cv : " <<  (int)(bool)str_ptr << " " << s << std::endl;
	return str_ptr;
}


struct OperatorBase;
using Ex = std::shared_ptr<OperatorBase>;

enum NamedObjectTracking { NOT_TRACKED, TRACKED };

class NamedObjectBase {
public:
	NamedObjectBase(const std::string & _name = "", NamedObjectTracking _tracked = TRACKED , NamedObjectBase * _parent = nullptr, bool _isUsed = true)
		: parent(_parent), isUsed(_isUsed), tracked(_tracked) {
		namePtr = std::make_shared<std::string>(_name);
		//std::cout << " end check" << std::endl;
	}

	~NamedObjectBase();

	Ex alias() const;

	static const std::string typeStr() { return "dummyT"; }
	stringPtr namePtr;
	mutable bool isUsed = true;
	NamedObjectBase * parent = nullptr;
	NamedObjectTracking tracked = TRACKED;

public:
	Ex exp;

public:
	const stringPtr myNamePtr() const { return (parent ? std::make_shared<std::string>(parent->myName() + "." + *namePtr) : namePtr); }
	const std::string myName() const { return *myNamePtr(); }

};

template<typename T>
class NamedObject : public NamedObjectBase {
public:

	static const std::string typeStr() { return "dummyNameObjT"; }

protected:
	NamedObject(const std::string & _name = "", NamedObjectTracking _tracked = TRACKED, NamedObjectBase * _parent = nullptr, bool _isUsed = true) :
		NamedObjectBase(_name, _tracked, _parent, _isUsed) {
		if (_name == "") {
			namePtr = std::make_shared<std::string>(getTypeStr<T>() + "_" + std::to_string(counter));
			std::replace(namePtr->begin(), namePtr->end(), ' ', '_');

			++counter;
		}
	}
	static int counter;

public:

};
template<typename T> int NamedObject<T>::counter = 0;

enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN, IN_BETWEEN_NOSPACE, BEHIND };
enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };
enum CtorTypeDisplay { DISPLAY, HIDE};
enum CtorStatus { DECLARATION, INITIALISATION, TEMP };

enum StatementOptions {
	SEMICOLON = 1 << 0,
	COMMA = 1 << 1,
	NOTHING = 1 << 2,
	NEW_LINE = 1 << 3,
	IGNORE_DISABLE = 1 << 4,
	DEFAULT = SEMICOLON | NEW_LINE 
};

enum OperatorPredence {

	ALIAS = 1,

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
	OperatorBase(stringPtr _op_str_ptr = {}) : op_str_ptr(_op_str_ptr) {
		if (!op_str_ptr) {
			op_str_ptr = std::make_shared<std::string>(" >no_str< ");
		}
	}

	virtual const std::string str() const {
		return "no_str";
	}

	const std::string op_str() const {
		return *op_str_ptr;
	}

	const std::string explore() const {
		return "no_exploration";
	}

	virtual uint rank() const {
		return 0;
	}

	const bool inversion(Ex other) const {
		return rank() < other->rank();
	}

	const std::string checkForParenthesis(Ex exp) const {
		if (inversion(exp)) {
			return "(" + exp->str() + ")";
		} 
		return exp->str();
	}

	stringPtr op_str_ptr;
	bool disabled = false;
};

template<OperatorPredence predecence>
struct Predecense : OperatorBase {
	Predecense(stringPtr _op_str_ptr = {}) : OperatorBase(_op_str_ptr) {}
	virtual uint rank() const { return (int)predecence; }
};

struct Alias : Predecense<ALIAS> {
	Alias(stringPtr _obj_str_ptr) : obj_str_ptr(_obj_str_ptr) {}
	Alias(const std::string & s) : obj_str_ptr(makeStringPtr(s)) {}
	virtual const std::string str() const { return *obj_str_ptr; }
	
	stringPtr obj_str_ptr;
}; 

template<OperatorPredence predecence>
struct MiddleOperator : Predecense<predecence> {
	MiddleOperator(const std::string & op_str, Ex _lhs, Ex _rhs)
		: Predecense<predecence>(makeStringPtr(op_str)), lhs(_lhs), rhs(_rhs) {
		//std::cout << "middle op : " << op_str << " " << *OperatorBase::op_str_ptr << std::endl;
 	}

	const std::string str() const {
		return OperatorBase::checkForParenthesis(lhs) + OperatorBase::op_str() + OperatorBase::checkForParenthesis(rhs);
	}

	Ex lhs, rhs;
};

template<uint N>
struct ArgsCall {

	template<typename ... Args>
	ArgsCall(const Args & ... _args) : args{ _args... } {}

	const std::string args_str_body() const {
		std::string out = "";
		for (uint i = 0; i < N; ++i) {
			out += args[i]->str() + ((i == N - 1) ? "" : ", ");
		}
		return out;
	}

	const std::string args_str() const {
		return "(" + args_str_body() + ")";
	}

	std::vector<Ex> args;
};

template<uint N>
struct FunctionCall : Predecense<FUNCTION_CALL>, ArgsCall<N> {

	template<typename ... Args>
	FunctionCall(const std::string & s, const Args & ... _args)
		: Predecense<FUNCTION_CALL>(makeStringPtr(s)), ArgsCall<N>(_args...) {
	}

	virtual const std::string str() const {
		return op_str() + ArgsCall<N>::args_str();
	}

};

struct ConstructorBase : OperatorBase {
	ConstructorBase(CtorStatus _status = INITIALISATION) : status(_status) { }
	CtorStatus status;
};

template<typename T, uint N = 0, CtorTypeDisplay typeDisplay = DISPLAY, ParenthesisRule parenthesis = USE_PARENTHESIS>
struct Constructor : ArgsCall<N>, ConstructorBase {

	virtual uint rank() const {
		if (N == 0) {
			return (uint)FUNCTION_CALL;
		} 
		return ArgsCall<N>::args[0]->rank();
	}

	template<typename ... Args>
	Constructor(stringPtr _obj_name_ptr, CtorStatus _status, const Args & ...  _args)
		: ArgsCall<N>(_args...), ConstructorBase(_status), obj_name_ptr(_obj_name_ptr) {
		//std::cout << "ctpr" << _status << " " << status << std::endl;
	}

	const std::string obj_name() const {
		return *obj_name_ptr;
	}

	const std::string type_str() const {
		return getTypeStr<T>();
	}

	const std::string lhs_str() const {
		return type_str() + " " + obj_name();
	}

	const std::string rhs_str() const {
		return (typeDisplay == DISPLAY ? type_str() : std::string("")) + 
			(parenthesis == USE_PARENTHESIS	? ArgsCall<N>::args_str() : ArgsCall<N>::args_str_body());
	}

	const std::string str() const {
		if (status == INITIALISATION) {
			return lhs_str() + " = " + rhs_str();
		} else if (status == DECLARATION) {
			return lhs_str();
		} else {
			return rhs_str();
		}
	}

	stringPtr obj_name_ptr;
};

template<typename T, uint M>
struct Constructor<Array<T, M>> : Constructor<T> {
	Constructor(stringPtr _obj_name_ptr, CtorStatus _status)
		: Constructor<T>(_obj_name_ptr, _status) {
	}

	const std::string str() const {
		return Constructor<T>::str() + "[" + std::to_string(M) + "]";
	}
};

struct FieldSelector : Predecense<FIELD_SELECTOR> {
	FieldSelector(Ex _obj, stringPtr member_str)
		: member_str_ptr(member_str), obj(_obj) {
	}
	const std::string str() const {
		return obj->str() + "." + *member_str_ptr;
	}
	Ex obj;
	stringPtr member_str_ptr;
};

template<uint N>
struct MemberSelector : FunctionCall<N> {
	template<typename ...Args>
	MemberSelector(Ex _obj, const std::string & fun_name, const Args & ... _args) 
		: FunctionCall<N>(fun_name, _args...) , obj(_obj) {
	}

	const std::string str() const {
		return OperatorBase::checkForParenthesis(obj) + "." + FunctionCall<N>::str();
	}

	Ex obj;
};

struct ArraySubscript : Predecense<ARRAY_SUBSCRIPT> {
	ArraySubscript(Ex _obj, Ex _arg) : obj(_obj), arg(_arg) {
	}

	const std::string str() const {
		return OperatorBase::checkForParenthesis(obj) + "[" + arg->str() + "]";
	}

	Ex obj, arg;
};

struct PrefixUnary : Predecense<PREFIX> {
	PrefixUnary(const std::string & op_str, Ex _obj)
		: Predecense<PREFIX>(makeStringPtr(op_str)), obj(_obj) {
	}

	const std::string str() const {
		return op_str() + OperatorBase::checkForParenthesis(obj);
	}

	Ex obj;
};

struct PostfixUnary : Predecense<POSTFIX> {
	PostfixUnary(const std::string & op_str, Ex _obj)
		: Predecense<POSTFIX>(makeStringPtr(op_str)), obj(_obj) {
	}

	const std::string str() const {
		return OperatorBase::checkForParenthesis(obj) + op_str();
	}

	Ex obj;
};

struct Ternary : Predecense<TERNARY> {
	Ternary(Ex _condition, Ex _first, Ex _second)  
		: condition(_condition), first(_first), second(_second) {
	}

	const std::string str() const {
		return OperatorBase::checkForParenthesis(condition) + " ? " + OperatorBase::checkForParenthesis(first) + " : " + OperatorBase::checkForParenthesis(second);
	}

	Ex condition, first, second;
};

struct EmptyExp : OperatorBase {
	static Ex get() { return std::static_pointer_cast<OperatorBase>(std::make_shared<EmptyExp>()); }

	const std::string str() const {
		return "";
	}
};

template<typename T>
struct Litteral : OperatorBase {
	Litteral(const T & _i) : i(_i) {}
	virtual const std::string str() const { return std::to_string(i); }
	T i;
};

template<> struct Litteral<bool> : OperatorBase {
	Litteral(const bool & _b) : b(_b) {}
	virtual const std::string str() const { return b ? "true" : "false"; }
	bool b;
};

void isNotInit(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(expr)) {
		//ctor->isInit = false;
		ctor->disabled = true;
		ctor->status = TEMP;
	}
}

template<typename ...Args> void areNotInit(const Args &... args);
template<typename A, typename ...Args>
void areNotInit(const A & a, const Args &... args) {
	areNotInit(a);
	areNotInit(args...);
}
template<typename A> void areNotInit(const A & a) {
	//std::cout << "not init " << a.name << std::endl;
	isNotInit(a.exp);
}
template<> void areNotInit() {}
template<> void areNotInit<bool>(const bool &) {}
template<> void areNotInit<int>(const int &) {}
template<> void areNotInit<double>(const double &) {}

template<typename Operator, typename ... Args>
Ex createExp(const Args &... args) {
	return std::static_pointer_cast<OperatorBase>(std::make_shared<Operator>(args...));
}

template<typename T, bool temp = !std::is_lvalue_reference<T>::value> Ex getExp(const T & t) {
	//std::cout << t.exp.use_count() << std::endl;
	t.isUsed = true;
	if(temp) {
		areNotInit(t);
		//std::cout << " get exp temp " << t.myName() << std::endl;
		return t.exp;
	} else {
		//std::cout << " ref use " << t.myName() << std::endl;		
		return createExp<Alias>(t.myNamePtr());
	}
}

template<bool temp, typename T> Ex getExpForced(const T &t) {
	return getExp<T, temp>(t);
}

Ex NamedObjectBase::alias() const {
	return createExp<Alias>(myNamePtr());
}

template<> Ex getExp<bool>(const bool & b) {
	return createExp<Litteral<bool>>(b);
}

template<> Ex getExp<int>(const int & i) {
	return createExp<Litteral<int>>(i);
}

template<> Ex getExp<double>(const double & d) {
	return createExp<Litteral<double>>(d);
}

struct InstructionBase {
	using Ptr = std::shared_ptr<InstructionBase>;

	static const std::string instruction_begin(int trailing) {
		std::string out;
		for (int t = 0; t < trailing; ++t) {
			out += "   ";
		}
		return out;
	}
	static const std::string instruction_end(uint opts) {
		return std::string(opts & SEMICOLON ? ";" : opts & COMMA ? "," : "") + (opts & NEW_LINE ? "\n" : "");
	}

	virtual void cout(int & trailing, uint otps = DEFAULT) {}
	virtual void explore() {}
};

struct Block {
	using Ptr = std::shared_ptr<Block>;
	Block(const Block::Ptr & _parent = {}) : parent(_parent) {}

	template<typename I>
	void push_instruction(const I & i) {
		instructions.push_back(std::static_pointer_cast<InstructionBase>(i));
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

struct ReturnBlockBase : Block {
	using Ptr = std::shared_ptr<ReturnBlockBase>;
	using Block::Block;

	virtual RunTimeInfos getType() {
		return getRunTimeInfos<void>();
	}

	bool hasReturnStatement = false;
};

template<typename ReturnType>
struct ReturnBlock : ReturnBlockBase {
	using Ptr = std::shared_ptr<ReturnBlock<ReturnType>>;

	ReturnBlock(const Block::Ptr & _parent = {} ) : ReturnBlockBase(_parent) {}
	virtual RunTimeInfos getType() {
		return getRunTimeInfos<ReturnType>();
	}

	virtual void cout(int & trailing, uint opts = DEFAULT);
};

//struct IfBlockT : Block {
//	using Ptr = std::shared_ptr<IfBlockT>;
//	using Block::Block;
//};

struct Statement : InstructionBase {
	using Ptr = std::shared_ptr<Statement>;
	Statement(const Ex & e) : ex(e) {}
	virtual void cout(int & trailing, uint opts = SEMICOLON & NEW_LINE) {
		if ( (opts & IGNORE_DISABLE) || !ex->disabled) {
			std::cout << ( opts & NEW_LINE ? instruction_begin(trailing) : "" ) << ex->str() << instruction_end(opts);
		}

	}
	void explore() {
		ex->explore();
	}
	Ex ex;
};

struct ReturnStatement : Statement {
	using Ptr = std::shared_ptr<ReturnStatement>;
	ReturnStatement(const Ex & e) : Statement(e) {}
	virtual void cout(int & trailing, uint opts = SEMICOLON & NEW_LINE) {
		std::cout << instruction_begin(trailing) << str() << instruction_end(opts);
	}

	const std::string str() const {
		std::string s = "return";
		const std::string ex_str = ex->str();
		if (ex_str != "") {
			s += " " + ex_str;
		}
		return s;
	}

	void explore() {
		ex->explore();
	}
};

InstructionBase::Ptr toInstruction(const Ex & e) {
	auto statement = std::make_shared<Statement>(e);
	return std::dynamic_pointer_cast<InstructionBase>(statement);
}

struct CommentInstruction : InstructionBase {
	CommentInstruction(const std::string & s) : comment(s) {}
	virtual void cout(int & trailing, uint otps = DEFAULT) {
		std::cout << instruction_begin(trailing) << "//" << comment << std::endl;
	}
	std::string comment;
};

template<typename ReturnType>
void ReturnBlock<ReturnType>::cout(int & trailing, uint opts)
{
	Block::cout(trailing, opts);
	if (!std::is_same<ReturnType, void>::value && !ReturnBlockBase::hasReturnStatement) {
		CommentInstruction("need return statement here").cout(trailing, opts);
	}
}

struct ForInstruction : InstructionBase {
	using Ptr = std::shared_ptr<ForInstruction>;
	
	virtual void cout(int & trailing, uint opts) {
		std::cout << instruction_begin(trailing) << "for( ";
		init->cout(trailing, SEMICOLON); 
		std::cout << " ";
		condition->cout(trailing, SEMICOLON);
		std::cout << " ";
		loop->cout(trailing, NOTHING);
		std::cout << " ){\n";
		++trailing;
		body->cout(trailing, opts);
		--trailing;
		std::cout << instruction_begin(trailing) << "}\n";
	}

	Statement::Ptr init, condition, loop;
	Block::Ptr body;
};

struct WhileInstruction : InstructionBase {
	using Ptr = std::shared_ptr<WhileInstruction>;

	WhileInstruction(const Ex & ex, const Block::Ptr & parent ) {
		condition = std::make_shared<Statement>(ex);
		body = std::make_shared<Block>(parent);
	}
	virtual void cout(int & trailing, uint opts) {
		std::cout << instruction_begin(trailing) << "while( ";
		condition->cout(trailing, NOTHING | IGNORE_DISABLE);
		std::cout << " ){\n";
		++trailing;
		body->cout(trailing, opts);
		--trailing;
		std::cout << instruction_begin(trailing) << "}\n";
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

	virtual void cout(int & trailing, uint opts) {
		const int numBodies = (int)bodies.size();
		for (int i = 0; i < numBodies; ++i) {
			if (i == 0) {
				std::cout << instruction_begin(trailing) << "if( ";
				bodies[0].condition->cout(trailing, NOTHING | IGNORE_DISABLE);
				std::cout << " ) {\n";
			} else if(bodies[i].condition) {
				std::cout << "else if( ";
				bodies[i].condition->cout(trailing, NOTHING | IGNORE_DISABLE);
				std::cout << " ) {\n";
			} else {
				std::cout << "else {\n";
			}

			++trailing;
			bodies[i].body->cout(trailing);
			--trailing;
			std::cout << instruction_begin(trailing) << "}" << (i == numBodies - 1 ? "\n" : " ");
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
	static const std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
		return InstructionBase::instruction_begin(trailing) + T::typeStr() + " " + v[v.size() - N] + 
			 ( (s==SEP_AFTER_ALL || ( s==SEP_IN_BETWEEN && N!=1) ) ? separator : "" ) + 
			DisplayDeclaration<s, N - 1, Ts...>::str(v, trailing, separator);
	}
};

template<SeparatorRule s, typename ... T>
struct DisplayDeclaration<s, 0,T...> {
	static const std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts>
std::string memberDeclarations(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
	return DisplayDeclaration<s, sizeof...(Ts), Ts...>::str(v, trailing, separator);
}

template<SeparatorRule s, int N, typename ... Ts>
struct DisplayDeclarationTuple {
	static const std::string str(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) {
		return InstructionBase::instruction_begin(trailing) + 
			std::tuple_element_t<sizeof...(Ts) - N,std::tuple<Ts...> >::typeStr() + " " + 
			std::get<sizeof...(Ts) - N>(v).myName() +
			((s == SEP_AFTER_ALL || (s == SEP_IN_BETWEEN && N != 1)) ? separator : "") +
			DisplayDeclarationTuple<s, N - 1, Ts...>::str(v, trailing, separator);
	}
};
template<SeparatorRule s, typename ... Ts>
struct DisplayDeclarationTuple<s, 0, Ts...> {
	static const std::string str(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts>
std::string memberDeclarationsTuple(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) {
	return DisplayDeclarationTuple<s, sizeof...(Ts), Ts...>::str(v, trailing, separator);
}

template<typename ... Args>
struct StructDeclaration : InstructionBase {
	template<typename ... Strings>
	StructDeclaration(const std::string & _name, const Strings &... _names) : 
		name(_name), member_names{ _names... } {}

	virtual void cout(int & trailing, uint opts) {
		std::cout << instruction_begin(trailing) << "struct " << name << " { " << std::endl;
		++trailing;
		std::cout << memberDeclarations<SEP_AFTER_ALL,Args...>(member_names, trailing, ";\n");
		--trailing;
		std::cout << instruction_begin(trailing) << "}" << std::endl;
	}

	std::vector<std::string> member_names;
	std::string name;
};

template<int N, typename ... Strings>
std::array<std::string,N> fill_args_names(const Strings & ... _argnames) {
	return std::array<std::string, N>{_argnames...};
}

template <typename ReturnType, typename... Args, typename ... Strings>
void init_function_declaration(const std::string & fname, const std::function<void(Args...)>& f, const Strings & ... args_name);

template<typename T> struct FunctionReturnType;
template<typename Lambda> typename FunctionReturnType<decltype(&Lambda::operator())>::type functionFromLambda(const Lambda &func);

template<typename ...Args> struct ArgTypeList {};
template<typename Arg, typename Arg2, typename ...Args> struct ArgTypeList<Arg,Arg2,Args...> {
	using first = ArgTypeList<Arg>;
	using rest = ArgTypeList<Arg2,Args...>;
};

template <typename ...Ts> constexpr bool SameTypeList = false;

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
	using NamedObject<FunBase>::NamedObject;
	static const std::string typeStr() { return "function"; }
};

template<typename ReturnType, typename F_Type>
struct Fun : FunBase {
	//using FunctionType = std::result_of_t< plugType<ReturnType>(F_Type));

	template<typename ... Strings>
	Fun(const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FunBase(_name), f(_f) {
		init_function_declaration<ReturnType>(myName(), functionFromLambda(_f), _argnames...);
	}

	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	ReturnType operator()(R_Args &&  ... args) {
		checkArgsType<ArgTypeList<CleanType<R_Args>...> >(functionFromLambda(f));
		return ReturnType(createFCallExp(myName(), getExp<R_Args>(args)...));
	}

	F_Type f;
};

template<typename ReturnType, typename F_Type, typename ... Strings >
Fun<ReturnType, F_Type> makeFun(const std::string & name, const F_Type & f, const Strings & ...argnames) {
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
	
	FunctionDeclarationArgs(const std::string & _name, const std::tuple<Args...> & _args) : name(_name), args(_args) {	
	}

	std::string name;
	std::tuple<Args...> args;
};

template<typename ReturnT, typename ... Args>
struct FunctionDeclaration : FunctionDeclarationArgs<ReturnT,Args...> {
	using Base = FunctionDeclarationArgs<ReturnT, Args...>;
	using Base::Base;

	virtual void cout(int & trailing, uint opts) {
		std::cout << InstructionBase::instruction_begin(trailing) << ReturnT::typeStr() << " " << Base::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(Base::args, trailing, ", ") << ") \n{" << std::endl;
		++trailing;
		Base::getBody()->cout(trailing);
		--trailing;
		std::cout << InstructionBase::instruction_begin(trailing) << "}" << std::endl;
	}
};

//specialization for ReturnT == void
template<typename ... Args>
struct FunctionDeclaration<void, Args...> : FunctionDeclarationArgs<void,Args...> {
	using Base = FunctionDeclarationArgs<void, Args...>;
	using Base::Base;

	virtual void cout(int & trailing, uint opts) {
		std::cout << InstructionBase::instruction_begin(trailing) << "void " << Base::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(Base::args, trailing, ", ") << ") \n{" << std::endl;
		++trailing;
		Base::getBody()->cout(trailing);
		--trailing;
		std::cout << InstructionBase::instruction_begin(trailing)  << "}" << std::endl;
	}
};

struct ControllerBase {

	void queueEvent(const Ex & e) {
		currentBlock->instructions.push_back(toInstruction(e));
	}

	Block::Ptr currentBlock;
};

struct ForController : virtual ControllerBase {
	enum ForStatus { NONE, INIT, BODY };

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
		for_status = INIT;	
	}

	bool feed_for(const Ex & e) {
		if (for_status == BODY) {
			//queueEvent(e);
			return false;
		} else if (!current_for->init) {
			current_for->init = std::make_shared<Statement>(e);
			//std::cout << "init : " << e->str() << std::endl;
		} else if (!current_for->condition) {
			current_for->condition = std::make_shared<Statement>(e);
			//std::cout << "condition : " << e->str() << std::endl;
		} else if (!current_for->loop) {
			current_for->loop = std::make_shared<Statement>(e);
			current_for->body = std::make_shared<Block>(currentBlock);
			//std::cout << "loop : " << e->str() << std::endl;
			currentBlock = current_for->body;
			for_status = BODY;
		} 
		return true;
	}

	virtual void end_for() {
		for_status = NONE;
		currentBlock = currentBlock->parent;
	}

	ForInstruction::Ptr current_for;
	ForStatus for_status = NONE;
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
		if (current_if) {
			//std::cout << " nested if" << std::endl;
			current_if = std::make_shared<IfInstruction>(current_if);
		} else {
			//std::cout << " non nested if" << std::endl;
			current_if = std::make_shared<IfInstruction>();
		}
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
	
	virtual void begin_for() {
		check_end_if();
		ForController::begin_for();
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
	
		if (for_status != NONE) {
			if (feed_for(e)) {
				return;
			}
		} 
		check_end_if();

		queueEvent(e);
	}
};

struct TShader : MainController {
	using Ptr = std::shared_ptr<TShader>;

	TShader() {
		declarations = std::make_shared<Block>();
		currentBlock = declarations;

		//add defaut main
		//functions.push_back(std::make_shared<Block>(createExp<>()))
	}

	Block::Ptr declarations;
	std::vector<InstructionBase::Ptr> structs; 
	std::vector<InstructionBase::Ptr> functions;
	
	int version;

	void cout() {
		int trailing = 0;
		for (const auto & struc : structs) {
			struc->cout(trailing);
			std::cout << "\n";
		}
		for (const auto & fun : functions) {
			fun->cout(trailing);
			std::cout << "\n";
		}
		declarations->cout(trailing);
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
	void begin_function(const std::string & name, const std::tuple<Args...> & args) {
		auto function_declaration = std::make_shared < FunctionDeclaration<ReturnT,Args...> >(name, args );
		functions.push_back(std::static_pointer_cast<InstructionBase>(function_declaration));
		function_declaration->body->parent = currentBlock;
		currentBlock = std::static_pointer_cast<Block>(function_declaration->body);
	}
	void end_function() {
		currentBlock = std::static_pointer_cast<FunctionDeclarationBase>(functions.back())->getBody()->parent;
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
				currentBlock->push_instruction(std::make_shared<CommentInstruction>("unexpected " + return_statement->str() + "; in non void function "));
			}
		}
	}

	template<typename R_T>
	void add_return_statement(R_T && t) {
		Ex ex = getExp<R_T>(t);
		auto return_statement = std::make_shared<ReturnStatement>(ex);

		if (auto return_block = get_return_block()) {
			if (return_block->getType().isConvertibleTo(getRunTimeInfos< CleanType<R_T>>())) {
				currentBlock->push_instruction(return_statement);
				return_block->hasReturnStatement = true;
			} else {
				currentBlock->push_instruction(std::make_shared<CommentInstruction>("wrong result type in : return " + ex->str()));
			}
		}
	}
};


struct MainListener {
	
	MainListener() {
		shader = std::make_shared<TShader>();
		currentShader = shader.get();
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

	void end_for() {
		if (currentShader) {
			currentShader->end_for();
		}
	}

	/////////////////////////////////////////////////

	template<typename R_B, typename = std::enable_if_t< EqualType<CleanType<R_B>, Bool> > >
	void begin_if(R_B && b) {
		if (currentShader) {
			currentShader->begin_if(getExp<R_B>(b));
		}
	}
	void begin_else() {
		if (currentShader) {
			currentShader->begin_else();
		}
	}
	template<typename R_B, typename = std::enable_if_t< EqualType<CleanType<R_B>, Bool> > >
	void begin_else_if(R_B && b) {
		if (currentShader) {
			currentShader->begin_else_if(getExp<R_B>(b));
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
	void begin_function(const std::string & name, const std::tuple<Args...> & args) {
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

	void cout() {
		if (currentShader) {
			currentShader->cout();
		}
	}
	void explore() {
		if (currentShader) {
			std::cout << "############## explore ############" << std::endl;
			currentShader->explore();
			std::cout << "###################################" << std::endl;
		}
	}

	bool & active() { return isListening; }

	TShader::Ptr shader;
	TShader * currentShader;
	bool isListening = true;
	static MainListener overmind;
};

MainListener MainListener::overmind = MainListener();

MainListener & listen() { return MainListener::overmind; }

//specialization of Fun when ReturnType == void
template<typename F_Type>
struct Fun<void, F_Type> : FunBase {
	template<typename ... Strings>
	Fun(const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FunBase(_name), f(_f) {
		init_function_declaration<void>(myName(), functionFromLambda(_f), _argnames...);
	}
	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	void operator()(R_Args &&  ... args) {
		checkArgsType<ArgTypeList<CleanType<R_Args>...> >(functionFromLambda(f));
		listen().addEvent(createFCallExp(myName(), getExp<R_Args>(args)...));
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

template<typename T, CtorTypeDisplay tRule = DISPLAY, ParenthesisRule pRule = USE_PARENTHESIS, CtorStatus status = INITIALISATION, typename ... Args>
Ex createInit(const stringPtr & name, const Args &... args)
{
	auto ctor = std::make_shared<Constructor<T, sizeof...(args), tRule, pRule> >(name, status, std::vector<Ex> { args... });
	Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
	listen().addEvent(expr);
	return expr;
}


template<typename T, typename ... Args>
Ex createDeclaration(const stringPtr & name, const Args &... args)
{
	return createInit<T, DISPLAY, USE_PARENTHESIS, DECLARATION, Args...>(name, args...);
}

template<typename T, uint N>
Ex createArrayDeclaration(const stringPtr & name) {
	auto ctor = std::make_shared<Constructor<T>>(name,DECLARATION);
	Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
	listen().addEvent(expr);
	return expr;
}

template<typename ... Args> 
Ex createFCallExp(const std::string & f_name, const Args & ... args) {
	return createExp<FunctionCall<sizeof...(Args)>>(f_name, args...);
}

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
			std::tuple<Arg>( Arg( names[S - sizeof...(Args) - 1], NOT_TRACKED ) ),
			getTuple<S, Args...>(names)
		);
	}
};


template <typename ReturnType, typename... Args, typename ... Strings>
void init_function_declaration(const std::string & fname, const std::function<void(Args...)>& f, const Strings & ... args_name)
{
	static_assert(sizeof...(Strings) <= sizeof...(Args), "too many arguments string for function declaration");
	
	std::array<std::string, sizeof...(Args)> args_names_array{args_name...};
	
	std::tuple<Args...> args = getTuple<sizeof...(Args),Args...>(args_names_array);

	listen().begin_function<ReturnType, Args...>(fname, args);
	
	call_from_tuple(f, args);

	//std::cout << r.myName() << std::endl;
	//std::cout << r.exp->args[0]->str() << std::endl;
	//std::cout << r.exp->args.size() << std::endl;
	//r.exp->args[0]->op->explore();
	//areNotInit(r);
	
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


#define GL_FOR(...) listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
listen().begin_for(); __VA_ARGS__;  \
for(ForController::EndFor csl_dummy_for; csl_dummy_for; )



IfController::BeginIf::~BeginIf() {
	listen().end_if_sub_block();
}
IfController::BeginElse::~BeginElse() {
	listen().end_if();
}

#define GL_IF(condition) listen().check_begin_if(); listen().begin_if(condition); if(IfController::BeginIf csl_begin_if = {})

#define GL_ELSE else {} listen().begin_else(); if(IfController::BeginElse csl_begin_else = {}) {} else 

#define GL_ELSE_IF(condition) else if(true){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(IfController::BeginIf csl_begin_else_if = {})


WhileController::BeginWhile::~BeginWhile() {
	listen().end_while();
}

#define GL_WHILE(condition) listen().begin_while(condition); for(WhileController::BeginWhile csl_begin_while = {}; csl_begin_while; )

//////////////////////////////////////

NamedObjectBase::~NamedObjectBase() {
	//if (!isUsed) {
	//	if (exp) {
	//		auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op);
	//		std::cout << "not used " << exp->str() << " " << myName() << " " << ctor->status << std::endl;
	//	}
	//}

	//should check inside scope
	if (!isUsed && exp) {
		auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp);
		//std::cout << "not used " << exp->str() << " " << myName() << " " << ctor->status << std::endl;
		if (ctor) {
			ctor->status = TEMP;
		}
	}
}





