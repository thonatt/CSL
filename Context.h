#pragma once
#include <vector>
#include <string>
#include <functional>

//#include <set>

//#define ER std::cerr <<"ERROR : " << __FILE__ ",\n\tLINE " << __LINE__ << ",\n\tFUNCTION " << __FUNCTION__ << std::endl; exit(1)
//#define ERR std::cout << "error" << std::endl; exit(1);

template<typename ... Ts> struct SizeOfT;
template<typename ... Ts> constexpr unsigned int SizeOf = SizeOfT<Ts...>::value;

template<> struct SizeOfT<> {
	static const unsigned int value = 0;
};

template<typename First, typename ... Rest> struct SizeOfT < First, Rest... > {
	static const unsigned int value = 1 + SizeOf<Rest...>;
};

template<typename T> struct TypeStr {
	static const std::string str() { return "dummyT"; }
};
template<> struct TypeStr<void> {
	static const std::string str() { return "void"; }
};

template<typename ... Ts> struct Releaser;

template<> struct Releaser<double> {
	static void release(const double & d) {}
};

template<typename TA, typename TB, typename ... Ts> struct Releaser<TA, TB, Ts...> {
	static void release(const TA & ta, const TB & tb, const Ts & ... ts) {
		Releaser<TA>::release(ta);
		Releaser<TB, Ts...>::release(tb, ts...);
	}
};

template<typename ... Ts> void release(const Ts & ...ts) {
	Releaser<Ts...>::release(ts...);
}

template<typename T> struct Releaser<T> {
	static void release(const T & t) {
		t.released = true;
	}
};

template<typename T> struct NameAccessor {
	static std::string & getName(const T & t) {
		return t.name;
	}
};

template<typename T> std::string & getName(const T & t) {
	return NameAccessor<T>::getName(t);
}

//class NamedObjectBase {
//protected:
//	
//	NamedObjectBase(const std::string & _name = "") : name(_name) { }
//
//	static const std::string typeStr() { return "dummyT"; }
//public:
//	
//};

template<typename T>
class NamedObject  /*: public NamedObjectBase */ {
public:
	friend struct Releaser<NamedObject<T>>;
	friend struct NameAccessor<NamedObject<T>>;

	static const std::string typeStr() { return "dummyNameObjT"; }

protected:
	
	NamedObject(const std::string & _name = "") : name(_name) {
		if (name == "") {
			name = T::typeStr() + "_" + std::to_string(counter);
			++counter;
		}
	}
	
	static int counter;

public:
	

protected:
	mutable std::string name;
	mutable bool released = true;
};
template<typename T> int NamedObject<T>::counter = 0;

//template<typename T> struct Releaser<T> {
//	static void release(const T & t) {
//		release(static_cast<const NamedObject<T::GLtype>&>(t));
//		//t.released = true;
//	}
//};


struct Line {
	std::string cmd;
	int offset;
};

class ShaderBase;

class BlockBase {

protected:
	virtual void addCmd(const std::string & cmd) {
		//std::cout << "add cmd base " << &lines << std::endl;
		getLines().push_back({ cmd, currentOffset });
	}

	std::vector<Line> & getLines();
public:
	void ignoreNextCmds(int n) { ignoreCounter += n; }

public:
	bool & enable() { return enableCmd; }

	virtual void removeCmd(int i = 1) {
		if (!currentShader) {
			return;
		}
		for (int j = 0; j < i; ++j) {
			if (lastCmdWasIgnored) {
				++ignoreCounter;
				continue;
			}
			if (!getLines().empty()) {
				//std::cout << "rmv " << getLines().back().cmd << std::endl;
				getLines().pop_back();
			}
		}
	}

protected:
	ShaderBase * currentShader;
	int currentOffset = 0;
	int ignoreCounter = 0;
	bool enableCmd = true;
	bool lastCmdWasIgnored = false;
};

struct ForBlock : virtual BlockBase {

	bool for_declaration = false, for_declaration_empty = true, for_body = false;
	int for_levels = 0;

	virtual void addCmd(const std::string & cmd) {
		//std::cout << "add cmd for " << &lines << std::endl;
		if (for_declaration) {
			getLines().back().cmd += " " + cmd;
			for_declaration_empty = false;
		} else {
			BlockBase::addCmd(cmd);
		}
	}

