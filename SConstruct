import os, sys
platform = sys.platform

env = Environment(ENV = {'PATH' : os.environ['PATH']})
env = Environment(CPPPATH='src')
env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa'] 

flags = '-Wall -pedantic -g'
libs = ['SDL','SDL_net', 'SDL_ttf','moag']


#this is where libmoag gets built
env.Append(LIBPATH = ['.'])

#includes for macports
if platform == 'darwin' :
    env.Append(LIBPATH = ['/opt/local/lib'])
    env.Append(CPPPATH = ['/opt/local/include/'])

Library('moag', Glob('moag*.cpp'),    LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH=env['LIBPATH'], CPPPATH=env['CPPPATH'], CPPFLAGS=flags )

if platform == 'darwin' :
    Object( 'SDLMain.o', 'SDLMain.m', LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH='.', CPPPATH=env['CPPPATH'], CPPFLAGS=flags)
    Program('client', ['SDLMain.o']+['client.cpp'], LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH=env['LIBPATH'], CPPPATH=env['CPPPATH'], CPPFLAGS=flags)
    Program('server', ['SDLMain.o']+['server.cpp'], LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH=env['LIBPATH'], CPPPATH=env['CPPPATH'], CPPFLAGS=flags)

else:
    Program('client', ['client.cpp'],     LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH=env['LIBPATH'], CPPPATH=env['CPPPATH'], CPPFLAGS=flags )
    Program('server', ['server.cpp'],     LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH=env['LIBPATH'], CPPPATH=env['CPPPATH'], CPPFLAGS=flags )

