#pragma once

#include <vector>

class SyntaxNode;
class TextScanner;

struct Argument {
	std::string text;
	std::vector<int> parameters;
};

class Action {
public:
	virtual ~Action();

public:
	void SetArgument(const Argument& argument);

public:
	virtual std::string ToString() const = 0;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container) = 0;
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);

private:
	bool SplitParameters(int* parameters, int& count, TextScanner& scanner);

protected:
	Argument argument_;
};

class ActionInteger : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionSingle : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionLiteral : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionCode :public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionSymbol : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionIndex : public Action {
public:
	virtual std::string ToString() const;
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);
};

class ActionCreate : public Action {
public:
	virtual std::string ToString() const;
	virtual bool ParseParameters(TextScanner& scanner, Argument& argument);

protected:
	virtual std::string GetName() const = 0;
};

class ActionMake : public ActionCreate {
public:
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);

protected:
	virtual std::string GetName() const { return "make"; }
};

class ActionMerge : public ActionMake {
public:
	virtual SyntaxNode* Invoke(const std::vector<void*>& container);

protected:
	virtual std::string GetName() const { return "merge"; }
};

class ActionParser {
public:
	static Action* Parse(const std::string& cmd);
	static void Destroy(Action* action);

private:
	static Action* CreateAction(const std::string& cmd);
	static bool IsOperand(const char* text);
};
