#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "compilerdefines.h"

enum SyntaxNodeType {
	SyntaxNodeSymbol,
	SyntaxNodeCode,
	SyntaxNodeLiteral,
	SyntaxNodeConstant,
	SyntaxNodeOperation,
};

class Sym {
public:
	Sym(const std::string& text);

public:
	std::string ToString() const;

private:
	std::string value_;
};

class Literal {
public:
	Literal(const std::string& text);

public:
	std::string ToString() const;

private:
	std::string value_;
};

class Code {
public:
	Code(const std::string& text);

public:
	std::string ToString() const;

private:
	std::string value_;
};

class Constant {
public:
	Constant(const std::string& text);

public:
	std::string ToString() const;

private:
	int value_;
};

class COMPILER_API SyntaxNode {
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
	void SetConstantAddress(Constant* addr);

	const std::string& ToString() const;
	const std::string& GetText() const { return text_; }
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
		Constant* constant;

		// 子节点, 第一个元素表示子节点个数.
		SyntaxNode** children;
	} value_;
};

class COMPILER_API SyntaxTree {
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
