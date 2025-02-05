#pragma once
#include <unordered_map>
#include <optional>
#include <exception>
#include <limits>

#include "node.hpp"

namespace executer {
    constexpr size_t DEFAULT_NAME_COUNT = 32; 
    namespace symTable {
        class SymbolTable final {
        public:
            void SetOrAddValue(std::string &name, int value) {
                varMap_[name] = value;
            }

            std::optional<int> GetValue(std::string &name) {
                auto hit = varMap_.find(name);
                if (hit != varMap_.end()) 
                    return varMap_[name];

                return std::nullopt;
            }

        private:
            std::unordered_map<std::string, int> varMap_;
        };

        class SymbolTables final {
        public:
            SymbolTables() {
                symbolTables_.reserve(DEFAULT_NAME_COUNT);
            }

            void SetValue(std::string &name, int value) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it) {
                    auto find = it->GetValue(name);
                    if (find.has_value()) {
                        it->SetOrAddValue(name, value);
                        return;
                    }
                }
                symbolTables_.back().SetOrAddValue(name, value);
            } 

            int GetValue(std::string &name) {
                for (auto it = symbolTables_.rbegin(), end = symbolTables_.rend(); it != end; ++it) {
                    auto find = it->GetValue(name);
                    if (find.has_value())
                        return *find;
                }

                std::string error_mes = "'";
                error_mes += name;
                error_mes += "' was not declared in this scope";
                throw std::runtime_error(error_mes);
            }

            void PushSymTable() {
                symbolTables_.push_back(SymbolTable{});
            }

            void PopSymTable() {
                symbolTables_.pop_back();
            }
        private:
            std::vector<SymbolTable> symbolTables_;
        };
    }; // namespace symTable

    class ExecuteVisitor final : public node::NodeVisitor {
    public:
        ExecuteVisitor(std::vector<std::string> &program_text) : program_text_(program_text) {}

        void visitBinOpNode(node::BinOpNode &node) override {
            node.left_->Accept(*this);
            int operand1 = int_param_;
            node.right_->Accept(*this);
            int operand2 = int_param_;

            switch (node.type_) {
                case node::BinOpNode_t::add:
                    int_param_ = operand1 + operand2;
                    return;
                case node::BinOpNode_t::sub:
                    int_param_ = operand1 - operand2;
                    return;
                case node::BinOpNode_t::mul:
                    int_param_ = operand1 * operand2;
                    return;
                case node::BinOpNode_t::div:
                    if (operand2 == 0) {
                        std::string msg = "Division by zero, at line #" +
                                          std::to_string(node.info_.num_line_) + "\n" +
                                          program_text_[node.info_.num_line_ - 1];
                        throw std::runtime_error(msg);
                    }
                    int_param_ = operand1 / operand2;
                    return;
            }
        }

        void visitBinCompOpNode(node::BinCompOpNode &node) override {
            node.left_->Accept(*this);
            int operand1 = int_param_;
            node.right_->Accept(*this);
            int operand2 = int_param_;

            switch (node.type_) {
                case node::BinCompOpNode_t::equal:
                    bool_param_ = (operand1 == operand2);
                    return;
                case node::BinCompOpNode_t::not_equal:
                    bool_param_ = (operand1 != operand2);
                    return;
                case node::BinCompOpNode_t::greater:
                    bool_param_ = (operand1 > operand2);
                    return;
                case node::BinCompOpNode_t::less:
                    bool_param_ = (operand1 < operand2);
                    return;
                case node::BinCompOpNode_t::greater_or_equal:
                    bool_param_ = (operand1 >= operand2);
                    return;
                case node::BinCompOpNode_t::less_or_equal:
                    bool_param_ = (operand1 <= operand2);
                    return;
            }
        }

        void visitNumberNode(node::NumberNode &node) override {
            int_param_ = node.number_;
        }

        void visitInputNode(node::InputNode &node) override {
            int input = 0;
            std::cin >> input;
            int_param_ = input;
        }

        void visitVarNode(node::VarNode &node) override {
            try
            {
                int_param_ = symbolTables_.GetValue(node.name_);
            }
            catch(std::runtime_error& err)
            {
                std::string msg = err.what();
                msg += ", at line #" + std::to_string(node.info_.num_line_) + "\n" +
                                       program_text_[node.info_.num_line_ - 1];
                throw std::runtime_error(msg);
            }
        }

        void visitScopeNode(node::ScopeNode &node) override {
            symbolTables_.PushSymTable();
            for (auto &statement : node.kids_) {
                if (statement != nullptr)
                    statement->Accept(*this);
            }
            symbolTables_.PopSymTable();
        }

        void visitDeclNode(node::DeclNode &node) override {
            string_param_ = node.name_;
        }

        void visitCondNode(node::CondNode &node) override {
            node.predicat_->Accept(*this);
            if (bool_param_) {
                node.first_->Accept(*this);
            } else {
                if (node.second_)
                    node.second_->Accept(*this);
            }
        }

        void visitLoopNode(node::LoopNode &node) override {
            node.predicat_->Accept(*this);
            while (bool_param_) {
                node.scope_->Accept(*this);
                node.predicat_->Accept(*this);
            }
        }

        void visitAssignNode(node::AssignNode &node) override {
            node.expr_->Accept(*this);
            int result = int_param_;
            node.var_->Accept(*this);
            auto name = string_param_;
            symbolTables_.SetValue(name, result);
        }

        void visitOutputNode(node::OutputNode &node) override {
            node.expr_->Accept(*this);
            int res = int_param_; 
            std::cout << res << std::endl;
        }

    private:
        bool bool_param_ = false;
        int int_param_ = 0;
        std::string string_param_;
        symTable::SymbolTables symbolTables_;
        std::vector<std::string> &program_text_;
    }; // class ExecuteVisitor
}