import sys
import sre
import os

import SCons.Defaults

env = Environment( tools = [ 'default', 'doxygen' ], toolpath = '.' )

arguments = { 'DESTDIR' : '/',
			  'PREFIX' : '/usr',
			  'SYSCONFDIR' : '/etc',
			  'GMBUS_API' : '1.0',
			  'PACKAGE' : 'gmbus',
			  'VERSION' : '0.3.0' }

for arg, default in arguments.items():
	if ARGUMENTS.has_key( arg ):
		env[ arg ] = ARGUMENTS[ arg ]
	else:
		env[ arg ] = default

if env.has_key( 'PREFIX' ) and env.has_key( 'GMBUS_API' ):
	env[ 'INCLUDEDIR' ] = '%s/include/gmbus-%s' % \
						  ( env[ 'PREFIX' ], env[ 'GMBUS_API' ] )
	env[ 'LIBDIR' ] = env[ 'PREFIX' ] + '/lib'

env.Append( CCFLAGS = [ '-Wall', '-O2' ] )
# for debugging purpose
# env.Append( CCFLAGS = [ '-Wall', '-g3' ] )

env.Append( CPPDEFINES = [ 'PREFIX="${PREFIX}"',
						   'SYSCONFDIR=\\"${SYSCONFDIR}\\"',
						   'PACKAGE=\\"%s\\"' % env[ 'PACKAGE' ],
						   'GETTEXT_PACKAGE=\\"gmbus\\"',
						   'VERSION=\\"%s\\"' % env[ 'VERSION' ] ] )
required_libs = ( ( 'glib-2.0', '2.6.0' ), )

## custom tests
def CheckPKGConfig( context, version ):
	context.Message( 'Checking for pkg-config... ' )
	ret = context.TryAction( 'pkg-config --atleast-pkgconfig-version=%s' % version )[ 0 ]
	context.Result( ret )
	return ret

def CheckPKG( context, name, version, required = True ):
	check = '%s >= %s' % ( name, version )
	context.Message( 'Checking for %s... ' % name )
	ret = context.TryAction( 'pkg-config --exists \'%s\'' % check )[ 0 ]
	if not ret:
		print '%s not found' % name
		if required:
			Exit( 1 )
	context.Result( ret )
	return ret

# custom builder
def build_template( target, source, env ):
	_pattern = sre.compile( '@(\w*)@' )
	src = open( str( source[ 0 ] ), 'r' )
	res = open( str( target[ 0 ] ), 'w' )
	for line in src.readlines():
		for match in _pattern.findall( line ):
			if env.has_key( match ):
				line = line.replace( '@%s@' % match, env[ match ] )
		res.write( line )
	src.close()
	res.close()
	return None

tmpl = Builder( action = build_template, src_suffix = '.in' )
env.Append( BUILDERS = { 'Template' : tmpl } )

# custom install
def link_or_copy( dest, source, env ):
	if os.path.islink( source ):
		os.symlink( os.readlink( source ), dest )
	else:
		SCons.Defaults.installFunc( dest, source, env )

env[ 'INSTALL' ] = link_or_copy
# Build shared library and install symlink
def SystemLibrary( env, target, source, version = '0' ):
	shlib_unversioned = env[ 'LIBPREFIX' ] + target + env[ 'SHLIBSUFFIX' ]
	shlibpath_unversioned = shlib_unversioned
	env[ 'SHLIBSUFFIX' ] += '.%s' % version
	shlib = env[ 'LIBPREFIX' ] + target + env[ 'SHLIBSUFFIX' ]
	shlibpath = os.path.join( '${DESTDIR}/${PREFIX}/lib/', shlib )

	link = env.Command( shlib_unversioned, shlib,
						'/bin/ln -s %s $TARGET' % shlib )
 	env.Install( '${DESTDIR}/${PREFIX}/lib/', shlib_unversioned )

	lib = env.SharedLibrary( target, source )
	env.Install( '${DESTDIR}/${PREFIX}/lib/', lib )
	env.Alias( 'install', '${DESTDIR}/${PREFIX}/lib/' )

	return [ lib, link ]

## configuration
conf = Configure( env, custom_tests = { 'CheckPKGConfig' : CheckPKGConfig,
										'CheckPKG' : CheckPKG } )

if not conf.CheckPKGConfig( '0.15.0' ):
	print 'pkg-config >= 0.15.0 not found.'
	Exit( 1 )

for lib, version in required_libs:
	conf.CheckPKG( lib, version )

env = conf.Finish()

## find pkg libs
env_glib = env.Copy()
env_glib.ParseConfig( 'pkg-config --cflags --libs glib-2.0' )
env_test = env_glib.Copy()

# Templates
templates = [ 'gmbus-1.0.pc.in' ]
tmpls = []
for src in templates:
	tmpls.append( env.Template( source = src ) )
env.Install( '${DESTDIR}/${PREFIX}/lib/pkgconfig/', tmpls )
env.Alias( 'install', '${DESTDIR}/${PREFIX}/lib/pkgconfig/' )

# documentation
doc = SConscript( [ 'doc/SConscript' ], exports = [ 'env' ] )

# library
libgmbus = SConscript( [ 'gmbus/SConscript' ],
					   exports = [ 'env_glib', 'SystemLibrary' ] )

# test programs
tests = SConscript( [ 'tests/SConscript' ], exports = [ 'env_test' ] )
