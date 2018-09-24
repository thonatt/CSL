#pragma once

#include <string>
#include <map>
#include <memory>

struct MainListener;
MainListener & listen();


struct Exp;
using Ex = std::shared_ptr<Exp>;

enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN, BEHIND };
enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };
enum CtorTypeDisplay { DISPLAY, HIDE};

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
	CtorBase(const std::string & s, bool args) : 
		OpBase(args ? IN_FRONT : NONE, args ? USE_PARENTHESIS : NO_PARENTHESIS, args), name(s),  n(counter++) {}

	std::string name;
	int n;
	bool isInit = true;
	mutable bool firstStr = true;

	virtual void explore() { std::cout << "CtorBase " << name << ", id " << n << ", init? " << isInit << ", firstStr? "<< firstStr; OpBase::explore(); }

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
	static const std::string str() { return "dummyT"; }
};

template<typename T> std::string  getTypeStrTest() {
	return TypeStrT<T>::str();
}

template<typename T, OperatorDisplayRule dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS, CtorTypeDisplay tRule = DISPLAY>
struct Ctor : CtorBase {
	Ctor(const std::string & s, bool hasArgs)
		: CtorBase(s, hasArgs)
	{
		displayRule = dRule;
		parRule = pRule;
	}

	virtual std::string str() const { 
		if (firstStr) {
			firstStr = false;
			if (isInit) {
				if (hasArgs) {
					return getTypeStrTest<T>() + " " + name + " = " + (tRule == DISPLAY ? getTypeStrTest<T>() : std::string("") );
				} else {
					return getTypeStrTest<T>() + " " + name;
				}
			} else {
				return (tRule == DISPLAY ? getTypeStrTest<T>() : std::string(""));
			}
		} else {
			hasArgs = false;
			return name;
		}
		
	}
};

struct Alias : OpBase {
	Alias(const std::string & s) : OpBase(IN_FRONT,NO_PARENTHESIS), name(s) {}
	virtual std::string str() const { return name; }
	virtual void explore() { std::cout << "Alias " << name << " "; OpBase::explore(); }
	std::string name;
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
			a->explore(k+1);
		}
	}
	OpB op;
	std::vector<Ex> args;
};

void isNotInit(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(expr->op)) {
		ctor->isInit = false;
		ctor->disabled = true;
	}
}

void isTest(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(expr->op)) {
		ctor->isInit = false; 
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
	CheckForTempT<!std::is_lvalue_reference<T>::value>::check(t);
	checkForTemp<Ts...>(ts...);
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

template<typename T> Ex getExp(const T & t); // { return Ex(); }

template<typename Operator, typename ... Args>
Ex createExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	return std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...});
}

template<typename T, CtorTypeDisplay tRule = DISPLAY, OperatorDisplayRule dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS, typename ... Args>
Ex createInit(const std::string & name, const Args &... args);


template<typename Operator, typename ... Args> void addExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	Manager::man.add(std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...}));
}

template<> Ex getExp(const bool & b) {
	return createExp(std::make_shared<Litteral<bool>>(b));
}


template<> Ex getExp(const int & i) {
	return createExp(std::make_shared<Litteral<int>>(i));
}

template<> Ex getExp(const double & d) {
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
	//std::cout << " manager ctor " << tor->n << std::endl;
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
		decls[tor->n]->isInit = false;
		decls[tor->n]->disabled = true;
	} else {
		do {
			--currentUp;
		} while (currentUp >= 0 && (decls.count(currentUp) == 0 || !decls[currentUp]->isInit));
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
	virtual void cout() {}
	virtual void explore() {}
};

struct Block {
	using Ptr = std::shared_ptr<Block>;
	Block(const Block::Ptr _parent = {}) : parent(_parent) {}

	void cout() {
		for (const auto & inst : instructions) {
			inst->cout();
		}
	}
	void explore() {
		for (const auto & inst : instructions) {
			inst->explore();
		}
	}
	std::vector<InstructionBase::Ptr> instructions;
	Block::Ptr parent;
};

