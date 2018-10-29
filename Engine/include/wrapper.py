# -*- coding: utf-8 -*-

import os;
import re;

kFilePostfix = "_wrapper";
kClassPostfix = "_Wrapper";
kDestFolder = "F:/GitHub/Suede/Engine/internal/lua/wrappers/";
kArray = "[]";
kPyCStr = "py_cstr";
kEnumerable = "Enumerable";
kLuaFunction = "Lua::Func";
kGenericContainer = "std::vector";

kEnumPattern = re.compile(r"enum\s+class\s+([a-zA-Z0-9_]+)");
kBetterEnumPattern = re.compile(r"BETTER_ENUM\(([a-zA-Z0-9_]+)");
kClassPattern = re.compile(r"\s*(struct|class)\s+([A-Z_]+\s+)?\s*([A-Za-z0-9_]+)\s*\:?\s*(?:public )?([A-Za-z0-9_:<>]*)\s*\{");
kMethodPattern = re.compile(r"\s*(static|virtual)?\s*((?!return)[A-Za-z0-9:<>_\*]+)\s+([A-Za-z0-9]+)\s*\((.*?)\)\s*(?:const)?\s*(=\s*0)?[;\{]\s*");

kExcludeFiles = [
	"gui.h",
	"imgui.h",
	"gizmospainter.h",
	"graphicsviewer.h",
	"graphicscanvas.h",

	"profiler.h",
	"opengldriver.h",
	"geometryutility.h",
];

kExcludeClasses = [
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

	"IObject",
	"IComponent",
	"ITransform",
	"IRenderer",
	"IProjector",
	"ILight",
	"ICamera",
	"ITextMesh",
	"IMeshProvider",
	"IMeshFilter",
	"IMeshRenderer",
	"ISkinnedMeshRenderer",
	"IParticleRenderer",
	"IAnimation",
	"IParticleSystem",
	"ITexture",
	"IParticleEmitter",
];

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

		if self._static: Warning(self._text);

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
		return self._methods

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
		if self._methodWrapable(prev, method):
			list = self._overloads.get(method.Name(), None);
			if not list: self._overloads[method.Name()] = list = [];
			list.append(method);

			self._nmethods += 1;
			if len(list) == 1: self._methods.append(method);
		else:
			Warning("  Skip unwrappable method: %s" % method.Text());

	def _methodWrapable(self, prev, method):
		if "template" in prev: return False;
		if "*" in method.Return(): return False;
		if "operator" in method.Text(): return False;

		for arg in method.Arguments():
			if "*" in arg.type: return False;

		return True;

def Warning(message):
	color = '\033[91m'; endc = '\033[0m';
	print(color + message + endc);

def WrapClassName(className):
	sharedPtr = className[0] == "I" and className[1].isupper();
	if sharedPtr: className = className[1:];
	return (sharedPtr, className)

def WrapFileName(name):
	p = name.rfind(".");
	return name[:p] + kFilePostfix + name[p:];

def CallMethod(className, method, sharedPtr, maxArgs):
	if method.IsStatic():
		ans = "%s::%s(" % (sharedPtr and "I" + className or className, method.Name());
	else:
		ans = "_p->%s(" % method.Name();

	for i in range(min(maxArgs, len(method.Arguments()))):
		if i != 0: ans += ", ";
		arg = method.Arguments()[i];
		ans += (arg.value);
		if kArray in arg.type:
			ans += ".data()";
		elif arg.type == kPyCStr:
			ans += ".c_str()";

	ans += ")";
	return ans;

def WriteConstructor(f, className, instance, sharedPtr, abstract):
	if instance:
		f.write(
'''	static int %sInstance(lua_State* L) {
		return Lua::reference<%s>(L);
	}\n'''  % (className, className));
	elif sharedPtr:
		f.write(
'''	static int New%s(lua_State* L) {
		return Lua::fromShared(L, ::New%s());
	}\n''' % (className, className));
	elif abstract:
		f.write(
'''	static int New%s(lua_State* L) {
		return Lua::newInterface<%sInternal>(L);
	}\n''' % (className, className));
	else:
		f.write(
'''	static int New%s(lua_State* L) {
		return Lua::newObject<%s>(L);
	}\n''' % (className, className));

