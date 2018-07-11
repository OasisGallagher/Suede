#pragma once
#include <string>
#include <vector>
#include "glefdefines.h"
#include "tools/noncopyable.h"

enum SyntaxNodeType {
	SyntaxNodeSymbol,
	SyntaxNodeCode,
	SyntaxNodeLiteral,
	SyntaxNodeInteger,
	SyntaxNodeSingle,
	SyntaxNodeBoolean,
	SyntaxNodeOperation,
};

class Sym;
class Code;
class Literal;
class Integer;
class Single;
class Boolean;

class SUEDE_API SyntaxNode : public NonCopyable {
public:
	SyntaxNode(SyntaxNodeType type, const std::string& text);
	~SyntaxNode();

public:
	SyntaxNodeType GetNodeType() const;

	void AddChildren(SyntaxNode** buffer, int count);
	
	int GetChildCount() const;

	SyntaxNode* GetChildAt(uint index);
	const SyntaxNode* GetChildAt(uint index) const;

	void SetSymbolAddress(Sym* addr);
	void SetLiteralAddress(Literal* addr);
	void SetCodeAddress(Code* addr);
	void SetIntegerAddress(Integer* addr);
	void SetSingleAddress(Single* addr);
	void SetBooleanAddress(Boolean* addr);

	const std::string& ToString() const;

private:
	std::string text_;
	SyntaxNodeType type_;

	union {
		Sym* symbol;
		Code* code;
		Literal* literal;
		Integer* integer;
		Single* single;
		Boolean* boolean;

		// 子节点, 第一个元素表示子节点个数.
		SyntaxNode** children;
	} value_;
};

class SUEDE_API SyntaxTree : public NonCopyable {
public:
	SyntaxTree();
	~SyntaxTree();

public:
	void SetRoot(SyntaxNode* root);
	SyntaxNode* GetRoot() { return root_; }

	void Destroy();

public:
	std::string ToString() const;

private:
	void ToStringRecursively(std::ostringstream& oss, const std::string& prefix, const SyntaxNode* current, bool tail) const;
	void DeleteTreeNode(SyntaxNode* node);

	typedef void (SyntaxTree::*TreeWalkCallback)(SyntaxNode* node);
	void PreorderTreeWalk(TreeWalkCallback callback);

private:
	SyntaxNode* root_;
};
