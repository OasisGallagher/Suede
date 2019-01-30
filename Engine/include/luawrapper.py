# -*- coding: utf-8 -*-

#
# TODO: better way to handle indent. count "{" and "}" for example.
#

import os;
import re;
import sys;
import traceback;
from stat import S_IREAD, S_IRGRP, S_IROTH, S_IWRITE;

RunAsExternal = False;

kFilePostfix = "_wrapper";
kClassPostfix = "_Wrapper";
kDestFolder = "../internal/lua/wrappers/";
kArray = "[]";
kPyCStr = "py_cstr";
kEnumerable = "Enumerable";
kLuaFunction = "Lua::Func";
kGenericContainer = "std::vector";

kEnumPattern = re.compile(r"enum\s+class\s+([a-zA-Z0-9_]+)");
kBetterEnumPattern = re.compile(r"BETTER_ENUM\(([a-zA-Z0-9_]+)");
kClassPattern = re.compile(r"\s*(struct|class)\s+([A-Z_]+\s+)?\s*([A-Za-z0-9_]+)\s*\:?\s*(public|private)?\s*([A-Za-z0-9_:<>]*)\s*\{");
kMethodPattern = re.compile(r"\s*(static|virtual)?\s*((?!return)[A-Za-z0-9:<>_\*]*?)\s*([A-Za-z0-9]+)\s*\((.*?)\)\s*(?:const)?\s*(=\s*0)?[;\{]\s*");

kExcludeFiles = [
	"gui.h",
	"imgui.h",
	"lua++.h",
	"variant.h",
	"gizmospainter.h",
	"graphicsviewer.h",
	"graphicscanvas.h",

	"profiler.h",
	"opengldriver.h",
	"geometryutility.h",
];

kExcludeParamters = [
	"WorldEventBase",
];

kExcludeClasses = [
	"InputInternal",
	
	"WorldEventBase",
	"GameObjectEvent",
	"ComponentEvent",
	"GameObjectCreatedEvent",
	"GameObjectDestroyedEvent",
	"GameObjectParentChangedEvent",
	"GameObjectActiveChangedEvent",
	"GameObjectTagChangedEvent",
	"GameObjectNameChangedEvent",
	"GameObjectUpdateStrategyChangedEvent",
	"GameObjectComponentChangedEvent",
	"CameraDepthChangedEvent",
	"GameObjectTransformChangedEvent",

	"WorldEventListener",
	"FrameEventListener",
	"GameObjectImportedListener",
	"ScreenSizeChangedListener",
	"FontMaterialRebuiltListener",
	"WorldGameObjectWalker",
];

kNotNewables = [
	"CameraUtility",
	"ComponentUtility",
	"GeometryUtility",
	"RenderTextureUtility",
	
	"Engine",
	"Environment",
	"Gizmos",
	"Graphics",
	"Input",
	"Profiler",
	"Resources",
	"Physics",
	"Screen",
	"Statistics",
	"TagManager",
	"Geometries",
	"Time",
	"World",

	"IObject",
	"IComponent",
	"IBehaviour",
	#"ITransform",
	"IRenderer",
	#"IRigidbody",
	#"IProjector",
	#"ILight",
	#"ICamera",
	#"ITextMesh",
	"IMeshProvider",
	#"IMeshFilter",
	#"IMeshRenderer",
	#"ISkinnedMeshRenderer",
	#"IParticleRenderer",
	#"IAnimation",
	#"IParticleSystem",
	"ITexture",
	"IParticleEmitter",
];

def Msg(message):
	if not RunAsExternal: print(message);

def Warning(message):
	if not RunAsExternal:
		color = '\033[91m'; endc = '\033[0m';
		print(color + message + endc);

def Success(message):
	if RunAsExternal:
		print("-" * 5 + " " + message + " " + "-" * 5);
	else:
		color = '\033[92m'; endc = '\033[0m';
		print((color + message + endc).center(os.get_terminal_size().columns, "-"));
	
