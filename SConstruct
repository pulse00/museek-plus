# Import local helper functions

from muscons import *
import os
import sys
import string
import SCons

print "Using SCons version", SCons.__version__

check_deps = 1

# Debug logging (LOG=debug,cycle,calltrace,traffictime)

tmp = Environment()

opts = Options('defaults.py')

opts.Add('CC', ['C compiler to use'], tmp['CC'])
opts.Add('CXX', ['C++ compiler to use'], tmp['CXX'])
opts.Add('LINK', ['Linker to use'], tmp['LINK'])
opts.Add('PREFIX', ['Target prefix'])
opts.Add('BINDIR', ['Where binaries go to'])
opts.Add('DATADIR', ['Where data files go to'])
opts.Add('MANDIR', ['Where Man Pages are installed'])
opts.Add('DESTDIR', ['Physical installation prefix', ''])
opts.Add('QTDIR', ['Where Qt is installed', ''])
opts.Add('QT_LIB', ['Specify qt library to use', ''])
opts.Add('QT_MOC', ['Specify meta object compiler to use', ''])
opts.Add('QT_UIC', ['Specify user interface compiler to user', ''])
opts.Add(BoolOption('BUILDDIR', 'Build to build-*-* directory', ''))
opts.Add(BoolOption('PROFILE', 'Build for profiling (gcc only)', ''))
opts.Add(BoolOption('RELEASE', 'Build for release', ''))
opts.Add(ListOption('MULOG', 'Set debug output', '', ['debug', 'cycle', 'calltrace', 'traffictime']))
opts.Add(BoolOption('EPOLL', 'Use epoll when available', ''))
opts.Add(BoolOption('MUSEEQ', 'Build the Qt gui (when Qt can be found', ''))
opts.Add(BoolOption('ONLYMUSEEQ', 'Build the Qt gui (when Qt can be found', ''))
opts.Add(BoolOption('VORBIS', 'Build the File Scanner with OGG Vorbis support', ''))
opts.Add(BoolOption('QSA', 'Specify wether to use QSA if available', ''))
opts.Add(BoolOption('RELAY', 'Use relaytool to generate run-time conditional linking', ''))
opts.Add(BoolOption('RELAY_QSA', 'Use relaytool to generate run-time conditional linking for QSA', ''))
opts.Add(BoolOption('BINRELOC', 'Support binary relocation', ''))
opts.Add(BoolOption('LIBXMLPP_STATIC', 'Horrible hack to statically link against libxml++', ''))



# Set up our environment

env = Environment (
    options = opts,
    CPPPATH = [
        '#',
    ],
    tools = ['default'],
)

env.SConsignFile()


# Build-dir, inspired by <...>

if env['BUILDDIR']:
    arch = os.uname()[4]
    if arch == 'Power Macintosh': arch = 'ppc'
    os_name = os.uname()[0].lower().replace('-','')
    buildDir = 'build-%s-%s' % (arch, os_name)
else:
    buildDir = None

if buildDir is None:
    libbase = '#'
else:
    libbase = os.path.join('#', buildDir)

if not env.has_key('DESTDIR'):
	env.Replace(DESTDIR = '')

if env['DESTDIR'] == '/':
    env.Replace(DESTDIR = '')

env.Append(
    LIBPATH = [
        os.path.join(libbase, 'Muhelp'),
        os.path.join(libbase, 'Museekal'),
        os.path.join(libbase, 'Museek'),
        os.path.join(libbase, 'Tools'),
    ]
)
env.Replace(LIBPATH_MUCIPHER = [os.path.join(libbase, 'Mucipher')])
env.Append(LIBPATH = env['LIBPATH_MUCIPHER'])



# All warnings if compiler is gcc

if os.path.basename(env['CC']) in ['gcc', 'apgcc']:
    env.Append(CCFLAGS = ['-fPIC', '-Wall', '-pipe']) # allows x86_64 to compile
#    env.Append(CCFLAGS = ['-Wall', '-pipe']) # original
    if env['RELEASE']:
        env.Append(CCFLAGS = ['-fomit-frame-pointer'])
    if env['PROFILE']:
        env.Append(CCFLAGS = ['-pg'])
    if not env['RELEASE'] and not env['PROFILE']:
        env.Append(CCFLAGS = ['-g'])



# Set up additional include and library paths

home = os.environ['HOME']
for dir in ['/usr/local/include', '/usr/include', '/usr/X11R6/include', '/sw/include', os.path.join(home, 'include')]:
    if os.path.isdir(dir):
        env.Append(CPPPATH = dir)
for dir in ['/usr/local/lib', '/usr/X11R6/lib', '/sw/lib', os.path.join(home, 'lib')]:
    if os.path.isdir(dir):
        env.Append(LIBPATH = dir)
    if os.path.isdir(dir + '/pkgconfig'):
        if os.environ.has_key('PKG_CONFIG_PATH'):
            old = os.environ['PKG_CONFIG_PATH'] + ':'
        else:
            old = ''
        os.environ['PKG_CONFIG_PATH'] = old + dir + '/pkgconfig'



