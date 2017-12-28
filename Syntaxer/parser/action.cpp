#include "table.h"
#include "action.h"
#include "scanner.h"
#include "utilities.h"
#include "debug/debug.h"
#include "syntaxtree.h"
#include "tokendefine.h"
#include "compilerdefines.h"

void Action::SetArgument(const Argument& argument) {
	argument_ = argument;
}

Action::~Action() {

}

bool Action::ParseParameters(TextScanner& scanner, Argument& argument) {
	int count = COMPILER_MAX_PARSER_FUNCTION_PARAMTERS;
	int parameters[COMPILER_MAX_PARSER_FUNCTION_PARAMTERS];
	if (!SplitParameters(parameters, count, scanner)) {
		return false;
	}

	argument.parameters.insert(argument.parameters.end(), parameters, parameters + count);
	return true;
}

bool Action::SplitParameters(int* parameters, int& count, TextScanner& scanner) {
	std::string token;
	ScannerTokenType tokenType = ScannerTokenEndOfFile, expectedTokenType = ScannerTokenIdentifier;

	int index = 0;
	for (; (tokenType = scanner.GetToken(token)) != ScannerTokenRightParenthesis;) {
		if (tokenType == ScannerTokenLeftParenthesis) {
			continue;
		}

		if (tokenType != expectedTokenType) {
			Debug::LogError("invalid token type %d.", tokenType);
			return false;
		}

		if (tokenType == ScannerTokenComma) {
			expectedTokenType = ScannerTokenIdentifier;
		}
		else {
			int integer = 0;
			if (token.front() != '$' || !Utility::ParseInteger(token.c_str() + 1, &integer)){
				Debug::LogError("invalid parameter format %d.", token);
				return false;
			}
			
			if (index >= count) {
				Debug::LogError("buffer too small");
				return false;
			}

			parameters[index++] = integer;
			expectedTokenType = ScannerTokenComma;
		}
	}

	if ((tokenType = scanner.GetToken(token)) != ScannerTokenEndOfFile) {
		Debug::LogError("invalid tailing characters");
		return false;
	}

	count = index;

	return true;
}

