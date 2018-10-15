import os;
import sys;

sys.path.append("../../../Shared/generator");
import generator;

class Argument:
	def __init__(self, type, value):
		self.type = type;
		self.value = value;

class Method:
	def __init__(self, define):
		self._arguments = []
		self._initialize(define.strip("; \t\n"));

	def Name(self):
		return self._name;

	def Return(self):
		return self._r;

	def DefBody(self):
		answer = "";
		for i in range(len(self._arguments)):
			if i != 0: answer = answer + ", ";
			answer += self._arguments[i].type + " " + self._arguments[i].value;

		answer = self._name + "(" + answer + ")";
		return answer

	def Def(self):
		return self.Return() + " " + self.DefBody();

	def Body(self):
		return self._r + " " + self.DefBody();

	def Arguments(self):
		return self._arguments

	def _initialize(self, define):
		pointer = define.find("gl");
		self._r = define[:pointer].strip();

		pointer += 2;

		left = define.find("(");
		self._name = define[pointer: left].strip();

		left += 1;

		args = define[left: define.rfind(")")];
		if (args == ""): return;

		arglist = ("," not in args) and [ args ] or args.split(",");

		for arg in arglist:
			stripped = arg.strip();
			space = stripped.rfind(' ');
			self._arguments.append(Argument(stripped[:space], stripped[space + 1:]));

class Wrapper:
	def __init__(self, src):
		self._methods = sorted([Method(line) for line in src], key = lambda x: x.Name().lower());

	def Generate(self, dest):
		self._file = dest;
		
		self._AddHeaders();
		self._AddDefines();
		self._AddHelperMacros();
		self._AddImplementations();
		self._AddCleanup();

		self._file.close();
		return len(self._methods);

	def _AddHeaders(self):
		self._Line('''#pragma once
#include <gl/glew.h>

#include "debug/debug.h"
#include "tools/string.h"''');
		self._Line();

	def _AddDefines(self):
		self._Line('''class GL {
public:''');

		for method in self._methods:
			self._Line("\tstatic %s;" % method.Def());
		
		self._Line();
		self._Line("private:");

		self._Line("\tstatic void Verify(const char* func);");
		self._Line();

		self._Line('''\ttemplate <class T>
\tstatic T VerifyR(const char* func, T ans);
};''');
		self._Line();


	def _AddHelperMacros(self):
		self._Line('''#ifdef _DEBUG
#define GL_CALL(expression)		expression; Verify(__func__)
#define GL_CALL_R(expression)	return VerifyR(__func__, expression)
#else
#define GL_CALL(expression)		expression
#define GL_CALL_R(expression)	return expression
#endif''');

		self._Line();

		self._Line('''inline void GL::Verify(const char* func) {
#define CASE(errorEnum)		case errorEnum: message = #errorEnum; break
	GLenum error = glGetError();
	std::string message;
	switch (error) {
		case GL_NO_ERROR:
			break;
		CASE(GL_INVALID_ENUM);
		CASE(GL_INVALID_VALUE);
		CASE(GL_INVALID_OPERATION);
		CASE(GL_STACK_OVERFLOW);
		CASE(GL_STACK_UNDERFLOW);
		CASE(GL_OUT_OF_MEMORY);
		CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
		default: message = String::Format("undefined error 0x % x", error); break;
	}
#undef CASE

	if (!message.empty()) {
		Debug::LogError("%s: %s.", func, message.c_str());
	}
}''');

		self._Line();

		self._Line('''template <class T> inline
T GL::VerifyR(const char* func, T ans) {
	Verify(func);
	return ans;
}''');

		self._Line();

	def _AddImplementations(self):
		for method in self._methods:
			self._Line("inline %s GL::%s {" % (method.Return(), method.DefBody()));
			line = "\t" + ((method.Return() == "void") and "GL_CALL" or "GL_CALL_R") \
			    + "(gl" + method.Name() + "(";

			arguments = "";
			for i in range(len(method.Arguments())):
				if (i != 0): arguments = arguments + ", ";
				arguments += method.Arguments()[i].value;

			line += arguments;
			line += "));";

			self._Line(line);
			self._Line("}");
			self._Line();

	def _AddCleanup(self):
		self._Line("#undef GL_CALL");
		self._Line("#undef GL_CALL_R");

	def _Line(self, line = ""):
		self._file.write(line + "\n");

def Generate(src, dest):
	Wrapper(src).Generate(dest)

generator.Run(__file__, Generate);
