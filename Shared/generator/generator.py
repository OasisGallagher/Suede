import os;
import sys;
from stat import S_IREAD, S_IRGRP, S_IROTH, S_IWRITE;

def CompareModifiedTime(inf, outf, path):
	tinf = os.stat(inf).st_mtime;
	toutf = os.stat(outf).st_mtime;
	tcaller = os.stat(path).st_mtime;
	tself = os.stat(__file__).st_mtime;

	if tcaller <= toutf and tinf <= toutf and tself <= toutf:
		print("Nothing changed, skip.");
		return False;
	return True;

def Run(path, callback):
	fullname = os.path.basename(path);
	filename = fullname[:fullname.find(".")];

	inf = filename + ".txt";
	outf = filename + ".h";

	if (len(sys.argv) > 1): inf = sys.argv[1];
	if (len(sys.argv) > 2): outf = sys.argv[2];

	answer = "";

	while answer.lower() != "y" and answer.lower() != "n":
		answer = input("read from %s, write to %s, continue? (y/n)\n" % (inf, outf));

	if answer.lower() != "y": return;

	if CompareModifiedTime(inf, outf, path):
		os.chmod(outf,  S_IREAD | S_IWRITE);
		dest = open(outf, "w");
		dest.write("// Warning: this file is generated by " + fullname + ".\n\n");

		try:
			callback(open(inf, "r"), dest);
			os.chmod(outf,  S_IREAD | S_IRGRP | S_IROTH);
		except Exception as e:
			print(e);
		else:
			print("Success.");

	input("Press enter to exit...");
