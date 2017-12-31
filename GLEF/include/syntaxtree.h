#pragma once
#include <string>
#include <vector>
#include "glefdefines.h"

enum SyntaxNodeType {
	SyntaxNodeSymbol,
	SyntaxNodeCode,
	SyntaxNodeLiteral,
	SyntaxNodeInteger,
	SyntaxNodeOperation,
};

class Sym;
class Code;
class Literal;
class Integer;

class GLEF_API SyntaxNode {
public:
	SyntaxNode(SyntaxNodeType type, const std::string& text);
	~SyntaxNode();

public:
	SyntaxNodeType GetNodeType() const;

	void AddChildren(SyntaxNode** buffer, int count);
	
	int GetChildCount() const;

	SyntaxNode* GetChild(int index);
	const SyntaxNode* GetChild(int index) const;

	void SetSymbolAddress(Sym* addr);
	void SetLiteralAddress(Literal* addr);
	void SetCodeAddress(Code* addr);
	void SetIntegerAddress(Integer* addr);

	const std::string& ToString() const;

private:
	SyntaxNode(const SyntaxNode& other);
	SyntaxNode& operator = (const SyntaxNode& other);

private:
	std::string text_;
	SyntaxNodeType type_;

	union {
		Sym* symbol;
		Code* code;
		Literal* literal;
		Integer* integer;

		// 子节点, 第一个元素表示子节点个数.
		SyntaxNode** children;
	} value_;
};

class GLEF_API SyntaxTree {
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
	SyntaxTree(const SyntaxTree&);
	SyntaxTree& operator = (const SyntaxTree&);

	void ToStringRecursively(std::ostringstream& oss, const std::string& prefix, const SyntaxNode* current, bool tail) const;
	void DeleteTreeNode(SyntaxNode* node);

	typedef void (SyntaxTree::*TreeWalkCallback)(SyntaxNode* node);
	void PreorderTreeWalk(TreeWalkCallback callback);

private:
	SyntaxNode* root_;
};
