import os
env = Environment(ENV = {'PATH' : os.environ['PATH']})

env = Environment(CPPPATH='src')
env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa'] 

flags = '-Wall -pedantic -g'
libs = ['SDL','SDL_net', 'SDL_ttf','moag']

Library('moag', Glob('moag*.c'), CPPFLAGS=flags)

env.Append(CPPPATH = ['/opt/local/include/'])
print env['CPPPATH']

Program('client', ['client.c'], LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH='.', CPPPATH=env['CPPPATH'], CPPFLAGS=flags)
Program('server', ['server.c'], LIBS=libs, FRAMEWORKS=env['FRAMEWORKS'], LIBPATH='.', CPPPATH=env['CPPPATH'], CPPFLAGS=flags)