def Chmod(file, mode):
	try:
		os.chmod(file, mode);
	except Exception as e:
		traceback.print_stack();

def WrapperFileName(name):
	p = name.rfind(".");
	return name[:p] + kFilePostfix + name[p:];

class Argument:
	def __init__(self, type, value, optional):
		self.type = type;
		self.value = value;
		self.optional = optional;

class Method:	# void Print(const char* message) for example.
	def __init__(self, text, groups):
		self._text = text.strip();
		self._text = self._text[:self._text.rfind(")") + 1];

		self._arguments = [];
		self._pureVirtual = False;
		self._initialize(groups);

	def Name(self):
		'''Print'''
		return self._name;

	def Text(self):
		return self._text;

	def Return(self):
		'''void'''
		return self._r;
		
	def IsConstructor(self):
		return not self._r;

	def DefBody(self):
		'''Print(const char* message)'''
		answer = "";
		for i in range(len(self._arguments)):
			if i != 0: answer = answer + ", ";
			answer += self._arguments[i].type + " " + self._arguments[i].value;

		answer = self._name + "(" + answer + ")";
		return answer

	def Def(self):
		'''void Print(const char* message)'''
		return self.Return() + " " + self.DefBody();

	def IsStatic(self):
		return self._static;

	def IsPureVirtual(self):
		return self._pureVirtual;

	def Arguments(self):
		return self._arguments

	def _split(self, arg):
		equals = arg.find("=");
		if equals >= 0: arg = arg[:equals];

		arg = arg.replace("const char*", kPyCStr);

		# remove & and const.
		if kLuaFunction not in arg:
			arg = arg.replace("&", "").replace("const ", "").strip();

		space = arg.rfind(' ');

		type = arg[:space];
		value = arg[space + 1:];

		type = self._parseGenericContainer(type);

		# is array.
		if "[" in value:
			type = kArray + "#" + type;
			value = value[:value.find("[")];

		return (type, value, equals >= 0);

	def _parseGenericContainer(self, type):
		if kGenericContainer in type:
			type = kGenericContainer + "#" + type[type.find("<") + 1:type.rfind(">")];
		return type;

	def _initialize(self, groups):
		self._r = groups[1];
		self._r = self._parseGenericContainer(self._r);

		self._static = groups[0] and "static" in groups[0];

		self._name = groups[2];
		self._pureVirtual = groups[4] != None;

		args = groups[3];
		if args == "": return;

		arg = "";
		inTemplate = False;
		for i in range(len(args)):
			if args[i] == "," and not inTemplate:
				fields = self._split(arg);
				self._arguments.append(Argument(fields[0], fields[1], fields[2]));
				arg = "";
			else:
				if args[i] == "<": inTemplate = True;
				if args[i] == ">": inTemplate = False;
				if inTemplate or args[i] != ",": arg += args[i];

		if arg:
			fields = self._split(arg);
			self._arguments.append(Argument(fields[0], fields[1], fields[2]));

class Interface:
	def __init__(self, className, defination, struct):
		self._methods = [ ];
		self._abstract = False;
		self._hasStatic = False;

		self._nmethods = 0;
		self._overloads = {};
		self._notNewable = (className in kNotNewables);

		public = struct;
		prev = "";

		for line in defination:
			if line.startswith("public:") or line.startswith("protected:") or line.startswith("private:"):
				public = line.startswith("public:");

			m = public and kMethodPattern.match(line);

			if m: self._parseMethod(line, m, prev);
			prev = line;

	def Methods(self):
		return self._methods;

	def MethodCount(self):
		''' count of methods, includes overloads'''
		return self._nmethods;

	def Abstract(self):
		return self._abstract;

	def HasStatic(self):
		return self._hasStatic;

	def IsNotNewable(self):
		return self._notNewable;

	def Overloads(self, methodName):
		return self._overloads.get(methodName) or [];

	def _parseMethod(self, line, m, prev):
		method = Method(line, m.groups());
		self._abstract = self._abstract or method.IsPureVirtual();
		self._hasStatic = self._hasStatic or method.IsStatic();
		msg = self._methodWrapable(prev, method);
		if not msg:
			list = self._overloads.get(method.Name(), None);
			if not list: self._overloads[method.Name()] = list = [];
			list.append(method);

			self._nmethods += 1;
			if len(list) == 1: self._methods.append(method);
		else:
			Warning("  Skip method(%s): %s" % (msg, method.Text()));

	def _methodWrapable(self, prev, method):
		if "template" in prev: return "template method";
		if "*" in method.Return(): return "c style pointer";
		if "operator" in method.Text(): return "operator overload";
		if method.IsPureVirtual(): return "pure virtual";
		
		for arg in method.Arguments():
			if "*" in arg.type: return "c style pointer";
			if arg.type in kExcludeParamters: return "invalid parameter";

		return "";