struct Statement : InstructionBase {
	using Ptr = std::shared_ptr<Statement>;
	Statement(const Ex & e) : ex(e) {}
	virtual void cout() {
		if (!ex->op->disabled) {
			std::cout << ex->str() << ";" << std::endl;
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

struct ForInstruction : InstructionBase {
	using Ptr = std::shared_ptr<ForInstruction>;

	Statement::Ptr init, condition, loop;
	Block::Ptr body;
};

struct WhileInstruction : InstructionBase {
	Block::Ptr body;
	Statement::Ptr condition;
};

struct IfInstruction : InstructionBase {
	std::vector<Block::Ptr> bodys;
	std::vector<Statement::Ptr> conditions;
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

	void feed_for(const Ex & e) {
		if (for_status == BODY) {
			return;
		} else if (!current_for->init) {
			current_for->init = std::make_shared<Statement>(e);
		} else if (!current_for->condition) {
			current_for->condition = std::make_shared<Statement>(e);
		} else {
			current_for->loop = std::make_shared<Statement>(e);
			current_for->body = std::make_shared<Block>(currentBlock);
			currentBlock = current_for->body;
			for_status = BODY;
		} 
	}

	void end_for() {
		for_status = NONE;
		currentBlock = currentBlock->parent;
	}

	ForInstruction::Ptr current_for;
	ForStatus for_status = NONE;
};

struct WhileController : virtual ControllerBase {

};

struct MainController : virtual ForController, virtual WhileController {
	using Ptr = std::shared_ptr<MainController>;
	InitManager init_manager;


	void handleEvent(const Ex & e) {
		init_manager.handle(e);
		
		if (for_status != NONE) {
			feed_for(e);
		} 
		queueEvent(e);
	}
};


struct TShader : MainController {

	TShader() {
		declarations = std::make_shared<Block>();
		currentBlock = declarations;

		//add defaut main
		//functions.push_back(std::make_shared<Block>(createExp<>()))
	}

	Block::Ptr declarations;
	std::vector<Block::Ptr> functions;
	
	void cout() {
		declarations->cout();
	}
	void explore() {
		declarations->explore();
	}

	int version;
};


struct MainListener {
	
	MainListener() {
		currentShader = &shader;
	}

	void addEvent(const Ex & ex) {
		if (currentShader && isListening) {
			currentShader->handleEvent(ex);
		}
	}

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

	TShader shader;
	TShader * currentShader;
	bool isListening = true;
	static MainListener overmind;
};

MainListener MainListener::overmind = MainListener();

MainListener & listen() { return MainListener::overmind; }

template<typename T, CtorTypeDisplay tRule, OperatorDisplayRule  dRule, ParenthesisRule pRule, typename ... Args>
Ex createInit(const std::string & name, const Args &... args) 
{
	auto tor = std::make_shared<Ctor<T, dRule, pRule, tRule>>(name, (sizeof...(args) != 0));
	Ex expr = std::make_shared<Exp>(std::static_pointer_cast<OpBase>(tor), std::vector<Ex>{args...});
	listen().addEvent(expr);
	return expr;
}


ForController::EndFor::~EndFor() {
	listen().end_for();
}


#define GL_FOR_T(...) listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
listen().begin_for(); __VA_ARGS__;  \
for(ForController::EndFor csl_dummy_for; csl_dummy_for; )





//////////////////////////////////////


class NamedObjectBaseT {
protected:
	NamedObjectBaseT(const std::string & _name = "") : name(_name) { 
		//std::cout << " end check" << std::endl;
	}

	static const std::string typeStr() { return "dummyT"; }
	std::string name;
	NamedObjectBaseT * parent = nullptr;
public:
	Ex exp;

public:
	const std::string myName() const { return (parent ? parent->myName() + "." : "") + name; }
};

template<> Ex getExp(const NamedObjectBaseT & obj) { return obj.exp; }


template<typename T>
class NamedObjectT : public NamedObjectBaseT {
public:

	static const std::string typeStr() { return "dummyNameObjT"; }

protected:
	NamedObjectT(const std::string & _name = "") : NamedObjectBaseT(_name) {
		if (name == "") {
			name = getTypeStrTest<T>() + "_" + std::to_string(counter);
			++counter;
		}
	}

	static int counter;

public:

};
template<typename T> int NamedObjectT<T>::counter = 0;




struct T {

	~T() {
		//std::cout << name << " : " << expr << " rel ? " << b << std::endl;
		//std::cout << "dor" << n << std::endl;
		if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {

			MainListener::overmind.addEvent(createExp(std::make_shared<DtorBase>(), exp));

			InitManager::initManager.dtor(ctor);
		} else {
			std::cout << " !!!!  " << std::endl;
		}

	}

	T() : name("myname_" + std::to_string(counter)), n(counter++) {
		//	exp = std::make_shared<Exp>(std::static_pointer_cast<OpBase>(std::make_shared<Ctor>(name)),true);
		//std::cout << "ctor" << n << std::endl;
		exp = createInit<T>(name);
	}

	T(const std::string & s) : name(s), n(counter++) {
		//std::cout << " str ctor " << std::endl;
		exp = createInit<T>(name);
	}

	template <std::size_t N>
	T(const char(&s)[N]) : name(s), n(counter++) {
		//std::cout << " char ctor " << std::endl;
		exp = createInit<T>(name);
	}

	template<typename U, typename ...Us, typename = std::enable_if_t<NotInits<U, Us...>::value> >
	T(const U & u, const Us & ...us) : name("myname_" + std::to_string(counter)), n(counter++) {
		//std::cout << "multictor " << std::endl; 
		exp = createInit<T>(name, getExp(u), getExp(us)...);
	}

	//////

	template<CtorTypeDisplay display = DISPLAY>
	T(const Ex & _exp) : name("myname_" + std::to_string(counter)), n(counter++) {
		exp = createInit<T, display, NONE, NO_PARENTHESIS>(name, _exp);
	}

	T(const Tinit & t) : name(t.name) {
		exp = createInit<T, DISPLAY, NONE, NO_PARENTHESIS>(name, t.exp);
	}

	///////


	T & operator=(const T& other) {
		//std::cout << " op = " << std::endl;
		isNotInit(other.exp);
		addExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), other.exp);
		return *this;
	}

	template<typename U, typename = std::enable_if_t<NotChars<U>::value> >
	T & operator=(const U& other) {
		//std::cout << " op = " << std::endl;
		auto otherExp = getExp(other);
		isNotInit(otherExp);
		addExp(std::make_shared<SingleCharBinaryOp<'=', NO_PARENTHESIS>>(), createExp(std::make_shared<Alias>(name)), otherExp);
		return *this;
	}

	T & operator=(const Tinit & other) = delete;

	Ex exp;
	std::string name;
	int n;
	static int counter;
};
int T::counter = 0;

template<> Ex getExp<T>(const T & t) {
	return t.exp;
}

const T operator+(const T&a, const T&b) {
	return T(createExp(std::make_shared<SingleCharBinaryOp<'+'>>(), getExp(a), getExp(b)));
}

const T operator*(const T&a, const T&b) {
	return T(createExp(std::make_shared<SingleCharBinaryOp<'*', NO_PARENTHESIS>>(), getExp(a), getExp(b)));

}
const T fun(const T &a, const T &b) {
	return T(createExp(std::make_shared<FunctionOp<>>("fun"), a.exp, b.exp));
}

Tinit::Tinit(const T & u, const std::string & s) : name(s), exp(u.exp) { }

Tinit operator<<(const T & t, const std::string & s) {
	return Tinit(t, s);
}

