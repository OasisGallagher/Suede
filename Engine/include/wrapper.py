# -*- coding: utf-8 -*-

import os;
import re;

kFilePostfix = "_wrapper";
kClassPostfix = "_Wrapper";
kDestFolder = "F:/GitHub/Suede/Engine/internal/lua/wrappers/";
kPyCStr = "py_cstr";
kGenericContainer = "std::vector<";
kClassPattern = re.compile(r"\s*class\s+([A-Z_]+\s+)?\s*([A-Za-z0-9_]+)\s*\:?\s*(?:public )?([A-Za-z0-9_:<>]*)");
kMethodPattern = re.compile(r"\s*(?:virtual)?\s*([A-Za-z0-9:<>_\*]+)\s+([A-Za-z0-9]+)\s*\((.*)\)\s*(?:const)?\s*(=\s*0)?[;\{]\s*");
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
	"WorldEventListener",
	"FrameEventListener",
	"GameObjectLoadedListener",
	"ScreenSizeChangedListener",
	"FontMaterialRebuiltListener",
	"WorldGameObjectWalker",
];

kNotNewables = [
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
	def __init__(self, type, value, array = False):
		self.type = type;
		self.value = value;
		self.array = array;

class Method:	# void Print(const char* message) for example.
	def __init__(self, groups):
		self._arguments = [];
		self._overloaded = "";
		self._pureVirtual = False;
		self._initialize(groups);

	def Name(self):
		'''Print2, 3, ...'''
		return self._overloaded or self._name;
	
	def RawName(self):
		'''Print'''
		return self._name;
		
	def SetOverloadedName(self, value):
		self._overloaded = value;

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

	def IsPureVirtual(self):
		return self._pureVirtual;

	def Arguments(self):
		return self._arguments
		
	def _split(self, arg):
		equals = arg.find("=");
		if equals >= 0: arg = arg[:equals];
		
		arg = arg.replace("const char*", kPyCStr);
		
		# remove & and const.
		arg = arg.replace("&", "").replace("const ", "").strip();

		space = arg.rfind(' ');
		
		type = arg[:space];
		value = arg[space + 1:];
		
		# is array.
		array = False;
		if "[" in value:
			array = True;
			type = "std::vector<" + type + ">";
			value = value[:value.find("[")];
			
		return (type, value, array);

	def _initialize(self, groups):
		self._r = groups[0];
		self._name = groups[1];
		self._pureVirtual = groups[3] != None;
		
		args = groups[2];
		if args == "": return;

		for arg in ("," not in args) and [ args ] or args.split(","):
			fields = self._split(arg);
			self._arguments.append(Argument(fields[0], fields[1], fields[2]));

class Interface:
	def __init__(self, className, defination):
		self._methods = [ ];
		self._abstract = False;
		self._notNewable = (className in kNotNewables);
		names = { };
		public = False;
		prev = "";
		for line in defination:
			if line.startswith("public:") or line.startswith("protected:") or line.startswith("private:"):
				public = line.startswith("public:");
			
			m = public and kMethodPattern.match(line);
			if m: self._parseMethod(m, names, prev);
			prev = line;

	def Methods(self):
		return self._methods
		
	def Abstract(self):
		return self._abstract;
		
	def IsNotNewable(self):
		return self._notNewable;
		
	def _parseMethod(self, m, names, prev):
		method = Method(m.groups());
		self._abstract = self._abstract or method.IsPureVirtual();
		if self._methodWrapable(prev, method):
			n = names.get(method.Name(), 0);
			names[method.Name()] = n + 1;
			
			if n > 0: 
				newName = method.Name() + str(n + 1);
				method.SetOverloadedName(newName);
				names[method.Name()] = 1;
			
			self._methods.append(method);
		else:
			Warning("  Skip unwrappable method: %s" % method.Def());
		
	def _methodWrapable(self, prev, method):
		if "template" in prev: return False;
		if "*" in method.Return(): return False;
		
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

def CallMethod(method):
	ans = "_p->%s(" % method.RawName();

	for i in range(len(method.Arguments())):
		if i != 0: ans += ", ";
		arg = method.Arguments()[i];
		ans += (arg.value);
		if method.Arguments()[i].array:
			ans += ".data()";
		elif method.Arguments()[i].type == kPyCStr:
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

def WriteMethods(f, className, instance, sharedPtr, interface):
	for method in interface.Methods():
		if instance:
			f.write(
'''	static int %s(lua_State* L) {
		%s* _p = %s::instance();''' % (method.Name(), className, className));
		elif sharedPtr:
			f.write(
'''	static int %s(lua_State* L) {
		%s& _p = *Lua::callerSharedPtr<%s>(L, %d);''' % (method.Name(), className, className, len(method.Arguments())));
		else:
			f.write(
'''	static int %s(lua_State* L) {
		%s* _p = Lua::callerPtr<%s>(L, %d);''' % (method.Name(), className, className, len(method.Arguments())));

		for i in range(len(method.Arguments())):
			argument = method.Arguments()[i];
			if kGenericContainer in argument.type:
				element = argument.type[len(kGenericContainer):argument.type.find(">")];
				f.write('''
		std::vector<%s> %s = Lua::getList<%s>(L, %d);''' % (element, argument.value, element, -(i + 1)));
			elif kPyCStr in argument.type:
				f.write('''
		std::string %s = Lua::get<std::string>(L, %d);''' % (argument.value, -(i + 1)));
			else:
				f.write('''
		%s %s = Lua::get<%s>(L, %d);''' % (argument.type, argument.value, argument.type, -(i + 1)));

		call = CallMethod(method);
		if method.Return() == "void":
			f.write('''
		%s;
		return 0;''' % call);
		elif kGenericContainer in method.Return():
			f.write('''
		return Lua::pushList(L, %s);''' % call);
		else:
			f.write('''
		return Lua::push(L, %s);''' % call);

		f.write('''
	}\n\n''');

def WriteRegister(f, className, baseName, instance, sharedPtr, interface):
	f.write("public:");
	f.write('''
	static void create(lua_State* L) {
		Lua::createMetatable<%s>(L);
	}
	''' % className);
	f.write('''
	static void initialize(lua_State* L, std::vector<luaL_Reg>& regs) {''');

	if instance:
		f.write('''
		regs.push_back(luaL_Reg { "%sInstance", %sInstance });\n''' % (className, className));
	elif not interface.IsNotNewable():
		f.write('''
		regs.push_back(luaL_Reg { "New%s", New%s });\n''' % (className, className));

	f.write('''
		luaL_Reg metalib[] = {''');

	if not instance:
		if sharedPtr:
			f.write('''
			{ "__gc", Lua::deleteSharedPtr<%s> },''' % className);
		else:
			f.write('''
			{ "__gc", Lua::deletePtr<%s> },''' % className);

	for method in interface.Methods():
		f.write('''
			{ "%s", %s },''' % (method.Name(), method.Name()));

	f.write('''
			{ nullptr, nullptr }
		};

		Lua::initMetatable<%s>(L, metalib, %s);
	}
''' % (className, (baseName and not instance) and ("Lua::metatableName<%s>()" % WrapClassName(baseName)[1])  or "nullptr"));

def WrapClass(f, className, baseName, defination, instance, wrappers):
	interface = Interface(className, defination);
	abstract = interface.Abstract();
	print("  Parsing class %s(%d)..." % (className, len(interface.Methods())));
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
	f.write('#include "../luax.h"\n');
	f.write('#include "' + name + '"\n');

	for c in classes:
		WrapClass(f, c[0], c[1], c[2], c[3], wrappers);
		
	f.close();

def ClassWrapable(className):
	return className not in kExcludeClasses;

def CollectClasses(filePath):
	classes = [];
	name = ""; base = ""; body = []; instance = False;

	prev = "";
	for line in open(filePath).readlines():
		m = ("template" not in prev) and kClassPattern.match(line);
		if not m: 
			body.append(line);
		else:
			if name: classes.append((name, base, body, instance));
			name = m.group(2);
			base = m.group(3);
			if "std::" in base: base = "";
			instance = ("Singleton" in line);
			if not ClassWrapable(name): name = "";
			body = [];
		
		prev = line;

	if name: classes.append((name, base, body, instance));
	return classes;

def BeginConfigure(config):
	config.append('''
namespace Lua {

static int configure(lua_State* L) {
	std::vector<luaL_Reg> regs;
	
''');

def EndConfigure(config):
	config.append('''
	// register constructors or getters.
	luaL_checkversion(L);
	lua_createtable(L, 0, (int)regs.size());

	regs.push_back(luaL_Reg{ nullptr, nullptr });
	luaL_setfuncs(L, regs.data(), 0);

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
ClearFolder(kDestFolder);

# include original files.
classes = [];
for path in paths:
	items = CollectClasses(path);
	if items:
		classes.append((path, items));
		config.append('#include "%s"\n' % (WrapFileName(path)));
	
BeginConfigure(config);

wrappers = [];
for _class in classes:
	if classes: WrapClasses(_class[0], _class[1], wrappers);

for w in wrappers:
	config.append("\t%s::create(L);\n" % w);

config.append("\n");

for w in wrappers:
	config.append("\t%s::initialize(L, regs);\n" % w);

EndConfigure(config);

# write configuration file.
f = open(kDestFolder + "luaconfig.h", "w", encoding = "utf-8");
f.writelines(config);
f.close();

input("Done. Press enter to exit...");

