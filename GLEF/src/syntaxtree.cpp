#include <stack>
#include <sstream>
#include "syntaxtree.h"
#include "debug/debug.h"

inline bool DebugNodeType(SyntaxNodeType current, SyntaxNodeType expected) {
#ifdef _DEBUG
	if (current != expected) {
		Debug::LogError("invalid syntax node type: %d != %d.", current, expected); return false;
	}
#endif

	return true;
}

SyntaxNode::SyntaxNode(SyntaxNodeType type, const std::string& text)
	: text_(text), type_(type) {
	memset(&value_, 0, sizeof(value_));
}

SyntaxNode::~SyntaxNode() {
	if (type_ == SyntaxNodeOperation) {
		delete[] value_.children;
	}
}

SyntaxNodeType SyntaxNode::GetNodeType() const {
	return type_;
}

void SyntaxNode::AddChildren(SyntaxNode** buffer, int count) {
	if (!DebugNodeType(type_, SyntaxNodeOperation)) {
		return;
	}

	value_.children = new SyntaxNode*[count + 1];
	*value_.children = (SyntaxNode*)(uint64)count;

	for (int i = 1; i <= count; ++i) {
		value_.children[i] = buffer[i - 1];
	}
}

SyntaxNode* SyntaxNode::GetChild(int index) {
	if (!DebugNodeType(type_, SyntaxNodeOperation)) {
		return nullptr;
	}

	if (index < 0 || index >= GetChildCount()) {
		Debug::LogError("index out of range");
		return nullptr;
	}

	return value_.children[index + 1];
}

const SyntaxNode* SyntaxNode::GetChild(int index) const {
	if (!DebugNodeType(type_, SyntaxNodeOperation)) {
		return nullptr;
	}

	if (index < 0 || index >= GetChildCount()) {
		Debug::LogError("index out of range");
		return nullptr;
	}

	return value_.children[index + 1];
}

int SyntaxNode::GetChildCount() const {
	if (!DebugNodeType(type_, SyntaxNodeOperation)) {
		return 0;
	}

	return (int)(uint64)*value_.children;
}

void SyntaxNode::SetIntegerAddress(Integer* addr) {
	if (DebugNodeType(type_, SyntaxNodeInteger)) {
		value_.integer = addr;
	}
}

void SyntaxNode::SetSingleAddress(Single* addr) {
	if (DebugNodeType(type_, SyntaxNodeSingle)) {
		value_.single = addr;
	}
}

void SyntaxNode::SetBooleanAddress(Boolean* addr) {
	if (DebugNodeType(type_, SyntaxNodeBoolean)) {
		value_.boolean = addr;
	}
}

void SyntaxNode::SetSymbolAddress(Sym* addr) {
	if (DebugNodeType(type_, SyntaxNodeSymbol)) {
		value_.symbol = addr;
	}
}

void SyntaxNode::SetLiteralAddress(Literal* addr) {
	if (DebugNodeType(type_, SyntaxNodeLiteral)) {
		value_.literal = addr;
	}
}

void SyntaxNode::SetCodeAddress(Code* addr) {
	if ((type_, SyntaxNodeCode)) {
		value_.code = addr;
	}
}

const std::string& SyntaxNode::ToString() const {
	return text_;
}

SyntaxTree::SyntaxTree() 
	: root_(nullptr) {
}

SyntaxTree::~SyntaxTree() {
	Destroy();
}

void SyntaxTree::SetRoot(SyntaxNode* root) {
	root_ = root;
}

void SyntaxTree::Destroy() {
	PreorderTreeWalk(&SyntaxTree::DeleteTreeNode);
	root_ = nullptr;
}

std::string SyntaxTree::ToString() const {
	std::ostringstream oss;
	if (root_ != nullptr) {
		ToStringRecursively(oss, "", root_, true);
	}
	return oss.str();
}

void SyntaxTree::ToStringRecursively(std::ostringstream& oss, const std::string& prefix, const SyntaxNode* current, bool tail) const {
	oss << prefix << (tail ? "└─── " : "├─── ") << (current != nullptr ? current->ToString() : "null") << "\n";
	if (current == nullptr || current->GetNodeType() != SyntaxNodeOperation) {
		return;
	}

	for (int i = 0; i < current->GetChildCount(); ++i) {
		bool lastChild = current != nullptr && current->GetChildCount() == (i + 1);
		ToStringRecursively(oss, prefix + (tail ? "     " : "│    "), current->GetChild(i), lastChild);
	}

	/*
	bool tail = false;

	oss << prefix << (tail ? "└─── " : "├─── ") << (current != nullptr ? current->ToString() : "null") << "\n";
	if (current == nullptr || current->GetNodeType() != SyntaxNodeOperation) {
		return;
	}

	for (int i = 0; i < current->GetChildCount(); ++i) {
		SyntaxNodeToString(oss, prefix + (tail ? "     " : "│    "), current, current->GetChild(i));
	}
	*/
}

void SyntaxTree::DeleteTreeNode(SyntaxNode* node) {
	delete node;
}

void SyntaxTree::PreorderTreeWalk(TreeWalkCallback callback) {
	if (root_ == nullptr) {
		return;
	}

	std::stack<SyntaxNode*> s;
	s.push(root_);

	for (; !s.empty();) {
		SyntaxNode* cur = s.top();
		s.pop();

		if (cur != nullptr && cur->GetNodeType() == SyntaxNodeOperation) {
			for (int i = cur->GetChildCount() - 1; i >= 0; --i) {
				s.push(cur->GetChild(i));
			}
		}

		if (cur != nullptr) {
			(this->*callback)(cur);
		}
	}
}
