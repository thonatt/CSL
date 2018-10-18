#pragma once

#include <string>
#include <map>
#include <memory>
#include <array>

#include "HelperClasses.h"

struct MainListener;
MainListener & listen();

using stringPtr = std::shared_ptr<std::string>;
using uint = unsigned int;

struct Exp;
using Ex = std::shared_ptr<Exp>;

enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN, BEHIND };
enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };
enum CtorTypeDisplay { DISPLAY, HIDE};
enum CtorStatus { DECLARATION, INITIALISATION, TEMP };
enum StatementOptions { SEMICOLON = 1 << 0, COMMA = 1 << 1, NOTHING = 1 << 2, NEW_LINE = 1 << 3, IGNORE_DISABLE = 1 << 4, DEFAULT = SEMICOLON | NEW_LINE };

struct OpBase {
	OpBase(OperatorDisplayRule rule = NONE, ParenthesisRule useParenthesis = USE_PARENTHESIS, bool args = true)
		: displayRule(rule), parRule(useParenthesis), hasArgs(args) {}

	OperatorDisplayRule displayRule = IN_FRONT;
	ParenthesisRule parRule;
	bool disabled = false;
	mutable bool hasArgs = true;

	virtual std::string str() const { return "dummyOpBase"; }
	virtual void explore() {
		std::cout << ", dRule : " << displayRule << ", pRule : " << parRule << ", disabled : " << disabled << ", hasArgs : " << hasArgs << std::endl;
	}
};

using OpB = std::shared_ptr<OpBase>;

struct Exp {
	Exp(const OpB & _op, const std::vector<Ex> & _args = {}) : op(_op), args(_args) {
		//std::cout << "Exp ctor " << n << std::endl;
	}
	std::string str() const {
		//std::cout << " str " << std::flush;
		//std::cout << op->str() << std::endl;

		std::string separator = op->displayRule == IN_BETWEEN ? " " + op->str() + " " : ", ";
		bool parenthesis = op->parRule == USE_PARENTHESIS;

		std::string out; // = (op->displayRule == IN_BETWEEN || op->displayRule == BEHIND) ? "" : op->str();

		if (op->displayRule == IN_FRONT) {
			out += op->str();
		}

		if (op->hasArgs) {
			if (parenthesis) {
				out += "(";
			}
			const int size = (int)args.size();
			for (int i = 0; i < size; ++i) {
				out += args[i]->str() + (i == size - 1 ? "" : separator);
			}
			if (parenthesis) {
				out += ")";
			}
		}

		if (op->displayRule == BEHIND) {
			out += op->str();
		}



		return out;
	}

	void explore(int k = 1) {
		std::cout << " count " << op.use_count() << ", ";
		op->explore();
		for (const auto & a : args) {
			for (int i = 0; i < k; ++i) {
				std::cout << "   ";
			}
			std::cout << " count " << a.use_count() << ", ";
			a->explore(k + 1);
		}
	}
	OpB op;
	std::vector<Ex> args;

};
struct Op : OpBase {};

template<typename T>
struct Litteral : OpBase {
	Litteral(const T & _i) : OpBase(IN_FRONT, NO_PARENTHESIS), i(_i) {}
	virtual std::string str() const { return std::to_string(i); }
	virtual void explore() { std::cout << "Litteral T " << i << " "; OpBase::explore(); }
	T i;
};

template<> struct Litteral<bool> : OpBase {
	Litteral(const bool & _b) : OpBase(IN_FRONT, NO_PARENTHESIS), b(_b) {}
	virtual std::string str() const { return b ? "true" : "false"; }
	virtual void explore() { std::cout << "Litteral Bool " << b << " "; OpBase::explore(); }
	bool b;
};

struct CtorBase : OpBase {
	CtorBase(const stringPtr & s, bool args, CtorStatus _status = INITIALISATION) :
		OpBase(args ? IN_FRONT : NONE, args ? USE_PARENTHESIS : NO_PARENTHESIS, args), name(s),  n(counter++), status(_status) {}

	stringPtr name;
	int n;
	//bool isInit = true;
	mutable CtorStatus status;