class Wrapper:
	def __init__(self): self._f = None;
	def _w(self, line): self._f.write(line);
	def _ws(self, lines): self._f.writelines(lines);

	def WrapClasses(self, name, classes):
		wrappers = [];
		dest = WrapperFileName(name);
		
		if os.path.isfile(kDestFolder + dest): Chmod(kDestFolder + dest, S_IWRITE);
		
		self._f = open(kDestFolder + dest, "w", encoding = "utf-8");
		Msg('Parsing "%s", write to "%s"...' % (name, dest));

		# common headers.
		self._w("// Warning: this file is generated by wrapper.py.\n\n");
		self._w("#pragma once\n\n");
		self._w('#include "' + name + '"\n\n');

		self._w('#include "lua++.h"\n');
		self._w('#include "tools/string.h"\n');

		for c in classes:
			wrappers.append(self._wrapClass(c[0], c[1], c[2], c[3], c[4]));

		self._f.close();
		Chmod(kDestFolder + dest, S_IREAD | S_IRGRP | S_IROTH);
		
		return wrappers;

	def _wrapClassName(self, className):
		sharedPtr = className[0] == "I" and className[1].isupper();
		if sharedPtr: className = className[1:];
		return (sharedPtr, className)
		
	def _argumentList(self, method, maxArgs):
		ans = "";
		for i in range(min(maxArgs, len(method.Arguments()))):
			if i != 0: ans += ", ";
			arg = method.Arguments()[i];
			ans += (arg.value);
			if kArray in arg.type:
				ans += ".data()";
			elif arg.type == kPyCStr:
				ans += ".c_str()";
		
		return ans;

	def _callMethod(self, className, method, sharedPtr, maxArgs):
		ans = "";
		if method.IsStatic():
			ans = "%s::%s(" % (sharedPtr and "I" + className or className, method.Name());
		elif not method.IsConstructor():
			ans = "_p->%s(" % method.Name();

		ans += self._argumentList(method, maxArgs);
		
		ans += ")";
		return ans;

	def _writeConstructor(self, className, instance, sharedPtr, interface):
		if instance:
			self._w(
'''	static int %sInstance(lua_State* L) {
		return Lua::reference<%s>(L);
	}\n'''  % (className, className));
		elif sharedPtr:
			self._w(
'''	static int New%s(lua_State* L) {
		return Lua::fromIntrusive(L, new I%s());
	}\n''' % (className, className));
		else:
			self._beginOverloadConstructor(className);
			self._writeOverloadConstructors(className, interface);
			self._endOverloadConstructor();
			
	def _formatExactArguments(self, method, argstart, maxArgs):
		code = [];
		pps = nargs = min(maxArgs, len(method.Arguments()));
		for i in range(nargs):
			argpos = nargs - i + 1 + (argstart - 2);
			argument = method.Arguments()[nargs - i - 1];
			if (kArray in argument.type) or (kGenericContainer in argument.type):
				key = (kArray in argument.type) and kArray or kGenericContainer;
				element = argument.type[len(key)+1:];
				code.append("std::vector<%s> %s = Lua::getList<%s>(L, %d);\n" % (element, argument.value, element, argpos));
			elif kPyCStr in argument.type:
				code.append("std::string %s = Lua::get<std::string>(L, %d);\n" % (argument.value, argpos));
			elif kLuaFunction in argument.type:
				type = argument.type[argument.type.find("<") + 1:argument.type.rfind(">")];
				if pps - (nargs - i) > 0:
					code.append("lua_pop(L, %d);	// ensure function at top of stack.\n" % (pps - (nargs - i)));

				code.append("auto %s = lua_isnil(L, -1) ? nullptr : Lua::make_func<%s>(L);\n" % (argument.value, type));
				pps = nargs - i - 1;
			else:
				code.append("%s %s = Lua::get<%s>(L, %d);\n" % (argument.type, argument.value, argument.type, argpos));
			
		return code;
		
	def _formatMethodCall(self, method, className, sharedPtr, argstart, maxArgs = float("inf"), ntabs = 2):
		code = self._formatExactArguments(method, argstart, maxArgs);
		call = self._callMethod(className, method, sharedPtr, maxArgs);
		if min(maxArgs, len(method.Arguments())) > 0: code.append("\n");
		
		if method.Return() == "void":
			code.append(call + ";\n");
			code.append("return 0;\n");
		elif kGenericContainer in method.Return():
			code.append("return Lua::pushList(L, %s);\n" % (call));
		elif kEnumerable in method.Return():
			etype = "I%s::%s" % (className, method.Return());
			code.append("%s _r = %s;\n" % (etype, call));
			code.append("return Lua::pushList(L, std::vector<%s::value_type>(_r.begin(), _r.end()));\n" % etype);
		else:
			code.append("return Lua::push(L, %s);\n" % (call));

		for i in range(len(code)):
			if code[i]: code[i] = ntabs * "\t" + code[i];

		return code;
		
	def _beginOverloadConstructor(self, className):
		self._w(
'''	static int New%s(lua_State* L) {\n''' % className);

	def _endOverloadConstructor(self):
		self._w(
'''	}\n''');
		
	def _formatOverloadConstructorCall(self, method, className, indent, maxArgs = float("inf")):
		code = [];
		arglist = None;
		
		if method:
			code.extend(["\t" + t for t in self._formatExactArguments(method, 1, maxArgs)]);
			arglist = self._argumentList(method, maxArgs);
			if min(maxArgs, len(method.Arguments())) > 0: code.append("\n");
		
		indent = indent and "\t" or "";
		if arglist:
			code.append(
'''%sreturn Lua::newObject<%s>(L, %s);\n''' % (indent, className, arglist));
		else:
			code.append(
'''%sreturn Lua::newObject<%s>(L);\n''' % (indent, className));

		for i in range(len(code)):
			if code[i]: code[i] = 2 * "\t" + code[i];

		return code;

	def _beginCaller(self, static, methodName, className, instance, sharedPtr):
		if static:
			self._w(
'''	static int %s(lua_State* L) {\n''' % methodName);
		elif instance:
			self._w(
'''	static int %s(lua_State* L) {
		%s* _p = %s::instance();\n''' % (methodName, className, className));
		elif sharedPtr:
			self._w(
'''	static int %s(lua_State* L) {
		%s& _p = *Lua::callerIntrusivePtr<%s>(L);\n''' % (methodName, className, className));
		else:
			self._w(
'''	static int %s(lua_State* L) {
		%s* _p = Lua::callerPtr<%s>(L);\n''' % (methodName, className, className));

	def _endCaller(self):
		self._w(
'''	}\n\n''');

	def _argumentsTypes(self, arguments, maxArgs):
		types = "";
		for arg in arguments:
			maxArgs -= 1;
			if maxArgs < 0: break;
			
			if types: types += ", ";
			if arg.type == kPyCStr:
				types += "std::string";
			elif (kArray in arg.type) or (kGenericContainer in arg.type):
				key = (kArray in arg.type) and kArray or kGenericContainer;
				element = arg.type[len(key)+1:];
				types += "std::vector<%s>" % element;
			else:
				types += arg.type;

		return types;

	def _writeOverloadMethod(self, methodName, className, sharedPtr, interface):
		overloads = interface.Overloads(methodName);
		# no overloads. do not check argument type for performance.
		if len(overloads) == 1 and (len(overloads[0].Arguments()) == 0 or not overloads[0].Arguments()[-1].optional):
			return self._ws(self._formatMethodCall(overloads[0], className, sharedPtr, overloads[0].IsStatic() and 1 or 2));

		for method in overloads:
			n = len(method.Arguments());
			for i in range(n, -1, -1):
				self._w(
'''		%s''' % "if (Lua::checkArguments");

				if i != 0:
					self._w("<" + self._argumentsTypes(method.Arguments(), i) + ">");
				self._w('''(L, 2)) {\n''');
				self._ws(self._formatMethodCall(method, className, sharedPtr, overloads[0].IsStatic() and 1 or 2, i, 3));
				self._w(
'''		}\n\n''');

				# break if the argument index i - 1 is not optional.
				if i != 0 and not method.Arguments()[i - 1].optional:
					break;

		self._w(
'''		Debug::LogError("failed to call \\"%s\\", invalid arguments.");
		return 0;
''' % methodName);

	def _writeOverloadConstructors(self, className, interface):
		overloads = interface.Overloads(className);
		# no explict constructor.
		if len(overloads) == 0:
			return self._ws(self._formatOverloadConstructorCall(None, className, False));

		# no overloads. do not check argument type for performance.
		if len(overloads) == 1 and (len(overloads[0].Arguments()) == 0 or not overloads[0].Arguments()[-1].optional):
			return self._ws(self._formatOverloadConstructorCall(overloads[0], className, False));

		for method in overloads:
			n = len(method.Arguments());
			for i in range(n, -1, -1):
				self._w(
'''		%s''' % "if (Lua::checkArguments");

				if i != 0:
					self._w("<" + self._argumentsTypes(method.Arguments(), i) + ">");
				self._w('''(L, 1)) {\n''');
				self._ws(self._formatOverloadConstructorCall(method, className, True, i));
				self._w(
'''		}\n\n''');

				# break if the argument index i - 1 is not optional.
				if i != 0 and not method.Arguments()[i - 1].optional:
					break;

		self._w(
'''		Debug::LogError("failed to call \\"%s\\", invalid arguments.");
		return 0;
''' % className);

	def _writeMethod(self, method, className, instance, sharedPtr, interface):
		for overload in interface.Overloads(method.Name()):
			self._w("\t// " + overload.Text() + "\n");

		self._beginCaller(method.IsStatic(), method.Name(), className, instance, sharedPtr);

		method = self._writeOverloadMethod(method.Name(), className, sharedPtr, interface);

		self._endCaller();

	def _writeToString(self, className, instance, sharedPtr, interface):
		self._beginCaller(False, "ToString", className, instance, sharedPtr);

		self._w('''
		lua_pushstring(L, String::Format("%s@0x%%p", %s).c_str());
		return 1;\n''' % (className, sharedPtr and "_p.get()" or "_p"));

		self._endCaller();

		if interface.HasStatic():
			self._beginCaller(True, "ToStringStatic", className, instance, sharedPtr);
			self._w(
'''		lua_pushstring(L, "static %s");
		return 1;\n''' % (className));
			self._endCaller();

	def _writeStaticMethods(self, className, interface):
		self._w(
'''	static int %sStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {''' % className);

		for method in interface.Methods():
			if method.IsStatic():
				self._w('''
			{ "%s", %s },''' % (method.Name(), method.Name()));

		self._w('''
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
''');

	def _writeMethods(self, className, instance, sharedPtr, interface):
		self._writeToString(className, instance, sharedPtr, interface);

		if interface.HasStatic():
			self._writeStaticMethods(className, interface);

		for method in interface.Methods():
			if method.IsConstructor(): continue;
			self._writeMethod(method, className, instance, sharedPtr, interface);

	def _writeRegister(self, className, baseName, instance, sharedPtr, interface):
		self._w("public:");
		self._w('''
	static void create(lua_State* L) {
		Lua::createMetatable<%s>(L);
	}
	''' % className);
		self._w('''
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {''');

		if instance:
			self._w('''
		funcs.push_back(luaL_Reg { "%sInstance", %sInstance });\n''' % (className, className));
		elif not interface.Abstract() and not interface.IsNotNewable():
			self._w('''
		funcs.push_back(luaL_Reg { "New%s", New%s });\n''' % (className, className));

		if interface.HasStatic():
			self._w('''
		fields.push_back(luaL_Reg{ "%s", %sStatic });\n''' % (className, className));

		self._w('''
		luaL_Reg metalib[] = {''');

		if not instance:
			if sharedPtr:
				self._w('''
			{ "__gc", Lua::deleteIntrusivePtr<%s> },''' % className);
			else:
				self._w('''
			{ "__gc", Lua::deletePtr<%s> },''' % className);

			self._w('''
			{ "__tostring", ToString }, ''');

		for method in interface.Methods():
			if not method.IsStatic() and not method.IsConstructor():
				self._w('''
			{ "%s", %s },''' % (method.Name(), method.Name()));

		baseMetatable = baseName and ("TypeID<%s>::string()" % self._wrapClassName(baseName)[1]) or "nullptr";
		
		self._w('''
			{ nullptr, nullptr }
		};

		Lua::initMetatable<%s>(L, metalib, %s);
	}
''' % (className, baseMetatable));

	def _wrapClass(self, className, baseName, defination, instance, struct):
		interface = Interface(className, defination, struct);

		Msg("  Parsing class %s(%d)..." % (className, interface.MethodCount()));
		sharedPtr, className = self._wrapClassName(className);

		self._w("\nclass " + className + kClassPostfix + " {\n");
		if not interface.Abstract() and not interface.IsNotNewable():
			self._writeConstructor(className, instance, sharedPtr, interface);
			self._w("\n");

		self._writeMethods(className, instance, sharedPtr, interface);
		self._writeRegister(className, baseName, instance, sharedPtr, interface);
		self._w("};\n");

		return className + kClassPostfix;

