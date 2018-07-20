import sys;

sys.path.append("../../Shared/generator");
import generator;

def Generate(src, dest):
	if src and dest:
		dest.write("#pragma once\n\n");
		dest.write("const char* const GLEFGrammar =\n");

		for line in src:
			line = line.rstrip().replace('"', '\\"');
			dest.write('\"' + line + '\\n"' + "\n");

		dest.seek(dest.tell() - 5);
		dest.truncate();

		dest.write('";');
		dest.close();

generator.Run(__file__, Generate);