	virtual void explore() { std::cout << "CtorBase " << *name << ", id " /* << n << ", init? " << isInit */; OpBase::explore(); }

	static int counter;
};
int CtorBase::counter = 0;

struct DtorBase : OpBase {
	DtorBase() { disabled = true;  }
	virtual std::string str() const { return "dtorOp"; }
	virtual void explore() { std::cout << "DTOR "; OpBase::explore(); }
};

using CtorBasePtr = std::shared_ptr<CtorBase>;

template<typename T> struct TypeStrT {
	static const std::string str() { return T::typeStr(); }
};

template<typename T> std::string  getTypeStrTest() {
	return TypeStrT<T>::str();
}

template<typename T, OperatorDisplayRule dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS, CtorTypeDisplay tRule = DISPLAY>
struct Ctor : CtorBase {
	Ctor(const stringPtr & s, bool hasArgs, CtorStatus _status)
		: CtorBase(s, hasArgs, _status)
	{
		//std::cout << " ctor " << *s << " " << _status << std::endl;
		displayRule = dRule;
		parRule = pRule;
	}

	//virtual std::string str() const { 
	//	if (firstStr) {
	//		firstStr = false;
	//		if (isInit) {
	//			if (hasArgs) {
	//				return getTypeStrTest<T>() + " " + name + " = " + (tRule == DISPLAY ? getTypeStrTest<T>() : std::string("") );
	//			} else {
	//				return getTypeStrTest<T>() + " " + name;
	//			}
	//		} else {
	//			return (tRule == DISPLAY ? getTypeStrTest<T>() : std::string(""));
	//		}
	//	} else {
	//		hasArgs = false;
	//		return name;
	//	}	
	//}

	virtual std::string str() const {
		if (status == INITIALISATION) {
			return getTypeStrTest<T>() + " " + *name + " = " + (tRule == DISPLAY ? getTypeStrTest<T>() : std::string(""));
		} else if (status == DECLARATION) {
			//std::cout << " decalration " << getTypeStrTest<T>() << "  / "  << *name << std::endl;
			return getTypeStrTest<T>() + " " + *name;
		} else {
			return (tRule == DISPLAY ? getTypeStrTest<T>() : std::string(""));
		}
	}
};

struct Alias : OpBase {
	Alias(std::shared_ptr<std::string> s) : OpBase(IN_FRONT,NO_PARENTHESIS), namePtr(s) {}
	virtual std::string str() const { return *namePtr; }
	virtual void explore() { std::cout << "Alias " << str() << " "; OpBase::explore(); }
	std::shared_ptr<std::string> namePtr;
};

//struct Assignment : OpBase {
//	Assignment() : {}
//	virtual std::string str(int m) const { return  "assign"; }
//};

template<char c, ParenthesisRule p = USE_PARENTHESIS>
struct SingleCharBinaryOp : OpBase {
	SingleCharBinaryOp() : OpBase(IN_BETWEEN, p) {}
	virtual std::string str() const { return std::string(1, c); }
	virtual void explore() { std::cout << "OP " << c << " "; OpBase::explore(); }
};

template<OperatorDisplayRule dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS>
struct FunctionOp : OpBase {
	FunctionOp(const std::string & s) : OpBase(dRule,pRule), name(s) {}
	virtual std::string str() const { return name; }
	virtual void explore() { std::cout << "OP " << name << " "; OpBase::explore(); }
	const std::string name;
};

void isNotInit(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(expr->op)) {
		//ctor->isInit = false;
		ctor->disabled = true;
		ctor->status = TEMP;
	}
}