class Environment:
	def __init__(self):
		self.enums = [];
		self.classes = [];
		self.wrapperFiles = [];
		
		for path in self._sourceFiles():
			(classes, enums) = self._collectClassesAndEnums(path);
			if classes:
				self.classes.append((path, classes));
				self.wrapperFiles.append((WrapperFileName(path)));
			
			if enums: self.enums.extend(enums);
	
	def _sourceFiles(self):
		paths = os.listdir("./");

		paths = [f for f in paths if f.endswith(".h") and f not in kExcludeFiles];
		#paths = [ "student.h" ]; # , "class.h", "imageeffect.h", "rect.h" ];
		return paths;
		
	def _collectClassesAndEnums(self, filePath):
		enum = None; enums = [];
		classes = [];
		name = ""; base = ""; body = []; instance = False; struct = False;

		prev = "";
		for text in open(filePath, encoding = "utf-8").readlines():
			line = text.strip();

			if enum and (line == ")" or line == ");" or line == "};"):
				enums.append(enum);
				enum = None;

			if enum and not line.startswith("_"):
				p = line.find("=");
				if p >= 0: line = line[:p];
				line = line.strip("\t ,");
				if line: enum.append(line);

			if not enum:
				m = kEnumPattern.match(line);
				if m: enum = [m.group(1)];
				else:
					m = kBetterEnumPattern.match(line);
					if m: enum = ["#" + m.group(1)];
				
			m = ("template" not in prev) and kClassPattern.match(line);
			if not m:
				body.append(line);
			else:
				if name: classes.append((name, base, body, instance, struct));
				struct = "struct" in m.group(1);
				name = m.group(3);
				base = m.group(5);

				if "std::enable_shared_from_this" in base: base = "";

				# protected or private inheritance, base class is inaccessible.
				if ("public" not in (m.group(4) or "")) and (not struct): base = "";

				instance = ("Singleton" in base);

				if not self._classWrapable(name): name = "";
				body = [];

			prev = line;

		if name: classes.append((name, base, body, instance, struct));
		return (classes, enums);

	def _classWrapable(self, className):
		return className not in kExcludeClasses;

