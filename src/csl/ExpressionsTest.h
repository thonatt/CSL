#pragma once

#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <sstream> 

#include <set> //for storing counters

#include <algorithm> //for std::replace
#include <iomanip> // for std::setprecision

#include "FunctionHelpers.h"

#include "Operators.h"

#define EX(type, var) getExp(std::forward<type>(var))

namespace csl {
	
	// Forward delcarations.
	
	template<typename ReturnTList, typename ... Fs>
	void init_function_declaration(const std::string & fname, const Fs & ...fs);
	
	
	//////////////////////////

	struct CounterData {
		size_t value = 0;
		bool is_tracked = false;
	};

	///////////////////////////

	class NamedObjectBase {
	public:
		NamedObjectBase(const std::string & _name = "", uint _flags = IS_USED | IS_TRACKED)
			: flags(_flags)
		{
			namePtr = std::make_shared<std::string>(_name);
			//std::cout << " end check" << std::endl;
		}

		bool isUsed() const {
			return flags & IS_USED;
		}

		bool isTracked() const {
			return flags & IS_TRACKED;
		}

		void setNotUsed() {
			flags = flags & ~(IS_USED);
		}

		Ex alias() const {
			return createExp<Alias>(strPtr());
		}

		Ex getExRef()
		{
			return static_cast<const NamedObjectBase*>(this)->getExRef();
		}

		Ex getExRef() const
		{
			flags = flags | IS_USED;
			if (flags & ALWAYS_EXP) {
				return exp;
				//return getExTmp();
			}
			if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
				ctor->setInit();
			}
			return alias();
		}

		Ex getExTmp()
		{
			return static_cast<const NamedObjectBase*>(this)->getExTmp();
		}