void isTest(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(expr->op)) {
		//ctor->isInit = false; 
		//ctor->disabled = true;
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

template<typename T> using CleanType = std::remove_const_t<std::remove_reference_t<T>>;

template<bool temp> struct CheckForTempT;
template<> struct CheckForTempT<true> {	
	template<typename T>
	static void check(const T &t) { 
		areNotInit(t);
		//std::cout << "temp " << std::endl; 
	}
};
template<> struct CheckForTempT<false> {
	template<typename T>
	static void check(const T &t) { 
		//std::cout << "not temp " << std::endl; 
	}
};

template<typename ... Ts> void checkForTemp(const Ts &... ts);
template<typename T, typename ... Ts> void checkForTemp(const T &t, const Ts &... ts) {
	//CheckForTempT<!std::is_lvalue_reference<T>::value>::check(t);
	//checkForTemp<Ts...>(ts...);
}
template<> void checkForTemp<>() { }

struct InitManager {
	void handle(const Ex & ex) {
		if (auto op = std::dynamic_pointer_cast<CtorBase>(ex->op)) {
			ctor(op);
		} else if (auto op = std::dynamic_pointer_cast<DtorBase>(ex->op)) {
			dtor(std::dynamic_pointer_cast<CtorBase>(ex->args[0]->op));
		}
	}

	void ctor(const CtorBasePtr & t);
	void dtor(const CtorBasePtr & t);

	std::map<int, CtorBasePtr> decls;
	int currentUp = -1;

	static InitManager initManager;
};
InitManager InitManager::initManager = InitManager();

struct Manager {
	void add(const Ex &t);

	void cout();
	
	std::vector<Ex> exps;
	static Manager man;
};
Manager Manager::man = Manager();


template<typename Operator, typename ... Args>
Ex createExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	return std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...});
}

template<typename T, bool temp = !std::is_lvalue_reference<T>::value> Ex getExp(const T & t) {
	//std::cout << t.exp.use_count() << std::endl;
	t.isUsed = true;
	if(temp) {
		areNotInit(t);
		//std::cout << " get exp temp " << t.myName() << std::endl;
		return t.exp;
	}/* else if (t.exp.use_count() == 1 && !t.parent) {
		std::cout << " first use " << t.myName() << std::endl;
		return t.exp;
	}*/ else {
		//std::cout << " ref use " << t.myName() << std::endl;
		return createExp(std::make_shared<Alias>(t.myNamePtr()));
	}
}

template<typename T, CtorTypeDisplay tRule = DISPLAY, OperatorDisplayRule dRule = IN_FRONT, 
	ParenthesisRule pRule = USE_PARENTHESIS, CtorStatus status = INITIALISATION, typename ... Args>
Ex createInit(const stringPtr & name, const Args &... args);


template<typename Operator, typename ... Args> void addExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	Manager::man.add(std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...}));
}

template<> Ex getExp<bool>(const bool & b) {
	return createExp(std::make_shared<Litteral<bool>>(b));
}

template<> Ex getExp<int>(const int & i) {
	return createExp(std::make_shared<Litteral<int>>(i));
}

template<> Ex getExp<double>(const double & d) {
	return createExp(std::make_shared<Litteral<double>>(d));
}

struct T;

struct Tinit {
	Tinit(const T & u, const std::string & s);

	std::string name;
	Ex exp;
};


template<typename ...Us> struct NotChars;
template<typename U> struct NotChars<U> {
	static const bool value = true;
};
template<typename U, typename ...Us> struct NotChars<U, Us...> {
	static const bool value = NotChars<U>::value && NotChars<Us...>::value;
};
template <std::size_t N>  struct NotChars<const char(&)[N]> {
	static const bool value = false;
};

template<typename ...Us> struct NotInits;
template<typename U> struct NotInits<U> {
	static const bool value = true;
};
template<typename U, typename ...Us> struct NotInits<U, Us...> {
	static const bool value = NotInits<U>::value && NotInits<Us...>::value;
};
template <>  struct NotInits<Tinit> {
	static const bool value = false;
};

//Tinit operator<<(const T & t, const char * s) {
//	return Tinit(t, s);
//}

void InitManager::ctor(const CtorBasePtr & tor) {
	std::cout << " manager ctor " << tor->n << std::endl;
	currentUp = tor->n;
	decls[currentUp] = tor;
}