class Configure:
	def Write(self, enums, wrapperClasses, wrapperFiles):
		dest = kDestFolder + "luaconfig.h";
		
		if os.path.isfile(dest): Chmod(dest, S_IWRITE);
		
		self._f = open(dest, "w", encoding = "utf-8");
		self._f.write("// Warning: this file is generated by %s.\n\n" % os.path.basename(__file__));
		self._f.write("#pragma once\n\n");

		for w in wrapperFiles:
			self._f.write('#include "%s"\n' % w);

		self._enums = enums;
		self._wrapperClasses = wrapperClasses;

		self._writeConfigBegin();
		self._writeTableConfig();
		self._writeEnumConfig();
		self._writeConfigEnd();

		self._f.close();
		Chmod(dest, S_IREAD | S_IRGRP | S_IROTH);
		
	def _writeConfigBegin(self):
		self._f.write('''
namespace Lua {

static int configure(lua_State* L) {
	luaL_checkversion(L);
''');

	def _writeConfigEnd(self):
		self._f.write('''
#pragma region register constructors and getters.
	funcs.push_back(luaL_Reg{ nullptr, nullptr });
	luaL_setfuncs(L, funcs.data(), 0);

	for (luaL_Reg& field : fields) {
		field.func(L);
		lua_setfield(L, -2, field.name);
	}
	
#pragma endregion

	return 1;
}

}	// namespace Lua
''');

	def _writeEnumConfig(self):
		self._f.write("\n#pragma region register enumerations\n");
		for e in self._enums:
			self._f.write('''
	// %s''' % (e[0].startswith("#") and e[0][1:] or e[0]));

			self._f.write('''
	lua_newtable(L);\n''');
	
			if (not e[0].startswith("#")):
				for i in range(1, len(e)):
					self._f.write(
'''	lua_pushinteger(L, (int)%s::%s);
	lua_setfield(L, -2, \"%s\");
''' % (e[0], e[i], e[i]));
			else:
				t = e[0][1:];
				self._f.write(
'''	for (int i = 0; i < %s::size(); ++i) {
		lua_pushinteger(L, %s::value(i));
		lua_setfield(L, -2, %s::value(i).to_string());
	}
''' % (t, t, t));

			self._f.write('''
	lua_setfield(L, -2, \"%s\");
''' % (e[0].startswith("#") and e[0][1:] or e[0]));

		self._f.write("\n#pragma endregion\n");

	def _writeTableConfig(self):
		self._f.write("\n#pragma region register metatables\n\n");
		for w in self._wrapperClasses:
			self._f.write("\t%s::create(L);\n" % w);

		self._f.write("\n\tstd::vector<luaL_Reg> funcs, fields;\n\n");

		for w in self._wrapperClasses:
			self._f.write("\t%s::initialize(L, funcs, fields);\n" % w);
	
		self._f.write('''
	lua_createtable(L, 0, (int)funcs.size());
''');
		self._f.write("\n#pragma endregion\n");

w = Wrapper();
env = Environment();

RunAsExternal = len(sys.argv) > 1;

def ClearFolder():
	for file in os.listdir(kDestFolder):
		path = os.path.join(kDestFolder, file);
		try:
			if os.path.isfile(path):
				Chmod(path, S_IWRITE);
				os.unlink(path);
		except Exception as e:
			traceback.print_stack();

ClearFolder();
wrapperClasses = [];
for cls in env.classes: wrapperClasses.extend(w.WrapClasses(cls[0], cls[1]));

conf = Configure();
conf.Write(env.enums, wrapperClasses, env.wrapperFiles);

Success("luawrapper exited successfully");

if not RunAsExternal:
	os.system("pause");