# Create our system.h file
if check_deps == 1:
	sys_h = open('system.h', 'w')
	sys_h.write('/* Auto-generated by build script, do not edit */\n\n')
	sys_h.write('#include "version.h"\n\n')



	# Write debug log configuration to system.h

	dtypes = [i for i in str(env['MULOG']).split(',') if i]
	if dtypes:
		if 'debug' in dtypes or dtypes == ['all']:
			sys_h.write('#define MULOG_DEBUG 1\n')
		if 'calltrace' in dtypes or dtypes == ['all']:
			sys_h.write('#define MULOG_CALLTRACE 1\n')
		if 'traffictime' in dtypes or dtypes == ['all']:
			sys_h.write('#define MULOG_TT 1\n')
		if 'cycle' in dtypes or dtypes == ['all']:
			sys_h.write('#define MULOG_CYCLE 1\n')
		sys_h.write('\n')



# Set up our configuration tester

conf = env.Configure(custom_tests = { 
  'CheckLargeFile' : CheckLargeFile,
  'CheckSignalRetType': CheckSignalRetType,
  'CheckScandirType': CheckScandirType,
})



# If relay is enabled, check for supported platform

if env['RELAY'] or env['RELAY_QSA']:
	if os.path.basename(env['CC']) not in ['gcc', 'apgcc']:
		print "GCC required for relayed linking (%s used)" % os.path.basename(env['CC'])
		Exit(1)
	if os.uname()[4] not in ['i386', 'i486', 'i586', 'i686']:
		print "Relayed linking only allowed on intel x86 architecture"
		Exit(1)
	libs = env.has_key('LIBS') and env['LIBS'] or []
	if not conf.CheckLibWithHeader('dl', 'dlfcn.h', 'C', 'dlopen(0, 0);'):
		print "Couldn't figure out how to use the dynamic run-time linker"
		Exit(1)
	env.Replace(LIBS = libs)
	env.Append(CCFLAGS = ['-fPIC'])


if check_deps == 1:
	# Check data size types
	result = conf.TryRun("""
#include <stdio.h>
int main() {
 printf("%i %i %i", sizeof(int), sizeof(short), sizeof(long int));
 return 0;
}""", ".c")
	if result[0] != 1:
		print "Couldn't compile type-size test.. Your system is strange"
		Exit(1);
	size_int, size_short, size_longint = result[1].split(" ")
	sys_h.write("#define SIZE_INT %s\n" % size_int)
	sys_h.write("#define SIZE_SHORT %s\n" % size_short)
	sys_h.write("#define SIZE_LONGINT %s\n\n" % size_longint)



# Check for system specific includes
if check_deps == 1:
	headers = []
	for header in Split("""
  stdio.h
  sys/types.h
  sys/stat.h
  stdlib.h
  stddef.h
  memory.h
  string.h
  strings.h
  inttypes.h
  stdbool.h
  stdint.h
  unistd.h
  stdarg.h
  netdb.h
  sys/socket.h
  sys/un.h
  fcntl.h
  netinet/in.h
  sys/poll.h
  pwd.h
  errno.h
"""):
		t = ""
		for h in headers:
			t += "#include <%s>\n" % h
		t += "#include <%s>\n" % header
		sys.stdout.write("Checking for header file %s..." % header)
		sys.stdout.flush()
		if conf.TryCompile(t, ".c"):
			sys.stdout.write(" yes\n")
			headers.append(header)
			sys_h.write('#define HAVE_%s 1\n' % (header.upper().replace('/', '_').replace('.', '_')))
			sys_h.write('#include <%s>\n\n' % header)
		else:
			sys.stdout.write(" no\n")


if check_deps == 1:
	# Check if iconv wants char ** or const char**
	sys.stdout.write('Checking for iconv input type...')
	sys.stdout.flush()
	t = """
#include <iconv.h>
int main() { size_t r = iconv(0, (%s)0, 0, (char**)0, 0); return 0; }
"""
	if conf.TryCompile(t % 'const char **' , '.cc'):
		sys.stdout.write(' const char**\n')
		sys_h.write('#define ICONV_IN const char **\n\n')
	elif conf.TryCompile(t % 'char **', '.cc'):
		sys.stdout.write(' char**\n')
		sys_h.write('#define ICONV_IN char **\n\n')
	else:
		sys.stderr.write('Couldn\'t determine what iconv wants for input.. Please contact the maintainer.\n')
		Exit(1)

	# Check if we can use epoll (linux 2.6+ async poller)

	if env['EPOLL']:
		if SCons.__version__[:4] == "0.95" or SCons.__version__ == "0.96.1":

			if conf.CheckCHeader(['stdint.h', 'sys/epoll.h']) and conf.CheckFunc('epoll_ctl', 'C++'):
				sys_h.write('#define HAVE_EPOLL_CTL 1\n')
				sys_h.write('#include <sys/epoll.h>\n\n')
		else:
			if conf.CheckCHeader(['stdint.h', 'sys/epoll.h']) and conf.CheckFunc('epoll_ctl', "", 'C++'):
				sys_h.write('#define HAVE_EPOLL_CTL 1\n')
				sys_h.write('#include <sys/epoll.h>\n\n')



	# Check for signal.h and signal handler return type

	if conf.CheckCHeader('signal.h'):
		sys_h.write('#define HAVE_SIGNAL_H 1\n')
		sys_h.write('#include <signal.h>\n')
		conf.CheckSignalRetType(sys_h)
		sys_h.write('\n')