def WriteMethodCall(f, method, className, sharedPtr, maxArgs = float("inf"), ntabs = 2):
	pps = nargs = min(maxArgs, len(method.Arguments()));
	for i in range(nargs):
		argument = method.Arguments()[nargs - i - 1];
		if (kArray in argument.type) or (kGenericContainer in argument.type):
			key = (kArray in argument.type) and kArray or kGenericContainer;
			element = argument.type[len(key)+1:];
			f.write('''
%sstd::vector<%s> %s = Lua::getList<%s>(L, %d);''' % (ntabs * "\t", element, argument.value, element, (nargs - i + 1)));
		elif kPyCStr in argument.type:
			f.write('''
%sstd::string %s = Lua::get<std::string>(L, %d);''' % (ntabs * "\t", argument.value, (nargs - i + 1)));
		elif kLuaFunction in argument.type:
			type = argument.type[argument.type.find("<") + 1:argument.type.rfind(">")];
			if pps - (nargs - i) > 0:
				f.write('''
%slua_pop(L, %d);	// ensure function at top of stack.''' % (ntabs * "\t", pps - (nargs - i)));

			f.write('''
%sauto %s = lua_isnil(L, -1) ? nullptr : Lua::make_func<%s>(L);''' % (ntabs * "\t", argument.value, type));
			pps = nargs - i - 1;
		else:
			f.write('''
%s%s %s = Lua::get<%s>(L, %d);''' % (ntabs * "\t", argument.type, argument.value, argument.type, (nargs - i + 1)));

	call = CallMethod(className, method, sharedPtr, maxArgs);
	if method.Return() == "void":
		f.write('''
%s%s;
%sreturn 0;''' % (ntabs * "\t", call, ntabs * "\t"));
	elif kGenericContainer in method.Return():
		f.write('''
%sreturn Lua::pushList(L, %s);''' % (ntabs * "\t", call));
	elif kEnumerable in method.Return():
		etype = "I%s::%s" % (className, method.Return());
		f.write('''
%s%s _r = %s;
%sreturn Lua::pushList(L, std::vector<%s::value_type>(_r.begin(), _r.end()));'''
		% (ntabs * "\t", etype, call, ntabs * "\t", etype));
	else:
		f.write('''
%sreturn Lua::push(L, %s);''' % (ntabs * "\t", call));

def BeginCaller(f, static, methodName, className, instance, sharedPtr):
	if static:
		f.write(
'''	static int %s(lua_State* L) {''' % methodName);
	elif instance:
		f.write(
'''	static int %s(lua_State* L) {
		%s* _p = %s::instance();''' % (methodName, className, className));
	elif sharedPtr:
		f.write(
'''	static int %s(lua_State* L) {
		%s& _p = *Lua::callerSharedPtr<%s>(L);''' % (methodName, className, className));
	else:
		f.write(
'''	static int %s(lua_State* L) {
		%s* _p = Lua::callerPtr<%s>(L);''' % (methodName, className, className));

def EndCaller(f):
	f.write('''
	}\n\n''');

def ArgumentsTypes(arguments):
	types = "";
	for arg in arguments:
		if types: types += ", ";
		if arg.type == kPyCStr:
			types += "std::string";
		else:
			types += arg.type;

	return types;

def WriteOverloadMethod(f, methodName, className, sharedPtr, interface):
	overloads = interface.Overloads(methodName);
	# no overloads. do not check argument type for performance.
	if len(overloads) == 1 and (len(overloads[0].Arguments()) == 0 or not overloads[0].Arguments()[-1].optional):
		return WriteMethodCall(f, overloads[0], className, sharedPtr);

	for method in overloads:
		n = len(method.Arguments());
		for i in range(n, -1, -1):
			f.write(
'''\n\n		%s''' % "if (Lua::checkArguments");

			if i != 0:
				f.write("<" + ArgumentsTypes(method.Arguments()) + ">");
			f.write('''(L, 2)) {''');
			WriteMethodCall(f, method, className, sharedPtr, i, 3);
			f.write('''
		}''');

			# break if the argument index i - 1 is not optional.
			if i != 0 and not method.Arguments()[i - 1].optional:
				break;

	f.write('''\n
		Debug::LogError("failed to call \\"%s\\", invalid arguments.");
		return 0;''' % methodName);