	void begin_for_declaration() {
		addCmd("for(");
		for_declaration = true;
		for_declaration_empty = true;
	}
	void end_for_declaration() {
		if (!for_declaration_empty) {
			getLines().back().cmd.pop_back();
		}
		addCmd("){");
		for_declaration = false;
		++currentOffset;
		for_body = true;
		++for_levels;
	}
	void end_for_body() {
		--currentOffset;
		--for_levels;
		addCmd("}");
		for_body = false;
	}

	void declare_break() {
		if (for_levels) {
			addCmd("break;");
		}
	}
	void declare_continue() {
		//std::cout << for_levels << std::endl;
		if (for_levels) {
			addCmd("continue;");
		}
	}

};

struct IfBlock : virtual BlockBase {
	template<typename T>
	void begin_if(const NamedObject<T> & obj) {
		addCmd("if( " + getName(obj) + " ){");
		++currentOffset;
	}
	void end_if() {
		--currentOffset;
		addCmd("}");
	}
	void begin_else() {
		getLines().back().cmd += " else {";
		++currentOffset;
	}

	template<typename T>
	void begin_else_if(const NamedObject<T> & obj) {
		getLines().back().cmd += " else if ( " + getName(obj) + " ){";
		++currentOffset;
	}
};


// We need to include these as they are declared in Algebra and BuildingBlocks and g++ won't find them there.
template<typename T> const std::string strFromDecl(const T & o);
template<typename ...Ts> const std::string strFromDecl(const Ts & ... ts) ;
template<typename ...Ts> const std::string strFromDecl(const std::tuple<Ts...> & vs);
template<typename R, template<typename...> class Params, typename... Args>
R call_from_tuple(const std::function<R(Args...)> &func, const Params<Args...> &params);

struct FunctionBlock : virtual BlockBase {

	bool insideFunction = false;
	
	template<typename ReturnType, typename ... Args, typename String, typename ... Strings>
	std::tuple<Args...> begin_function_declaration(const std::string & fname, const std::tuple<String,Strings...> & argsnames) {
		addCmd(""); 
		ignoreNextCmds(SizeOf<Args...>);
		std::tuple<Args...> args(argsnames);
		//addCmd("end " + std::to_string(SizeOf<Args...>));
		addCmd(TypeStr<ReturnType>::str() + " " + fname + "(" + strFromDecl(args) + ") {");
		begin_function_declaration_base();
		return args;
	}

	template<typename ReturnType, typename ... Args>
	std::tuple<Args...>  begin_function_declaration(const std::string & fname) {
		addCmd(""); 
		ignoreNextCmds(SizeOf<Args...>);
		std::tuple<Args...> args;
		addCmd( TypeStr<ReturnType>::str() + " " + fname + "(" + strFromDecl(args) + ") {");
		begin_function_declaration_base();
		return args;
	}

	void begin_function_declaration_base() {
		if (insideFunction) {
			enableCmd = false;
			return;
		}
		insideFunction = true;
		++currentOffset;
	}

	template<typename T>
	void end_function_declaration(const NamedObject<T> & o) {
		addCmd("return " + getName(o) + ";");
		release(o);
		end_function_declaration();
	}

	void end_function_declaration() {
		if (!insideFunction) {
			enableCmd = true;
			return;
		}
		--currentOffset;
		addCmd("}");
		insideFunction = false;
	}

};

class ShaderBase;

struct Context : virtual ForBlock, virtual IfBlock, virtual FunctionBlock {

	void addCmd(const std::string & cmd) {
		if (!currentShader) {
			return;
		}

		lastCmdWasIgnored = (ignoreCounter > 0);
		if (ignoreCounter > 0) {
			//std::cout << ignoreCounter << " ignore : " << cmd << std::endl;
			--ignoreCounter; 
			return;
		}

		//std::cout << "add cmd derived " << &lines << std::endl;
		if (enableCmd) {
			ForBlock::addCmd(cmd);
		}
	}

	void addStruct(const std::string & s);

	void setShader(ShaderBase * shader) { currentShader = shader; }
	void endShader();

public:
	static Context ctx;
	
};

Context Context::ctx;

Context & Ctx() { return Context::ctx; }