std::string ActionInteger::ToString() const {
	return std::string("$$ = integer($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionInteger::Invoke(const std::vector<void*>& container) {
	Integer* integer = (Integer*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeInteger, integer->ToString());
	ans->SetIntegerAddress(integer);
	return ans;
}

std::string ActionLiteral::ToString() const {
	return std::string("$$ = literal($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionLiteral::Invoke(const std::vector<void*>& container) {
	Literal* literal = (Literal*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeLiteral, literal->ToString());
	ans->SetLiteralAddress(literal);
	return ans;
}

std::string ActionCode::ToString() const {
	return std::string("$$ = code($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionCode::Invoke(const std::vector<void*>& container) {
	Code* code = (Code*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeCode, code->ToString());
	ans->SetCodeAddress(code);
	return ans;
}

std::string ActionSymbol::ToString() const {
	return std::string("$$ = symbol($") + std::to_string(argument_.parameters.front()) + ")";
}

SyntaxNode* ActionSymbol::Invoke(const std::vector<void*>& container) {
	Sym* sym = (Sym*)container[container.size() - argument_.parameters.front()];
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeSymbol, sym->ToString());
	ans->SetSymbolAddress(sym);
	return ans;
}

std::string ActionIndex::ToString() const {
	return std::string("$$ = $") + std::to_string(argument_.parameters.front());
}

SyntaxNode* ActionIndex::Invoke(const std::vector<void*>& container) {
	int index = argument_.parameters.front();
	if (index == 0) {
		return nullptr;
	}

	return (SyntaxNode*)container[container.size() - argument_.parameters.front()];
}

std::string ActionMake::ToString() const {
	std::ostringstream oss;
	
	oss << "$$ = make(\"" + argument_.text + "\"";

	for (std::vector<int>::const_iterator ite = argument_.parameters.begin();
		ite != argument_.parameters.end(); ++ite) {
		oss << ", ";
		oss << '$' << std::to_string(*ite);
	}

	oss << ")";

	return oss.str();
}

SyntaxNode* ActionMake::Invoke(const std::vector<void*>& container) {
	SyntaxNode* ans = new SyntaxNode(SyntaxNodeOperation, argument_.text);
	SyntaxNode** nodes = new SyntaxNode*[argument_.parameters.size()];
	for (int i = 0; i < (int)argument_.parameters.size(); ++i) {
		if (argument_.parameters[i] == 0) {
			nodes[i] = nullptr;
		}
		else {
			nodes[i] = (SyntaxNode*)container[container.size() - argument_.parameters[i]];
		}
	}

	ans->AddChildren(nodes, argument_.parameters.size());
	delete[] nodes;

	return ans;
}

bool ActionMake::ParseParameters(TextScanner& scanner, Argument& argument) {
	std::string token;
	ScannerTokenType tokenType = scanner.GetToken(token);
	if (tokenType != ScannerTokenLeftParenthesis) {
		Debug::LogError("invalid parameter");
		return false;
	}

	if ((tokenType = scanner.GetToken(token)) != ScannerTokenString || token.empty()) {
		Debug::LogError("invalid action name");
		return false;
	}

	argument.text = token;

	tokenType = scanner.GetToken(token);
	if (tokenType == ScannerTokenRightParenthesis) {
		return true;
	}

	if (tokenType != ScannerTokenComma) {
		Debug::LogError("invalid parameter, lhs = %s.", argument.text.c_str());
		return false;
	}

	return Action::ParseParameters(scanner, argument);
}

Action* ActionParser::Parse(const std::string& cmd) {
	if (cmd.empty()) {
		return nullptr;
	}

	return CreateAction(cmd);
}

void ActionParser::Destroy(Action* action) {
	delete action;
}

Action* ActionParser::CreateAction(const std::string& cmd) {
	TextScanner scanner;
	scanner.SetText(cmd.c_str());

	std::string token;
	ScannerTokenType tokenType = scanner.GetToken(token);

	if (tokenType == ScannerTokenEndOfFile) {
		Debug::LogError("empty command");
		return nullptr;
	}

	if (tokenType != ScannerTokenIdentifier || token != "$$") {
		Debug::LogError("invalid left hand side operand: %s.", token);
		return false;
	}

	if ((tokenType = scanner.GetToken(token)) != ScannerTokenAssign) {
		Debug::LogError("missing '='");
		return nullptr;
	}

	if ((tokenType = scanner.GetToken(token)) != ScannerTokenIdentifier) {
		Debug::LogError("invalid command");
		return nullptr;
	}

	Action* action = nullptr;
	Argument argument;

	if (IsOperand(token.c_str())) {
		int integer = 0;
		if (!Utility::ParseInteger(token.c_str() + 1, &integer)) {
			Debug::LogError("invalid right operand %s.", token);
			return nullptr;
		}

		action = new ActionIndex();
		argument.text = "identity";
		argument.parameters.push_back(integer);
	}
	else {
		if (token == "make") {
			action = new ActionMake();
			argument.text = "make";
		}
		else if (token == "integer") {
			action = new ActionInteger();
			argument.text = "integer";
		}
		else if (token == "symbol") {
			action = new ActionSymbol();
			argument.text = "symbol";
		}
		else if (token == "literal") {
			action = new ActionLiteral();
			argument.text = "literal";
		}
		else if (token == "code") {
			action = new ActionCode();
			argument.text = "code";
		}

		if (action != nullptr && !action->ParseParameters(scanner, argument)) {
			delete action;
			action = nullptr;
		}
	}

	if (action == nullptr) {
		Debug::LogError("invalid action %s.", token);
		return nullptr;
	}

	action->SetArgument(argument);
	return action;
}

bool ActionParser::IsOperand(const char* text) {
	if (strlen(text) < 2) {
		return false;
	}

	if (*text != '$') {
		return false;
	}

	++text;
	if (*text != '$' && !Utility::ParseInteger(text, nullptr)) {
		return false;
	}

	return true;
}