# Check if UNIX_PATH_MAX is defined, if not, define it

	if not conf.CheckType('UNIX_PATH_MAX'):
		sys_h.write('#define UNIX_PATH_MAX 108\n\n')



# Check sys/time.h and time.h inclusion

	if conf.CheckCHeader(['sys/time.h', 'time.h']):
		sys_h.write('#include <sys/time.h>\n#include <time.h>\n')
	elif conf.CheckCHeader('sys/time.h'):
		sys_h.write('#include <sys/time.h>')
	elif conf.CheckCHeader('time.h'):
		sys_h.write('#include <time.h>')
	sys_h.write('\n')



# Check if we have scandir, and check it's entry type

	CheckScandir(sys_h, conf)



# Check how we enable large file support

	conf.CheckLargeFile(sys_h)



# Check for uint and uint32
	sys_h.flush()
	if not conf.CheckType('uint', '#include <system.h>\n', 'c++'):
		sys_h.write("typedef unsigned int uint;\n")
	if not conf.CheckType('uint32', '#include <system.h>\n', 'c++'):
    		if(size_short == 4):
        		sys_h.write("typedef unsigned short uint32;\n\n")
    		else:
        		sys_h.write("typedef unsigned int uint32;\n\n")
	sys_h.write('\n')


# We're finished with system.h, close it
if check_deps == 1:
	sys_h.close()

# Close the configuration environment

env = conf.Finish()

# Export the environment

Export('env')


# Include our subdirectories
if env['ONLYMUSEEQ']:
	
	subdirs = ['Mucipher']
	if env['MUSEEQ']:
		subdirs.append('museeq')
		print "Compiling only Mucipher and Museeq."
	else:
		print "Whoops, Compiling only Mucipher, not Museeq!"
else:
	subdirs = ['Muhelp', 'Mucipher', 'Museekal', 'Museek', 'Tools', 'museekd', 'doc', 'pymuseekd']
	print "Compiling Muhelp, Mucipher, Museekal, Museek, museekd, Tools, doc, pymuseekd..."
	if env['MUSEEQ']:
		subdirs.append('museeq')
		print "\tand museeq, too."
	else:
		print "\tWithout museeq."
for dir in subdirs:
	if dir in ['Muhelp', 'Museekal', 'Museek', 'Tools', 'museeq']:
		print "Headers for %s..." % dir
	if buildDir is None:
		bd = None
	else:
		bd = os.path.join(buildDir, dir)
	SConscript(os.path.join(dir, 'SConscript'), build_dir = bd, duplicate = 1)



# Save the options we want to have saved (basically, everything but DESTDIR)
print env['DESTDIR']
f = open("mulocal.py", "w")
f.write('PREFIX = %s\n' % (`env['PREFIX']`))
f.write('BINDIR = %s\n' % (`env['BINDIR']`))
f.write('DATADIR = %s\n' % (`env['DATADIR']`))
f.write('MANDIR = %s\n' % (`env['MANDIR']`))
f.write('BUILDDIR = %s\n' % (`env['BUILDDIR']`))
f.write('RELEASE = %s\n' % (`env['RELEASE']`))
f.write('MULOG = %s\n' % (`string.join(env['MULOG'], ',')`))
f.write('EPOLL = %s\n' % (`env['EPOLL']`))
f.write('RELAY = %s\n' % (`env['RELAY']`))
f.write('BINRELOC = %s\n' % (`env['BINRELOC']`))
f.write('MUSEEQ = %s\n' % (`env['MUSEEQ']`))
f.write('ONLYMUSEEQ = %s\n' % (`env['ONLYMUSEEQ']`))
f.write('QSA = %s\n' % (`env['QSA']`))
f.write('RELAY_QSA = %s\n' % (`env['RELAY_QSA']`))
f.write('QTDIR = %s\n' % (`env['QTDIR']`))
f.write('QT_LIB = %s\n' % (`env['QT_LIB']`))
f.write('QT_MOC = %s\n' % (`env['QT_MOC']`))
f.write('QT_UIC = %s\n' % (`env['QT_UIC']`))
f.close()