#pragma once

#include <set>

#include "InstructionTree.hpp"
#include "NamedObjects.hpp"

namespace csl {

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

		virtual void pushInstruction(const InstructionBase::Ptr & i) {}

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
			pushInstruction(current_for);
			//currentBlock->push_instruction(current_for);
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
			pushInstruction(current_if);
			//currentBlock->push_instruction(current_if);
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
			pushInstruction(while_instruction);
			//currentBlock->push_instruction(while_instruction);
			currentBlock = while_instruction->body;
		}

		virtual void end_while() {
			currentBlock = currentBlock->parent;
		}
	};

	struct SwitchController : virtual ControllerBase {
		void begin_switch(const Ex & ex) {
			current_switch = std::make_shared<SwitchInstruction>(ex, currentBlock, current_switch);
			pushInstruction(current_switch);
			//currentBlock->push_instruction(current_switch);
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

		virtual void end_while() override {
			check_end_if();
			WhileController::end_while();
		}

		virtual void add_case(const Ex & ex) override {
			check_end_if();
			SwitchController::add_case(ex);
		}

		virtual void end_switch() override {
			check_end_if();
			SwitchController::end_switch();
		}

		void handleEvent(const Ex & e) {
			//std::cout << "e : " << e->str(0) << std::endl;		
			check_end_if();
			checkFuncArgs();
			queueEvent(e);
		}

		virtual void pushInstruction(const InstructionBase::Ptr & i) override {
			check_end_if();
			checkFuncArgs();
			currentBlock->push_instruction(i);
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

			pushInstruction(statement);
			//currentBlock->push_instruction(statement);
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

}