#pragma once

#include <vector>
#include <iostream>

#include "Operators.hpp"

namespace csl {

	namespace core {

		//////////////

		enum ArgOrderEvaluation { LEFT_TO_RIGHT, RIGHT_TO_LEFT, ORDER_NOT_SUPPORTED };

		struct ArgOrdering {
			ArgOrdering(size_t i) : value(i) {
				static size_t counter = 0;
				counter_value = counter;
				++counter;
			}

			static ArgOrderEvaluation call_args(ArgOrdering a, ArgOrdering b, ArgOrdering c) {
				ArgOrderEvaluation out;
				if (a.check(a) && b.check(b) && c.check(c)) {
					out = LEFT_TO_RIGHT;
				} else  if (a.check(c) && b.check(b) && c.check(a)) {
					out = RIGHT_TO_LEFT;
				} else {
					out = ORDER_NOT_SUPPORTED;
				}
				return out;
			}

			bool check(const ArgOrdering & other) const { return value == other.counter_value; }

			size_t value, counter_value;

		};

		inline ArgOrderEvaluation getArgOrder() {
			static bool first = true;
			static ArgOrderEvaluation order;
			if (first) {
				order = ArgOrdering::call_args(ArgOrdering(0), ArgOrdering(1), ArgOrdering(2));
				first = false;
			}
			return order;
		}

		inline void getArgOrderStr(ArgOrderEvaluation order) {
			std::string out;
			if (order == LEFT_TO_RIGHT) {
				out = "left to right";
			} else if (order == RIGHT_TO_LEFT) {
				out = "right to left";
			} else {
				out = "compiler's arg order is not supported";
			}
			std::cout << out << std::endl;
		}

		///////////

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
				if (!(opts & IGNORE_TRAILING)) {
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
										ctor->flags |= MULTIPLE_INITS;
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

			void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON | NEW_LINE) {
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
			void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON | NEW_LINE) {
				stream << instruction_begin(trailing, opts) << "continue" << instruction_end(opts);
			}
		};

		struct DiscardStatement : SpecialStatement {
			void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON | NEW_LINE) {
				stream << instruction_begin(trailing, opts) << "discard" << instruction_end(opts);
			}
		};

		struct BreakStatement : SpecialStatement {
			void str(std::stringstream & stream, int & trailing, uint opts = SEMICOLON | NEW_LINE) {
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

				int dummy_trailing = 0;
				//{
				//	std::stringstream ss;
				//	std::cout << "/////////////";
				//	for (const auto & i : funcs[it].args->instructions) {
				//		ss << "\n\t";
				//		i->str(ss, dummy_trailing, 0);
				//	}
				//	std::cout << ss.str() << "\n /////////////////" << std::endl;
				//}

				stream << InstructionBase::instruction_begin(trailing, opts) <<
					getTypeStr<typename ReturnTList::template GetType<it>>() << " " << fname << "(";

				const auto & args = funcs[it].args->instructions;
				const int size = static_cast<int>(args.size());
				if (getArgOrder() == LEFT_TO_RIGHT) {
					for (int i = 0; i < size; ++i) {
						args[i]->str(stream, dummy_trailing, (i == (size - 1) ? 0 : COMMA | ADD_SPACE));
					}
				} else if (getArgOrder() == RIGHT_TO_LEFT) {
					for (int i = size - 1; i >= 0; --i) {
						args[i]->str(stream, dummy_trailing, (i == 0 ? 0 : COMMA | ADD_SPACE));
					}
				} else {
					getArgOrderStr(getArgOrder());
					throw;
				}

				stream << ") {\n";
				++trailing;

				funcs[it].body->str(stream, trailing, opts);

				--trailing;
				stream << InstructionBase::instruction_begin(trailing, opts) << "}\n\n";

				FDeclImpl<ReturnTList, it + 1, Fs...>::str(stream, trailing, opts, fname, funcs);

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

	}
}