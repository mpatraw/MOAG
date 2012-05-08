import os, glob

env = Environment(ENV={'PATH' : os.environ['PATH']}, CPPPATH='src')
env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa']
env.Append(CPPPATH = ['/opt/local/include/'])
env.Append(CCFLAGS='-Wall -pedantic -g -std=c99')
env.Append(LIBPATH='.')

env.Object(glob.glob('*.c'))

server_libs = ['enet', 'm']
client_libs = ['SDL', 'SDL_ttf', 'enet', 'm']

env.Program('client', ['client.o', 'common.o'], LIBS=client_libs)
env.Program('server', ['server.o', 'common.o'], LIBS=server_libs)
env.Program('test', ['test.o', 'common.o'])