def WriteMethod(f, method, className, instance, sharedPtr, interface):
	for overload in interface.Overloads(method.Name()):
		f.write("\t// " + overload.Text() + "\n");

	BeginCaller(f, method.IsStatic(), method.Name(), className, instance, sharedPtr);

	method = WriteOverloadMethod(f, method.Name(), className, sharedPtr, interface);

	EndCaller(f);

def WriteToString(f, className, instance, sharedPtr, interface):
	BeginCaller(f, False, "ToString", className, instance, sharedPtr);

	f.write('''
		lua_pushstring(L, String::Format("%s@0x%%p", %s).c_str());
		return 1;''' % (className, sharedPtr and "_p.get()" or "_p"));

	EndCaller(f);

	if interface.HasStatic():
		BeginCaller(f, True, "ToStringStatic", className, instance, sharedPtr);
		f.write('''
		lua_pushstring(L, "static %s");
		return 1;''' % (className));
		EndCaller(f);

def WriteStaticMethods(f, className, interface):
	f.write(
'''	static int %sStatic(lua_State* L) {
		lua_newtable(L);

		luaL_Reg funcs[] = {''' % className);

	for method in interface.Methods():
		if method.IsStatic():
			f.write('''
			{ "%s", %s },''' % (method.Name(), method.Name()));

	f.write('''
			{"__tostring", ToStringStatic },
			{ nullptr, nullptr }
		};

		luaL_setfuncs(L, funcs, 0);

		return 1;
	}
	''');

def WriteMethods(f, className, instance, sharedPtr, interface):
	WriteToString(f, className, instance, sharedPtr, interface);

	if interface.HasStatic():
		WriteStaticMethods(f, className, interface);

	for method in interface.Methods():
		WriteMethod(f, method, className, instance, sharedPtr, interface);

def WriteRegister(f, className, baseName, instance, sharedPtr, interface):
	f.write("public:");
	f.write('''
	static void create(lua_State* L) {
		Lua::createMetatable<%s>(L);
	}
	''' % className);
	f.write('''
	static void initialize(lua_State* L, std::vector<luaL_Reg>& funcs, std::vector<luaL_Reg>& fields) {''');

	if instance:
		f.write('''
		funcs.push_back(luaL_Reg { "%sInstance", %sInstance });\n''' % (className, className));
	elif not interface.IsNotNewable():
		f.write('''
		funcs.push_back(luaL_Reg { "New%s", New%s });\n''' % (className, className));

	if interface.HasStatic():
		f.write('''
		fields.push_back(luaL_Reg{ "%s", %sStatic });\n''' % (className, className));

	f.write('''
		luaL_Reg metalib[] = {''');

	if not instance:
		if sharedPtr:
			f.write('''
			{ "__gc", Lua::deleteSharedPtr<%s> },''' % className);
		else:
			f.write('''
			{ "__gc", Lua::deletePtr<%s> },''' % className);

		f.write('''
			{ "__tostring", ToString }, ''');

	for method in interface.Methods():
		if not method.IsStatic():
			f.write('''
			{ "%s", %s },''' % (method.Name(), method.Name()));

	f.write('''
			{ nullptr, nullptr }
		};

		Lua::initMetatable<%s>(L, metalib, %s);
	}
''' % (className, (baseName and not instance) and ("TypeID<%s>::string()" % WrapClassName(baseName)[1])  or "nullptr"));

def WrapClass(f, className, baseName, defination, instance, struct, wrappers):
	interface = Interface(className, defination, struct);
	abstract = interface.Abstract();
	print("  Parsing class %s(%d)..." % (className, interface.MethodCount()));
	sharedPtr, className = WrapClassName(className);
	wrappers.append((className + kClassPostfix));

	f.write("\nclass " + className + kClassPostfix + " {\n");
	if not interface.IsNotNewable():
		WriteConstructor(f, className, instance, sharedPtr, abstract);
		f.write("\n");

	WriteMethods(f, className, instance, sharedPtr, interface);
	WriteRegister(f, className, baseName, instance, sharedPtr, interface);
	f.write("};\n");