void InitManager::dtor(const CtorBasePtr & tor) {
	//std::cout << " manager dtor " << tor->n << std::endl;
	//std::cout << " dtor cnt " << tor.use_count() << std::endl;

	if (tor->n < currentUp) {
		//std::cout << "temp" << std::endl;
		if (decls.count(tor->n) == 0) {
			std::cout << "~~~" << std::endl;
		}
		//decls[tor->n]->isInit = false;
		decls[tor->n]->disabled = true;
	} else {
		do {
			--currentUp;
		} while (currentUp >= 0 && (decls.count(currentUp) == 0  /*|| !decls[currentUp]->isInit */ ) );
	}
}

void Manager::add(const Ex & e) {
	//std::cout << " manager add " << std::endl;
	exps.push_back(e);
}

void Manager::cout() {

	for (const auto & ex : exps) {
		if (ex->op->disabled) {
			continue;
		}
		std::cout << ex->str() << ";" << std::endl;
	}
}


struct InstructionBase {
	using Ptr = std::shared_ptr<InstructionBase>;

	static std::string instruction_begin(int trailing) {
		std::string out;
		for (int t = 0; t < trailing; ++t) {
			out += "   ";
		}
		return out;
	}
	static std::string instruction_end(uint opts) {
		return std::string(opts & SEMICOLON ? ";" : opts & COMMA ? "," : "") + (opts & NEW_LINE ? "\n" : "");
	}

	virtual void cout(int & trailing, uint otps = DEFAULT) {}
	virtual void explore() {}
};

struct Block {
	using Ptr = std::shared_ptr<Block>;
	Block(const Block::Ptr & _parent = {}) : parent(_parent) {}

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

struct ReturnBlock : Block {
	ReturnBlock(const Block::Ptr & _parent = {} ) : Block(_parent) {}

