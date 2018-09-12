#pragma once

#include <string>
#include <map>
#include <memory>

struct Exp;
using Ex = std::shared_ptr<Exp>;

enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN };
enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };

struct OpBase {
	OpBase(OperatorDisplayRule rule, ParenthesisRule useParenthesis = USE_PARENTHESIS, bool args = true)
		: displayRule(rule), parRule(useParenthesis), hasArgs(args) {}

	OperatorDisplayRule displayRule = IN_FRONT;
	ParenthesisRule parRule;
	bool disabled = false;
	mutable bool hasArgs = true;

	virtual std::string str() const { return "dummy"; }
};

using OpB = std::shared_ptr<OpBase>;

struct Op : OpBase {};

template<typename T>
struct Litteral : OpBase {
	Litteral(const T & _i) : OpBase(NONE, NO_PARENTHESIS), i(_i) {}
	virtual std::string str() const { return std::to_string(i); }
	T i;
};

struct CtorBase : OpBase {
	CtorBase(const std::string & s, bool args) : 
		OpBase(args ? IN_FRONT : NONE, args ? USE_PARENTHESIS : NO_PARENTHESIS, args), name(s),  n(counter++) {}

	std::string name;
	int n;
	bool isInit = true;
	mutable bool firstStr = true;

	static int counter;
};
int CtorBase::counter = 0;

using CtorBasePtr = std::shared_ptr<CtorBase>;

template<typename T> std::string getTypeStrTest() {
	return "T";
}

template<typename T, OperatorDisplayRule dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS>
struct Ctor : CtorBase {
	Ctor(const std::string & s, bool hasArgs) : CtorBase(s,hasArgs){
		displayRule = dRule;
		parRule = pRule;
	}
	virtual std::string str() const { 
		if (firstStr) {
			firstStr = false;
			if (isInit) {
				if (hasArgs) {
					return getTypeStrTest<T>() + " " + name + " = " + (displayRule == NONE ? std::string("") : getTypeStrTest<T>());
				} else {
					return getTypeStrTest<T>() + " " + name;
				}
			} else {
				return (displayRule == NONE ? std::string("") : getTypeStrTest<T>());
			}
		} else {
			hasArgs = false;
			return name;
		}
		
	}
};

struct Alias : OpBase {
	Alias(const std::string & s) : OpBase(NONE,NO_PARENTHESIS), name(s) {}
	virtual std::string str() const { return name; }
	std::string name;
};

//struct Assignment : OpBase {
//	Assignment() : {}
//	virtual std::string str(int m) const { return  "assign"; }
//};

template<char c, ParenthesisRule p = USE_PARENTHESIS>
struct SingleCharBinaryOp : OpBase {
	SingleCharBinaryOp() : OpBase(IN_BETWEEN, p) {}
	virtual std::string str() const { return std::string(1,c); }
};

struct FunctionOp : OpBase {
	FunctionOp(const std::string & s) : OpBase(IN_FRONT), name(s) {}
	virtual std::string str() const { return name; }
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

		std::string out = op->displayRule == IN_BETWEEN ? "" : op->str();

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

		return out;
	}

	OpB op;
	std::vector<Ex> args;
};


template<typename Operator, typename ... Args> Ex createExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	return std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...});
}

template<typename T, OperatorDisplayRule  dRule = IN_FRONT, ParenthesisRule pRule = USE_PARENTHESIS,
	typename ... Args> Ex createInit(const std::string & name, const Args &... args) {
	auto tor = std::make_shared<Ctor<T,dRule,pRule>>(name, ( sizeof...(args) != 0 ) );
	Ex expr = std::make_shared<Exp>(std::static_pointer_cast<OpBase>(tor), std::vector<Ex>{args...});
	InitManager::initManager.ctor(tor);
	Manager::man.add(expr);
	return expr;
}

template<typename Operator, typename ... Args> void addExp(const std::shared_ptr<Operator> &op, const Args &... args) {
	Manager::man.add(std::make_shared<Exp>(std::static_pointer_cast<OpBase>(op), std::vector<Ex>{args...}));
}

void isNotInit(const Ex & expr) {
	if (auto ctor = std::dynamic_pointer_cast<CtorBase>(expr->op)) {
		ctor->isInit = false;
		ctor->disabled = true;
	}
}

struct InitManager {
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

template<typename T> Ex getExp(const T & t) { return Ex(); }

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

struct T {

	~T() {
		//std::cout << name << " : " << expr << " rel ? " << b << std::endl;
		//std::cout << "dor" << n << std::endl;
		if (auto ctor = std::dynamic_pointer_cast<CtorBase>(exp->op)) {
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
	T(const char(&s)[N] ) : name(s), n(counter++) {
		//std::cout << " char ctor " << std::endl;
		exp = createInit<T>(name);
	}

	template<typename U, typename ...Us, typename = std::enable_if_t<NotInits<U,Us...>::value> >
	T(const U & u, const Us & ...us) : name("myname_" + std::to_string(counter)), n(counter++) {
		//std::cout << "multictor " << std::endl; 
		exp = createInit<T>(name, getExp(u), getExp(us)...);
	}

	//////


	T(const Ex & _exp) : name("myname_" + std::to_string(counter)), n(counter++) {
		exp = createInit<T,NONE,NO_PARENTHESIS>(name, _exp);
	}

	T(const Tinit & t) : name(t.name) {
		exp = createInit<T,NONE, NO_PARENTHESIS>(name, t.exp);
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
		addExp(std::make_shared<SingleCharBinaryOp<'=',NO_PARENTHESIS>>(),  createExp(std::make_shared<Alias>(name)), otherExp);
		return *this;
	}

	T & operator=(const Tinit & other) = delete;

	Ex exp;
	std::string name;
	int n;
	static int counter;
};
int T::counter = 0;

template<> Ex getExp<T> (const T & t) {
	return t.exp;
}

const T operator+(const T&a, const T&b) {
	return T(createExp(std::make_shared<SingleCharBinaryOp<'+'>>(), getExp(a), getExp(b)));
}

const T operator*(const T&a, const T&b) {
	return T(createExp(std::make_shared<SingleCharBinaryOp<'*',NO_PARENTHESIS>>(), getExp(a), getExp(b)));

}
const T fun(const T &a, const T &b) {
	return T(createExp(std::make_shared<FunctionOp>("fun"), a.exp, b.exp));
}

Tinit::Tinit(const T & u, const std::string & s) : name(s), exp(u.exp) { }

Tinit operator<<(const T & t, const std::string & s) {
	return Tinit(t,s);
}



void InitManager::ctor(const CtorBasePtr & tor) {
	//std::cout << " manager ctor " << tor->n << std::endl;
	currentUp = tor->n;
	decls[currentUp] = tor;
}


void InitManager::dtor(const CtorBasePtr & tor) {
	//std::cout << " manager dtor " << tor->n << std::endl;
	if (tor->n < currentUp) {
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