def WrapClasses(name, classes, wrappers):
	dest = WrapFileName(name);

	f = open(kDestFolder + dest, "w", encoding = "utf-8");
	print('Parsing "%s", write to "%s"...' % (name, dest));

	# common headers.
	f.write("// Warning: this file is generated by wrapper.py.\n\n");
	f.write("#pragma once\n\n");
	f.write('#include "' + name + '"\n\n');

	f.write('#include "lua++.h"\n');
	f.write('#include "tools/string.h"\n');

	for c in classes:
		WrapClass(f, c[0], c[1], c[2], c[3], c[4], wrappers);

	f.close();

def ClassWrapable(className):
	return className not in kExcludeClasses;

def CollectClassesAndEnums(filePath):
	enum = None; enums = [];
	classes = [];
	name = ""; base = ""; body = []; instance = False; struct = False;

	prev = "";
	for text in open(filePath).readlines():
		line = text.strip(";\t ");
		if not line or line.startswith("//") or line.startswith("/*"): continue;

		if enum and (line == ")" or line == "); "or line == "};"):
			Warning("Got enum " + str(enum));
			enums.append(enum);
			enum = None;

		if enum and not line.startswith("_"): enum.append(line);

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
			struct = "struct" in m.group(0);
			name = m.group(3);
			base = m.group(4);
			if "std::" in base: base = "";
			instance = ("Singleton" in line);
			if not ClassWrapable(name): name = "";
			body = [];

		prev = line;

	if name: classes.append((name, base, body, instance, struct));

	return (classes, enums);

def BeginConfigure(config):
	config.append('''
namespace Lua {

static int configure(lua_State* L) {
	std::vector<luaL_Reg> funcs, fields;

''');

def EndConfigure(config):
	config.append('''
	// register constructors or getters.
	luaL_checkversion(L);
	lua_createtable(L, 0, (int)funcs.size());

	funcs.push_back(luaL_Reg{ nullptr, nullptr });
	luaL_setfuncs(L, funcs.data(), 0);

	for (luaL_Reg& field : fields) {
		field.func(L);
		lua_setfield(L, -2, field.name);
	}

	return 1;
}

}	// namespace Lua
''');

def SourceFiles():
	paths = os.listdir("f:/github/suede/engine/include");

	paths = [f for f in paths if f.endswith(".h") and f not in kExcludeFiles];
	#paths = [ "student.h" ]; # , "class.h", "imageeffect.h", "rect.h" ];
	return paths;

def ClearFolder(folder):
	for file in os.listdir(folder):
		path = os.path.join(folder, file);
		try:
			if os.path.isfile(path):
				os.unlink(path);
		except Exception as e:
			print(e);

paths = SourceFiles();

config = [
	"// Warning: this file is generated by %s.\n\n" % os.path.basename(__file__),
	"#pragma once\n\n"
];

print("clear folder " + kDestFolder);
#ClearFolder(kDestFolder);

# include original files.
enums = [];
classes = [];
for path in paths:
	(_classes, _enums) = CollectClassesAndEnums(path);
	if _classes:
		classes.append((path, _classes));
		config.append('#include "%s"\n' % (WrapFileName(path)));
	
	if _enums: enums.extend(_enums);

BeginConfigure(config);

wrappers = [];
for _class in classes:
	if classes: WrapClasses(_class[0], _class[1], wrappers);

for w in wrappers:
	config.append("\t%s::create(L);\n" % w);

config.append("\n");

for w in wrappers:
	config.append("\t%s::initialize(L, funcs, fields);\n" % w);

EndConfigure(config);

# write configuration file.
f = open(kDestFolder + "luaconfig.h", "w", encoding = "utf-8");
f.writelines(config);
f.close();

input("Done. Press enter to exit...");