	bool hasReturnStatement = false;
};

//struct IfBlockT : Block {
//	using Ptr = std::shared_ptr<IfBlockT>;
//	using Block::Block;
//};

struct Statement : InstructionBase {
	using Ptr = std::shared_ptr<Statement>;
	Statement(const Ex & e) : ex(e) {}
	virtual void cout(int & trailing, uint opts = SEMICOLON & NEW_LINE) {
		if ( (opts & IGNORE_DISABLE) || !ex->op->disabled) {
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
		std::cout << instruction_begin(trailing) << "return " << ex->str() << instruction_end(opts);
	}
	void explore() {
		ex->explore();
	}
};

InstructionBase::Ptr toInstruction(const Ex & e) {
	auto statement = std::make_shared<Statement>(e);
	return std::dynamic_pointer_cast<InstructionBase>(statement);
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

template<SeparatorRule s, int N, typename T, typename ... Ts> struct DisplayDeclaration<s, N, T, Ts...> {
	static const std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
		return InstructionBase::instruction_begin(trailing) + T::typeStr() + " " + v[v.size() - N] + 
			 ( (s==SEP_AFTER_ALL || ( s==SEP_IN_BETWEEN && N!=1) ) ? separator : "" ) + 
			DisplayDeclaration<s, N - 1, Ts...>::str(v, trailing, separator);
	}
};

template<SeparatorRule s, typename ... T> struct DisplayDeclaration<s, 0,T...> {
	static const std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts> std::string memberDeclarations(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
	return DisplayDeclaration<s, sizeof...(Ts), Ts...>::str(v, trailing, separator);
}

template<SeparatorRule s, int N, typename ... Ts> struct DisplayDeclarationTuple {
	static const std::string str(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) {
		return InstructionBase::instruction_begin(trailing) + std::tuple_element_t<sizeof...(Ts) - N,std::tuple<Ts...> >::typeStr() + " " + std::get<sizeof...(Ts) - N>(v).myName() +
			((s == SEP_AFTER_ALL || (s == SEP_IN_BETWEEN && N != 1)) ? separator : "") +
			DisplayDeclarationTuple<s, N - 1, Ts...>::str(v, trailing, separator);
	}
};
template<SeparatorRule s, typename ... Ts> struct DisplayDeclarationTuple<s, 0, Ts...> {
	static const std::string str(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) { return ""; }
};

template<SeparatorRule s, typename ... Ts> std::string memberDeclarationsTuple(const std::tuple<Ts...> & v, int & trailing, const std::string & separator) {
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
constexpr bool SameTypeList<ArgTypeList<Input>, ArgTypeList<Target> > = ConvertibleTo<Input, Target>; //EqualMat<Arg, ArgM>;

template <typename LA, typename LB>
constexpr bool SameTypeList<LA, LB> =
SameTypeList<typename LA::first,typename LB::first> && SameTypeList<typename LA::rest, typename LB::rest>;

template <typename List, typename ... Args>
void checkArgsType(const std::function<void(Args...)>& f){
	static_assert(SameTypeList < List, ArgTypeList<Args...> >, "arg types do not match function signature, or no implicit conversion available");
}

//template<typename Lambda>

template<typename F_Type>
struct FunBase_T {
	FunBase_T(const std::string & _name, const F_Type  & _f) : name(_name), f(_f) {}

	F_Type f;
	std::string name;
};

template<typename ReturnType, typename F_Type>
struct Fun_T : FunBase_T<F_Type> {
	//using FunctionType = std::result_of_t< plugType<ReturnType>(F_Type));

	template<typename ... Strings>
	Fun_T(const std::string & _name, const F_Type  & _f, const Strings & ... _argnames ) : FunBase_T<F_Type>(_name,_f) {
		init_function_declaration<ReturnType>(_name, functionFromLambda(_f), _argnames...);
	}

	//more type checking
	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	ReturnType operator()(R_Args &&  ... args) {
		//, std::enable_if_t< std::is_same_v<void, std::result_of_t<F_Type(CleanType<R_Args>...)> > >
		//using RT = typename std::result_of_t<F_Type(R_Args...)>;
		//std::cout << typeid(RT).name() << std::endl;
		
		checkArgsType<ArgTypeList<R_Args...> >(functionFromLambda(FunBase_T<F_Type>::f));
		//checkForTemp<R_Args...>(args...);

		return ReturnType(createExp(std::make_shared<FunctionOp<>>(FunBase_T<F_Type>::name), getExp<R_Args>(args)... ));
	}
};

template<typename ReturnType,typename F_Type, typename ... Strings >
Fun_T<ReturnType,F_Type> makeFunT(const std::string & name, const F_Type & f, const Strings & ...argnames) {
	return Fun_T<ReturnType,F_Type>(name, f, argnames...);
}

struct FunctionDeclarationBase : InstructionBase {
	FunctionDeclarationBase() {
		body = std::make_shared<ReturnBlock>();
	}
	ReturnBlock::Ptr body;
};

template<typename ... Args>
struct FunctionDeclarationArgs : FunctionDeclarationBase {
	
	FunctionDeclarationArgs(const std::string & _name, const std::tuple<Args...> & _args) : name(_name), args(_args) {	
	}

	std::string name;
	std::tuple<Args...> args;
};

template<typename ReturnT, typename ... Args>
struct FunctionDeclaration : FunctionDeclarationArgs<Args...> {

	using FunctionDeclarationArgs<Args...>::FunctionDeclarationArgs;

	virtual void cout(int & trailing, uint opts) {
		std::cout << InstructionBase::instruction_begin(trailing) << ReturnT::typeStr() << " " << FunctionDeclarationArgs<Args...>::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(FunctionDeclarationArgs<Args...>::args, trailing, ", ") << ") \n{" << std::endl;
		++trailing;
		FunctionDeclarationBase::body->cout(trailing);
		--trailing;
		std::cout << InstructionBase::instruction_begin(trailing) << "}" << std::endl;
	}
};

//specialization for ReturnT == void
template<typename ... Args>
struct FunctionDeclaration<void, Args...> : FunctionDeclarationArgs<Args...> {

	using FunctionDeclarationArgs<Args...>::FunctionDeclarationArgs;

	virtual void cout(int & trailing, uint opts) {
		std::cout << InstructionBase::instruction_begin(trailing) << "void " << FunctionDeclarationArgs<Args...>::name << "(" <<
			memberDeclarationsTuple<SEP_IN_BETWEEN, Args...>(FunctionDeclarationArgs<Args...>::args, trailing, ", ") << ") \n{" << std::endl;
		++trailing;
		FunctionDeclarationBase::body->cout(trailing);
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
		currentBlock->instructions.push_back(std::static_pointer_cast<InstructionBase>(current_for));
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
			std::cout << " nested if" << std::endl;
			current_if = std::make_shared<IfInstruction>(current_if);
		} else {
			std::cout << " non nested if" << std::endl;
			current_if = std::make_shared<IfInstruction>();
		}
		current_if->bodies.push_back({ std::make_shared<Block>(currentBlock), std::make_shared<Statement>(ex) });
		currentBlock->instructions.push_back(std::static_pointer_cast<InstructionBase>(current_if));
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
		std::cout << " end if " << std::endl;
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
			std::cout << " BeginWhile operator bool() const " << first << std::endl;
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
		std::cout << " begin while " << std::endl;
		auto while_instruction = std::make_shared<WhileInstruction>(ex, currentBlock);
		currentBlock->instructions.push_back(std::static_pointer_cast<InstructionBase>(while_instruction));
		currentBlock = while_instruction->body;
	}

	virtual void end_while() {
		std::cout << " end while " << std::endl;
		currentBlock = currentBlock->parent;
	}
};

struct MainController : virtual ForController, virtual WhileController, virtual IfController {
	using Ptr = std::shared_ptr<MainController>;
	InitManager init_manager;

	virtual void end_for() {
		check_end_if();
		ForController::end_for();
	}

	virtual void end_while() {
		check_end_if();
		WhileController::end_while();
	}

	void handleEvent(const Ex & e) {
		//init_manager.handle(e);
		
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
		currentBlock = std::static_pointer_cast<FunctionDeclarationBase>(functions.back())->body->parent;
	}
	void add_return_statement(const Ex & ex) {
		Block::Ptr cBlock = currentBlock;
		while (true) {
			if (auto testblock = std::dynamic_pointer_cast<ReturnBlock>(cBlock)) {
				break;
			} else if (cBlock->parent) {
				cBlock = cBlock->parent;
			} else {
				return;
			}
		}
		auto return_block = std::dynamic_pointer_cast<ReturnBlock>(cBlock);
		if (!return_block->hasReturnStatement) {
			//std::cout << "return statement " << ex->str() << std::endl;
			currentBlock->instructions.push_back(std::static_pointer_cast<InstructionBase>(std::make_shared<ReturnStatement>(ex)));
			return_block->hasReturnStatement = true;
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

	void add_return_statement(const Ex & ex) {
		if (currentShader) {
			currentShader->add_return_statement(ex);
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

	//to be changed to EqualMat<CleanType<R_B>, BoolT> when moved outside of Algebra.h
	template<typename R_B, typename = std::enable_if_t< std::is_same<CleanType<R_B>, BoolT>::value> >
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
	template<typename R_B, typename = std::enable_if_t< std::is_same<CleanType<R_B>, BoolT>::value> >
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

	template<typename R_B, typename = std::enable_if_t< std::is_same<CleanType<R_B>, BoolT>::value> >
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

//specialization of Fun_T when ReturnType == void
template<typename F_Type>
struct Fun_T<void, F_Type> : FunBase_T<F_Type> {
	template<typename ... Strings>
	Fun_T(const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FunBase_T<F_Type>(_name, _f) {
		init_function_declaration<void>(_name, functionFromLambda(_f), _argnames...);
	}
	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	void operator()(R_Args &&  ... args) {
		checkArgsType<ArgTypeList<R_Args...> >(functionFromLambda(FunBase_T<F_Type>::f));
		listen().addEvent(createExp(std::make_shared<FunctionOp<>>(FunBase_T<F_Type>::name), getExp<R_Args>(args)...));
	}
};

template<typename T, CtorTypeDisplay tRule, OperatorDisplayRule  dRule, ParenthesisRule pRule, CtorStatus status, typename ... Args>
Ex createInit(const stringPtr & name, const Args &... args)
{
	auto ctor = std::make_shared<Ctor<T, dRule, pRule, tRule>>(name, (sizeof...(args) != 0), status);
	Ex expr = std::make_shared<Exp>(std::static_pointer_cast<OpBase>(ctor), std::vector<Ex>{args...});
	listen().addEvent(expr);
	return expr;
}

template<typename T, typename ... Args>
Ex createDeclaration(const stringPtr & name, const Args &... args)
{
	return createInit<T, DISPLAY, IN_FRONT, USE_PARENTHESIS, DECLARATION, Args...>(name, args...);
}

enum class Matrix_Track { UNTRACKED };


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
			std::tuple<Arg>({ Matrix_Track::UNTRACKED, names[S - sizeof...(Args) - 1] }),
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

template<typename T> struct ReturnStT<T> {
	static void return_statement(T && t) {
		//checkForTemp<T>(t);
		listen().add_return_statement(getExp<T>(t));
	}
};

template<> struct ReturnStT<> {
	static void return_statement() {
		std::cout << "return void" << std::endl;
	}
};

template<typename ...Ts>
void return_statement(Ts && ... ts) {
	ReturnStT<Ts...>::return_statement(std::forward<Ts>(ts)...);
}

#define GL_RETURN_T(...) return_statement( __VA_ARGS__ )

ForController::EndFor::~EndFor() {
	listen().end_for();
}


#define GL_FOR_T(...) listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
listen().begin_for(); __VA_ARGS__;  \
for(ForController::EndFor csl_dummy_for; csl_dummy_for; )



IfController::BeginIf::~BeginIf() {
	listen().end_if_sub_block();
}
IfController::BeginElse::~BeginElse() {
	listen().end_if();
}

#define GL_IF_T(condition) listen().check_begin_if(); listen().begin_if(condition); if(IfController::BeginIf csl_begin_if = {})

#define GL_ELSE_T else {} listen().begin_else(); if(IfController::BeginElse csl_begin_else = {}) {} else 

#define GL_ELSE_IF_T(condition) else if(true){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(IfController::BeginIf csl_begin_else_if = {})


WhileController::BeginWhile::~BeginWhile() {
	listen().end_while();
}

#define GL_WHILE_T(condition) listen().begin_while(condition); for(WhileController::BeginWhile csl_begin_while = {}; csl_begin_while; )

//////////////////////////////////////


class NamedObjectBaseT {
public:
	NamedObjectBaseT(const std::string & _name = "", NamedObjectBaseT * _parent = nullptr, bool _isUsed = false)
		: parent(_parent), isUsed(_isUsed) {
		namePtr = std::make_shared<std::string>(_name);
		//std::cout << " end check" << std::endl;
	}
	
	~NamedObjectBaseT() {
		//if (!isUsed) {
		//	if (exp) {
		//		auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op);
		//		std::cout << "not used " << exp->str() << " " << myName() << " " << ctor->status << std::endl;
		//	}
		//}

		//should check inside scope
		if (!isUsed && exp) {
			auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op);
			//std::cout << "not used " << exp->str() << " " << myName() << " " << ctor->status << std::endl;
			ctor->status = TEMP;
		}
	}

	static const std::string typeStr() { return "dummyT"; }
	std::shared_ptr<std::string> namePtr;
	mutable bool isUsed = false;
	NamedObjectBaseT * parent = nullptr;
public:
	Ex exp;

public:
	const stringPtr myNamePtr() const { return (parent ? std::make_shared<std::string>(parent->myName() + "." + *namePtr) : namePtr); }
	const std::string myName() const { return *myNamePtr(); }
};

//template<> Ex getExp(const NamedObjectBaseT & obj) { return obj.exp; }


template<typename T>
class NamedObjectT : public NamedObjectBaseT {
public:

	static const std::string typeStr() { return "dummyNameObjT"; }

protected:
	NamedObjectT(const std::string & _name = "", NamedObjectBaseT * _parent = nullptr, bool _isUsed = false) : NamedObjectBaseT(_name, _parent, _isUsed) {
		if (_name == "") {
			namePtr = std::make_shared<std::string>(getTypeStrTest<T>() + "_" + std::to_string(counter));
			++counter;
		}
	}

	static int counter;

public:

};
template<typename T> int NamedObjectT<T>::counter = 0;