		Ex getExTmp() const
		{
			flags = flags | IS_USED;
			if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
				ctor->setTemp();
				ctor->disable();

				if (auto arg = ctor->firstArg()) {
					if (auto accessor = std::dynamic_pointer_cast<MemberAccessor>(arg)) {
						accessor->make_obj_tmp();
					}
				}
			}
			return exp;
		}

		Ex getEx() & { return getExRef(); }
		Ex getEx() const & { return getExRef(); }
		Ex getEx() && { return getExTmp(); }
		Ex getEx() const && { return getExTmp(); }

		void checkDisabling()
		{
			if (!(flags & IS_TRACKED)) {
				//std::cout << "disabling " << str() << std::endl;
				exp->disable();
			}
		}

		~NamedObjectBase()
		{
			if (!isUsed()) {
				//std::cout << " ~ setTemp " << exp->str() << std::endl;
				if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
					ctor->setTemp();
				}
			}
			//should check inside scope
		}

		//static std::string typeStr() { return "dummyT"; }

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
	class NamedObject : public NamedObjectBase {

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

		void checkName();

		static CounterData counterData;

	public:

	};

	template<typename T> CounterData NamedObject<T>::counterData = {};

	template<typename T>
	inline Ex getExp(T && t)
	{
		return std::forward<T>(t).getEx();
	}

	template<> inline Ex getExp<bool>(bool && b) {
		return createExp<Litteral<bool>>(b);
	}
	template<> inline Ex getExp<bool&>(bool & b) {
		return createExp<Litteral<bool>>(b);
	}

	template<> inline Ex getExp<int>(int && i) {
		return createExp<Litteral<int>>(i);
	}
	template<> inline Ex getExp<int&>(int & i) {
		return createExp<Litteral<int>>(i);
	}


	template<> inline Ex getExp<uint>(uint && i) {
		return createExp<Litteral<uint>>(i);
	}
	template<> inline Ex getExp<uint&>(uint & i) {
		return createExp<Litteral<uint>>(i);
	}

	template<> inline Ex getExp<float>(float && d) {
		return createExp<Litteral<float>>(d);
	}
	template<> inline Ex getExp<float&>(float & d) {
		return createExp<Litteral<float>>(d);
	}

	template<> inline Ex getExp<double>(double && d) {
		return createExp<Litteral<double>>(d);
	}
	template<> inline Ex getExp<double&>(double & d) {
		return createExp<Litteral<double>>(d);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

		virtual ~InstructionBase() = default;
		virtual void str(std::stringstream & stream, int & trailing, uint otps) { }
		virtual void cout(int & trailing, uint otps = DEFAULT) {}
		virtual void explore() {}
	};

	struct Block {
		using Ptr = std::shared_ptr<Block>;

		Block(const Block::Ptr & _parent = {}) : parent(_parent) {}

		virtual ~Block() = default;

		virtual void push_instruction(const InstructionBase::Ptr & i) {
			instructions.push_back(i);
		}

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
	};

	struct ReturnBlockBase : Block {
		using Ptr = std::shared_ptr<ReturnBlockBase>;
		using Block::Block;

		virtual ~ReturnBlockBase() = default;
		virtual RunTimeInfos getType() const {
			return getRunTimeInfos<void>();
		}

		virtual bool same_return_type(size_t other_type_hash) const {
			return false;
		}

		//bool hasReturnStatement = false;
	};

	template<typename ReturnType>
	struct ReturnBlock : ReturnBlockBase {
		using Ptr = std::shared_ptr<ReturnBlock<ReturnType>>;

		ReturnBlock(const Block::Ptr & _parent = {}) : ReturnBlockBase(_parent) {}
		virtual ~ReturnBlock() = default;

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

	struct Statement : InstructionBase {
		using Ptr = std::shared_ptr<Statement>;

		Statement(const Ex & e = {}) : ex(e) {}
		virtual ~Statement() = default;

		virtual void str(std::stringstream & stream, int & trailing, uint opts) {
			if ((opts & IGNORE_DISABLE) || !ex->disabled()) {
				stream << ((opts & NEW_LINE) ? instruction_begin(trailing, opts) : "");
				if ((opts & IGNORE_TRAILING)) {
					stream << ex->str(0);
				} else {
					stream << ex->str(trailing);
				}
				stream << instruction_end(opts);
			}
		}

		virtual void cout(int & trailing, uint opts = SEMICOLON & NEW_LINE) {
			if ((opts & IGNORE_DISABLE) || !ex->disabled()) {
				std::cout <<
					((opts & NEW_LINE) ? instruction_begin(trailing, opts) : "")
					<< ex->str(trailing)
					<< instruction_end(opts);
			}
		}

		void explore() {
			ex->explore();
		}

		Ex ex;
	};

	inline InstructionBase::Ptr toInstruction(const Ex & e) {
		auto statement = std::make_shared<Statement>(e);
		return std::dynamic_pointer_cast<InstructionBase>(statement);
	}

	struct EmptyStatement : Statement {

		EmptyStatement(uint _flags = 0) : Statement(Ex()), flags(_flags) {}

		static InstructionBase::Ptr create(uint _flags = 0) {
			return std::static_pointer_cast<InstructionBase>(std::make_shared<EmptyStatement>(_flags));
		}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			if (flags != 0) {
				stream << Statement::instruction_begin(trailing, flags) << Statement::instruction_end(flags);
			} else {
				stream << Statement::instruction_begin(trailing, opts) << Statement::instruction_end(opts);
			}
		}

		void cout(int & trailing, uint opts = 0) {
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

		void str(std::stringstream & stream, int & trailing, uint opts) {

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
							} else if (ctor->ctor_status == FORWARD) {
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
					} else {
						///
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

	struct SpecialStatement : Statement {
		using Statement::Statement;
		virtual ~SpecialStatement() = default;

		virtual bool checkStatementValidity(Block::Ptr block) const { return true; }
	};

	struct ReturnStatement : SpecialStatement {
		using Ptr = std::shared_ptr<ReturnStatement>;

		ReturnStatement(const Ex & e = {}) : SpecialStatement(e) {}

		void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON & NEW_LINE) {
			stream << instruction_begin(trailing, opts) << internal_str() << instruction_end(opts);
		}

		std::string internal_str() const {
			std::string s = "return";
			if (ex) {
				s += " " + ex->str(0);
			}
			return s;
		}

		void explore() {
			ex->explore();
		}
	};

	struct ContinueStatement : SpecialStatement {
		void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON & NEW_LINE) {
			stream << instruction_begin(trailing, opts) << "continue" << instruction_end(opts);
		}
	};

	struct DiscardStatement : SpecialStatement {
		void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON & NEW_LINE) {
			stream << instruction_begin(trailing, opts) << "discard" << instruction_end(opts);
		}
	};

	struct BreakStatement : SpecialStatement {
		void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON & NEW_LINE) {
			stream << instruction_begin(trailing, opts) << "break" << instruction_end(opts);
		}
	};

	struct CommentInstruction : InstructionBase {
		CommentInstruction(const std::string & s) : comment(s) {}

		void str(std::stringstream & stream, int & trailing, uint opts = DEFAULT) {
			stream << instruction_begin(trailing) << "//" << comment << std::endl;
		}
		std::string comment;
	};

	struct EmitVertexInstruction : InstructionBase {
		void str(std::stringstream & stream, int & trailing, uint opts = DEFAULT) {
			stream << instruction_begin(trailing) << "EmitVertex()" << instruction_end(opts);
		}
	};

	struct EndPrimitiveInstruction : InstructionBase {
		void str(std::stringstream & stream, int & trailing, uint opts = DEFAULT) {
			stream << instruction_begin(trailing) << "EndPrimitive()" << instruction_end(opts);
		}
	};

	template<typename ReturnType>
	void ReturnBlock<ReturnType>::str(std::stringstream & stream, int & trailing, uint opts)
	{
		Block::str(stream, trailing, opts);
		//if (!std::is_same<ReturnType, void>::value && !ReturnBlockBase::hasReturnStatement) {
		//	CommentInstruction("need return statement here").str(stream, trailing, opts);
		//}
	}

	struct OverloadData {
		OverloadData() {
			args = std::make_shared<Block>();
			body = std::make_shared<Block>();
		}
		Block::Ptr args, body;
	};

	template<typename ReturnTList, size_t it, typename ... Fs>
	struct FDeclImpl;

	template<typename ReturnTList, size_t it>
	struct FDeclImpl<ReturnTList, it> {
		static void str(std::stringstream & stream, int & trailing, uint opts,
			const std::string & fname, const std::vector<OverloadData> & funcs) { }
	};

	template<typename ReturnTList, size_t it, typename F, typename ... Fs>
	struct FDeclImpl<ReturnTList, it, F, Fs...> {
		static void str(std::stringstream & stream, int & trailing,
			uint opts, const std::string & fname, const std::vector<OverloadData> & funcs) {
			
			stream << InstructionBase::instruction_begin(trailing, opts) <<
				 getTypeStr<typename ReturnTList::GetType<it>>() << " " << fname << "(";

			int dummy_trailing = 0;
			const auto & args = funcs[it].args->instructions;
			for (int it = static_cast<int>(args.size()) - 1; it >= 0; --it) {
				args[it]->str(stream, dummy_trailing, (it == 0 ? 0 : COMMA | ADD_SPACE) );
			}

			stream << ") {\n";
			++trailing;

			funcs[it].body->str(stream, trailing, opts);
			
			--trailing;
			stream << InstructionBase::instruction_begin(trailing, opts) << "}\n\n";

			FDeclImpl<ReturnTList, it+1, Fs...>::str(stream, trailing, opts, fname, funcs);

		}
	};

	struct FuncDeclarationInstructionBase : InstructionBase {
		using Ptr = std::shared_ptr<FuncDeclarationInstructionBase>;

		FuncDeclarationInstructionBase(const std::string & name) : func_name(name) {
		}

		std::vector<OverloadData> overloads;
		std::string func_name;
	};

	template<typename ReturnTList, typename ... Fs>
	struct FuncDeclarationInstruction : FuncDeclarationInstructionBase {
		using Ptr = std::shared_ptr<FuncDeclarationInstruction>;

		FuncDeclarationInstruction(const std::string & name) 
		: FuncDeclarationInstructionBase(name) {
			FuncDeclarationInstructionBase::overloads.resize(sizeof...(Fs));
		}
	
		virtual void str(std::stringstream & stream, int & trailing, uint opts) override {
			FDeclImpl<ReturnTList, 0, Fs...>::str(stream, trailing, opts, 
				FuncDeclarationInstructionBase::func_name,
				FuncDeclarationInstructionBase::overloads
			);
		}
	};

	struct ForInstruction : InstructionBase {
		using Ptr = std::shared_ptr<ForInstruction>;

		ForInstruction() {
			args = std::make_shared<ForArgsBlock>();
			body = std::make_shared<Block>();
		}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			stream << instruction_begin(trailing, opts) << "for( ";
			args->str(stream, trailing, IGNORE_TRAILING);
			stream << "){\n";
			++trailing;
			body->str(stream, trailing, opts);
			--trailing;
			stream << instruction_begin(trailing, opts) << "}\n";
		}

		ForArgsBlock::Ptr args;
		Block::Ptr body;
	};

	struct IfInstruction : InstructionBase {
		using Ptr = std::shared_ptr<IfInstruction>;

		struct IfBody {
			Block::Ptr body;
			Statement::Ptr condition;
		};

		IfInstruction(std::shared_ptr<IfInstruction> _parent = {}) : parent_if(_parent) {}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			const int numBodies = (int)bodies.size();
			for (int i = 0; i < numBodies; ++i) {
				if (bodies[i].condition) {
					if (i == 0) {
						stream << instruction_begin(trailing, opts) << "if( ";
					} else {
						stream << "else if( ";
					}
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

	struct WhileInstruction : InstructionBase {
		using Ptr = std::shared_ptr<WhileInstruction>;

		WhileInstruction(const Ex & ex, const Block::Ptr & parent) {
			condition = std::make_shared<Statement>(ex);
			body = std::make_shared<Block>(parent);
		}

		void str(std::stringstream & stream, int & trailing, uint opts) {
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

	struct SwitchCase : InstructionBase {
		using Ptr = std::shared_ptr<SwitchCase>;

		SwitchCase(const Ex & ex, const Block::Ptr & parent) {
			if (ex) {
				label = std::make_shared<Statement>(ex);
			}
			body = std::make_shared<Block>(parent);
		}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			if (label) {
				stream << instruction_begin(trailing, opts) << "case ";
				label->str(stream, trailing, NOTHING | IGNORE_TRAILING);
			} else {
				stream << instruction_begin(trailing, opts) << "default";
			}
			stream << " : {\n";
			++trailing;
			body->str(stream, trailing, DEFAULT);
			--trailing;
			stream << instruction_begin(trailing, opts) << "}\n";
		}

		Statement::Ptr label;
		Block::Ptr body;
	};

	struct SwitchInstruction : InstructionBase {
		using Ptr = std::shared_ptr<SwitchInstruction>;

		SwitchInstruction(const Ex & ex, const Block::Ptr & parent, const SwitchInstruction::Ptr & _parent_switch = {}) {
			condition = std::make_shared<Statement>(ex);
			body = std::make_shared<Block>(parent);
			parent_switch = _parent_switch;
		}

		void add_case(const Ex & ex, Block::Ptr & currentBlock) {
			current_case = std::make_shared<SwitchCase>(ex, body);
			body->push_instruction(current_case);
			currentBlock = current_case->body;
		}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			stream << instruction_begin(trailing, opts) << "switch( ";
			condition->str(stream, trailing, NOTHING | IGNORE_DISABLE);
			stream << " ){\n";
			++trailing;
			body->str(stream, trailing, opts);
			--trailing;
			stream << instruction_begin(trailing, opts) << "}\n";
		}

		Statement::Ptr condition;
		Block::Ptr body;
		SwitchCase::Ptr current_case;
		SwitchInstruction::Ptr parent_switch;
	};

	enum SeparatorRule { SEP_IN_BETWEEN, SEP_AFTER_ALL };

	template<SeparatorRule s, int N, typename ... Ts> struct DisplayDeclaration;

	template<SeparatorRule s, int N, typename T, typename ... Ts>
	struct DisplayDeclaration<s, N, T, Ts...> {
		static std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
			return
				InstructionBase::instruction_begin(trailing) + DeclarationStr<T>::str(v[v.size() - N], trailing) +
				((s == SEP_AFTER_ALL || (s == SEP_IN_BETWEEN && N != 1)) ? separator : "") +
				DisplayDeclaration<s, N - 1, Ts...>::str(v, trailing, separator);
		}
	};

	template<SeparatorRule s, typename ... T>
	struct DisplayDeclaration<s, 0, T...> {
		static std::string str(const std::vector<std::string> & v, int & trailing, const std::string & separator) { return ""; }
	};

	template<SeparatorRule s, typename ... Ts>
	std::string memberDeclarations(const std::vector<std::string> & v, int & trailing, const std::string & separator) {
		return DisplayDeclaration<s, sizeof...(Ts), Ts...>::str(v, trailing, separator);
	}

	struct FuncBase : NamedObject<FuncBase> {
		FuncBase(const std::string & s = "") : NamedObject<FuncBase>(s, 0) {}
		static std::string typeStr(int trailing = 0) { return "function"; }
		static std::string typeNamingStr(int trailing = 0) { return typeStr(trailing); }
	};

	
	template<typename ... Args>
	struct StructDeclaration : InstructionBase {

		template<typename ... Strings>
		StructDeclaration(const std::string & _name, const Strings &... _names) :
			member_names{ _names... }, name(_name) {}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			stream << instruction_begin(trailing, opts) << "struct " << name << " {\n";
			++trailing;
			stream << memberDeclarations<SEP_AFTER_ALL, Args...>(member_names, trailing, ";\n");
			--trailing;
			stream << instruction_begin(trailing, opts) << "}\n";
		}

		std::vector<std::string> member_names;
		std::string name;
	};

	template<typename T, typename ... Args>
	struct UnNamedInterfaceDeclaration : InstructionBase {

		template<typename ... Strings>
		UnNamedInterfaceDeclaration(const Strings &... _names) :
			member_names{ _names... } {}

		void str(std::stringstream & stream, int & trailing, uint opts) {
			stream << instruction_begin(trailing, opts) << getTypeStr<T>() << " {\n";
			++trailing;
			stream << memberDeclarations<SEP_AFTER_ALL, Args...>(member_names, trailing, ";\n");
			--trailing;
			stream << instruction_begin(trailing, opts) << "};\n";
		}

		std::vector<std::string> member_names;
	};

	template<typename T, typename ... Args>
	struct InterfaceDeclarationStr {

		template<typename ... Strings>
		static std::string str(int trailing, const Strings & ... member_names) {
			std::string out = " {\n";
			++trailing;
			out += memberDeclarations<SEP_AFTER_ALL, Args...>({ member_names... }, trailing, ";\n");
			--trailing;
			out += InstructionBase::instruction_begin(trailing) + "}";
			return out;
		}

	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

			currentBlock->push_instruction(toInstruction(e));
		}

		Block::Ptr currentBlock;

		virtual ~ControllerBase() = default;

	};

	struct FunctionController : virtual ControllerBase {
		
		template<typename ReturnTList, typename ... Fs>
		void begin_func_internal(const std::string & name) {
			current_func = std::make_shared<FuncDeclarationInstruction<ReturnTList, Fs...>>(name);
			current_func_num_args = { GetArgTList<Fs>::size ... };
			current_func_parent = currentBlock; 
			current_func_overload = -1;
			next_overload();
		}

		void checkNumArgs() {
			if (current_func && current_func_arg_counter == current_func_num_args[current_func_overload]) {
				currentBlock = current_func->overloads[current_func_overload].body;
				feedingArgs = false;
			}
		}

		void checkFuncArgs() {
			if (current_func && feedingArgs) {
				checkNumArgs();
				++current_func_arg_counter;			
			}
		}

		void next_overload() {
			if (current_func) {
				++current_func_overload;
				current_func_arg_counter = 0;
				feedingArgs = true;
				if (current_func_overload < current_func->overloads.size()) {
					currentBlock = current_func->overloads[current_func_overload].args;
					checkNumArgs();
				}
			}
		}

		void end_func() {
			currentBlock = current_func_parent;
			current_func_parent = {};
			current_func = {};
		}


		FuncDeclarationInstructionBase::Ptr current_func;
		Block::Ptr current_func_parent;
		std::vector<size_t> current_func_num_args;
		int current_func_overload;
		size_t current_func_arg_counter = 0;
		bool feedingArgs = true;
	};

	struct ForController : virtual ControllerBase {

		virtual void begin_for() {
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

		void begin_while(const Ex & ex) {
			//std::cout << " begin while " << std::endl;
			auto while_instruction = std::make_shared<WhileInstruction>(ex, currentBlock);
			currentBlock->push_instruction(while_instruction);
			currentBlock = while_instruction->body;
		}

		virtual void end_while() {
			currentBlock = currentBlock->parent;
		}
	};

	struct SwitchController : virtual ControllerBase {
		void begin_switch(const Ex & ex) {
			current_switch = std::make_shared<SwitchInstruction>(ex, currentBlock, current_switch);
			currentBlock->push_instruction(current_switch);
			currentBlock = current_switch->body;
		}

		virtual void add_case(const Ex & ex) {
			current_switch->add_case(ex, currentBlock);
		}
		//void end_case() {
		//	currentBlock = currentBlock->parent;
		//}

		virtual void end_switch() {
			if (current_switch->current_case) {
				currentBlock = currentBlock->parent;
			}
			currentBlock = currentBlock->parent;
			current_switch = current_switch->parent_switch;
		}

		SwitchInstruction::Ptr current_switch;
	};

	struct MainController :
		virtual FunctionController,
		virtual ForController,
		virtual WhileController,
		virtual IfController,
		virtual SwitchController
	{
		using Ptr = std::shared_ptr<MainController>;

		void add_blank_line(int n = 0) {
			check_end_if();
			if (currentBlock) {
				for (int i = 0; i < n; ++i) {
					currentBlock->push_instruction(EmptyStatement::create(NEW_LINE | IGNORE_TRAILING | NOTHING));
				}
			}
		}

		virtual void begin_for() override {
			check_end_if();
			ForController::begin_for();
		}

		virtual void end_for() override {
			check_end_if();
			ForController::end_for();
		}

		virtual void end_while() override  {
			check_end_if();
			WhileController::end_while();
		}

		virtual void add_case(const Ex & ex) override  {
			check_end_if();
			SwitchController::add_case(ex);
		}

		virtual void end_switch() override {
			check_end_if();
			SwitchController::end_switch();
		}

		void handleEvent(const Ex & e) {
			check_end_if();
			checkFuncArgs();
			queueEvent(e);
		}
	};

	struct ShaderBase : MainController {
		using Ptr = std::shared_ptr<ShaderBase>;

		MainBlock::Ptr declarations;
		std::vector<InstructionBase::Ptr> structs;
		std::vector<InstructionBase::Ptr> unnamed_interface_blocks;
		std::vector<InstructionBase::Ptr> functions;

		ShaderBase() {
			declarations = std::make_shared<MainBlock>();
			currentBlock = declarations;

			//add defaut main
			//functions.push_back(std::make_shared<Block>(createExp<>()))
		}

		virtual std::string header() const { return ""; }

		std::string str() const {
			int trailing = 1;

			std::stringstream out;

			out << Statement::instruction_begin(trailing) << header() << "\n\n";

			for (const auto & struc : structs) {
				struc->str(out, trailing, DEFAULT);
				out << "\n";
			}

			for (const auto & interface_block : unnamed_interface_blocks) {
				interface_block->str(out, trailing, DEFAULT);
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
		}

		template<typename ...Args, typename ... Strings>
		void add_struct(const std::string & name, const Strings & ... names) {
			auto struct_declaration = std::make_shared < StructDeclaration<Args...> >(name, names...);
			structs.push_back(std::static_pointer_cast<InstructionBase>(struct_declaration));
		}

		template<typename T, typename ...Args, typename ... Strings>
		void add_unnamed_interface_block(const Strings & ... names) {
			auto interface_declaration = std::make_shared<UnNamedInterfaceDeclaration<T, Args...> >(names...);
			unnamed_interface_blocks.push_back(interface_declaration);
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string & name, const Fs & ... fs) {
			begin_func_internal<ReturnTList, Fs...>(name);
			functions.push_back(current_func);
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

		template<typename S, typename ... Args>
		void add_statement(Args && ... args) {
			auto statement = std::make_shared<S>(args...);
			if (auto st = std::dynamic_pointer_cast<SpecialStatement>(statement)) {
				if (!st->checkStatementValidity(currentBlock)) {
					return;
				}
			}

			currentBlock->push_instruction(statement);
		}

		//void add_return_statement() {
		//	auto return_statement = std::make_shared<ReturnStatement>(EmptyExp::get());
		//	if (auto return_block = get_return_block()) {
		//		if (return_block->getType().is_void()) {
		//			currentBlock->push_instruction(return_statement);
		//		} else {
		//			currentBlock->push_instruction(
		//				std::make_shared<CommentInstruction>("unexpected " + return_statement->internal_str() + "; in non void function ")
		//			);
		//		}
		//	}
		//}

		//template<typename R_T>
		//void add_return_statement(R_T && t) {
		//	using T = CleanType<R_T>;
		//	Ex ex = EX(R_T, t);
		//	auto return_statement = std::make_shared<ReturnStatement>(ex);

		//	if (auto return_block = get_return_block()) {
		//		bool same_type = return_block->same_return_type(typeid(T).hash_code());
		//		if (same_type || return_block->getType().isConvertibleTo(getRunTimeInfos<T>())) {
		//			currentBlock->push_instruction(return_statement);
		//			return_block->hasReturnStatement = true;
		//		} else {
		//			currentBlock->push_instruction(std::make_shared<CommentInstruction>("wrong result type in : return " + ex->str(0)));
		//		}
		//	}
		//}
	};

	template<GLVersion version>
	struct IShader : ShaderBase {
		using Ptr = std::shared_ptr<IShader>;

		virtual std::string header() const {
			return "#version " + gl_version_str<version>();
		}
	};

	struct MainListener {

		MainListener() {
			//shader = std::make_shared<TShader>();
			//currentShader = shader;
		}

		void add_blank_line(int n = 0) {
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

		template<typename S, typename ... Args>
		void add_statement(Args && ... args) {
			if (currentShader && active() ) {
				currentShader->add_statement<S, Args...>(std::forward<Args>(args)...);
			}
		}


		/////////////////////////////////////////////////

		void begin_for() {
			if (currentShader) {
				currentShader->begin_for();
			}
		}

		virtual void begin_for_args() {
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
				std::cout << "stacked for arg : " << ex->str(0) << std::endl;
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

		template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
		void begin_while(B && b) {
			if (currentShader) {
				currentShader->begin_while(EX(B, b));
			}
		}

		void end_while() {
			if (currentShader) {
				currentShader->end_while();
			}
		}

		/////////////////////////////////////////////////

		template<typename C, typename = std::enable_if_t< IsInteger<C> > >
		void begin_switch(C && cond) {
			if (currentShader) {
				currentShader->begin_switch(EX(C, cond));
			}
		}

		template<typename C>
		void begin_switch_case(C && _case) {
			if (currentShader) {
				currentShader->add_case(EX(C, _case));
			}
		}
		void begin_switch_case() {
			if (currentShader) {
				currentShader->add_case(Ex());
			}
		}


		void end_switch() {
			if (currentShader) {
				currentShader->end_switch();
			}
		}

		//////////////////////////////

		template<bool dummy, typename ...Args, typename ... Strings>
		void add_struct(const std::string & name, const Strings & ... names) {
			if (currentShader) {
				currentShader->add_struct<Args...>(name, names...);
			}
		}

		template<typename T, typename ...Args, typename ... Strings>
		void add_unnamed_interface_block(const std::string & dummy, Strings & ... names) {
			if (currentShader) {
				currentShader->add_unnamed_interface_block<T, Args...>(names...);
			}
		}

		/////////////////////////////////////////////////

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string & name, const Fs & ... fs) {
			if (currentShader) {
				currentShader->begin_func<ReturnTList>(name, fs...);
			}
		}

		void end_func() {
			if (currentShader) {
				currentShader->end_func();
			}
		}

		void next_overload() {
			if (currentShader) {
				currentShader->next_overload();
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

		void reset_counters() {
			for (CounterData * counter_ptr : counter_ptrs) {
				*counter_ptr = CounterData();
			}
			counter_ptrs.clear();
		}

		void add_active_counter(CounterData & counter) {
			counter_ptrs.insert(&counter);
		}

		bool & active() { return isListening; }

		//TShader::Ptr shader;
		ShaderBase::Ptr currentShader;

		std::set<CounterData *> counter_ptrs;
		bool isListening = true;
	};

	inline MainListener & listen() {
		static MainListener overmind;
		return overmind;
	}


	template<typename T>
	inline void NamedObject<T>::checkName() {

		if (*namePtr == "") {

			namePtr = std::make_shared<std::string>(getTypeNamingStr<T>() + "_" + std::to_string(counterData.value));
			std::replace(namePtr->begin(), namePtr->end(), ' ', '_');

			++counterData.value;

			if (!counterData.is_tracked) {
				listen().add_active_counter(counterData);
				counterData.is_tracked = true;
			}
			
		}

	}

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

		~BeginWhile() {
			listen().end_while();
		}
	};

	struct BeginIf {
		operator bool() const { return true; }
		~BeginIf() {
			listen().end_if_sub_block();
		}
	};

	struct BeginElse {
		operator bool() const { return false; }
		~BeginElse() {
			listen().end_if();
		}
	};

	struct BeginSwitch {
		operator int() const {
			++count;
			if (count == 1) {
				listen().active() = false;
			} else if (count == 2) {
				listen().active() = true;
			}
			return count > 2 ? 0 : unlikely_case;
		}

		~BeginSwitch() {
			listen().end_switch();
		}
		mutable int count = 0;
		int unlikely_case = 696969;
	};
	

	struct EndFor {

		~EndFor() {
			listen().end_for();
		}

		explicit operator bool() {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}

		bool first = true;
	};


	inline void lineBreak(int n = 1) { listen().add_blank_line(n); }
	
	template<typename ReturnTList, typename FuncTList>
	struct Function : FuncBase {
		static_assert(ReturnTList::size == FuncTList::size, "numbers of overload and return type dont match");
		
		template<typename ... Fs>
		Function(const std::string & _name, const Fs & ... _fs) : FuncBase(_name) {
			init_function_declaration<ReturnTList>(str(), _fs ...);
		}
		
		template<typename ... Args>
		using ReturnType = OverloadResolutionType<ReturnTList, FuncTList, TList<Args...>>;
		
		template<typename ... Args>
		ReturnType<Args...> operator()(Args && ...args) {
			//in case return type is void, no variable will be returned, so function call must be explicitely sent to the listener
			if (std::is_same<ReturnType<Args...>, void>::value) {
				listen().addEvent(createFCallExp(str(), EX(Args, args)...));
			}
			
			return ReturnType<Args...>(createFCallExp(str(), EX(Args, args)...));
		}
	};
	
	
	template<typename ... ReturnTypes, typename F, typename ... Fs >
	Function<TList<ReturnTypes...>, TList<F, Fs...> > declareFunc(const std::string & name, const F & f, const Fs & ... fs) {
		return Function<TList<ReturnTypes...>, TList<F, Fs...> >(name, f, fs...);
	}
	
	template<typename ... ReturnTypes, typename F, typename ... Fs, typename = std::enable_if_t<!std::is_convertible<F, std::string>::value > >
	Function<TList<ReturnTypes...>, TList<F, Fs...> > declareFunc(const F & f, const Fs & ... fs) {
		return Function<TList<ReturnTypes...>, TList<F, Fs...> >("", f, fs...);
	}

	
	template<GLVersion version>
	struct ShaderWrapper
	{

		ShaderWrapper() {
			shader_ptr = std::make_shared<IShader<version>>();
			listen().currentShader = shader_ptr;
			listen().reset_counters();
		}

		template<typename F_Type>
		void main(const F_Type & f) {
			Function<TList<void>, TList<F_Type>>("main", f);
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

	//specialization of Fun when ReturnType == void
	//template<typename F_Type>
	//struct Func<void, F_Type> : FuncBase {
	//	template<typename ... Strings>
	//	Func(const std::string & _name, const F_Type  & _f, const Strings & ... _argnames) : FuncBase(_name), f(_f) {
	//		init_function_declaration<void>(NamedObjectBase::str(), functionFromLambda(_f), _argnames...);
	//	}
	//	template<typename ... R_Args, typename = std::result_of_t<F_Type(CleanType<R_Args>...)> >
	//	void operator()(R_Args &&  ... args) {
	//		checkArgsType<ArgTypeList<CleanType<R_Args>...> >(functionFromLambda(f));
	//		listen().addEvent(createFCallExp(NamedObjectBase::str(), EX(R_Args, args)...));
	//	}

	//	F_Type f;
	//};

	template<typename T, typename ... Args>
	Ex createInit(const stringPtr & name, CtorStatus status, uint ctor_flags, const Args &... args)
	{
		//std::cout << "ctor : " << *name << " " << (bool)(ctor_flags & PARENTHESIS) << std::endl;
		auto ctor = std::make_shared<Constructor<T, sizeof...(args)>>(name, status, ctor_flags, args...);
		Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
		listen().addEvent(expr);
		return expr;
	}


	template<typename T, typename ... Args>
	Ex createDeclaration(const stringPtr & name, uint flags, const Args &... args) {
		return createInit<T, Args...>(name, DECLARATION, flags, args...);
	}


	template<typename FList> struct CallFuncs;

	template<> struct CallFuncs<TList<>> {
		static void run() {}
	};

	template<typename F, typename ...Fs > 
	struct CallFuncs<TList<F, Fs...>> {
		static void run(const F & f, const Fs & ... fs ) {
			call_with_only_non_default_args(f);
			listen().next_overload();
			CallFuncs<TList<Fs...>>::run(fs...);
		}
	};

	template<typename ReturnTList, typename ... Fs>
	void init_function_declaration(const std::string & fname, const Fs & ...fs)
	{
		listen().begin_func<ReturnTList>(fname, fs...);
		CallFuncs<TList<Fs...>>::run(fs...);
		listen().end_func();
	}

	struct ReturnKeyword {
		ReturnKeyword() {
			listen().add_statement<ReturnStatement>();
		}
		template<typename T>
		ReturnKeyword(T && t) {
			listen().add_statement<ReturnStatement>(EX(T, t));
		}
	};

#define GL_RETURN ReturnKeyword csl_return_statement

#define GL_FOR(...) \
	listen().begin_for(); listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
	listen().begin_for_args(); __VA_ARGS__;  listen().begin_for_body(); \
	for(EndFor csl_dummy_for; csl_dummy_for; )


#define GL_IF(condition) \
	listen().check_begin_if(); listen().begin_if(condition); if(BeginIf csl_begin_if = {})

#define GL_ELSE \
	else {} listen().begin_else(); if(BeginElse csl_begin_else = {}) {} else 

#define GL_ELSE_IF(condition) \
	else if(false){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(BeginIf csl_begin_else_if = {})

#define GL_CONTINUE \
	listen().add_statement<ContinueStatement>();

#define GL_DISCARD \
	listen().add_statement<DiscardStatement>();

#define GL_BREAK \
	if(false){break;} listen().add_statement<BreakStatement>();

#define GL_WHILE(condition) \
	listen().begin_while(condition); for(BeginWhile csl_begin_while = {}; csl_begin_while; )

#define GL_SWITCH(condition) \
	listen().begin_switch(condition); switch(BeginSwitch csl_begin_switch = {})while(csl_begin_switch)

#define GL_CASE(value) \
	listen().begin_switch_case(value); case value 

#define GL_DEFAULT \
	listen().begin_switch_case(); default

	template<typename B, typename A, typename C, typename I = Infos<CT<A>>, typename = std::enable_if_t<
		EqualMat<B, Bool> && EqualMat<A, C>
		>> Matrix< I::scalar_type, I::rows, I::cols > csl_ternary(B && condition, A && lhs, C && rhs) {
		return { createExp<Ternary>(EX(B,condition), EX(A,lhs), EX(C,rhs)) };
	}

#define GL_TERNARY(...) csl_ternary( __VA_ARGS__ )

	//////////////////////////////////////

} //namespace csl
