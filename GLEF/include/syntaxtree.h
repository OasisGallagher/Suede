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

class SUEDE_API SyntaxNode : private NonCopyable {
public:
	SyntaxNode(SyntaxNodeType type, const std::string& text);
	~SyntaxNode();

public:
	SyntaxNodeType GetNodeType() const;

	void AddChildren(SyntaxNode** buffer, int count);
	
	int GetChildCount() const;

	SyntaxNode* GetChildAt(uint index);
	const SyntaxNode* GetChildAt(uint index) const;

	uint GetCodeLineNumber() const;

	void SetSymbolAddress(Sym* addr);
	void SetLiteralAddress(Literal* addr);
	void SetCodeAddress(Code* addr, uint lineno);
	void SetIntegerAddress(Integer* addr);
	void SetSingleAddress(Single* addr);
	void SetBooleanAddress(Boolean* addr);

	const std::string& ToString() const;

private:
	std::string text_;
	SyntaxNodeType type_;

	union {
		Sym* symbol;
		struct {
			Code* ptr;
			uint lineno;
			uint linepos;
		} code;

		Literal* literal;
		Integer* integer;
		Single* single;
		Boolean* boolean;

		struct {
			SyntaxNode** ptr;
			uint count;
		} children;
	} value_;
};

class SUEDE_API SyntaxTree : private NonCopyable {
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
