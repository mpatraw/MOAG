import os, glob

env = Environment(ENV={'PATH' : os.environ['PATH']})
env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa']
env.Append(CPPPATH = ['/opt/local/include/'])
env.Append(CCFLAGS='-Wall -pedantic -g -std=c99 -D_POSIX_C_SOURCE=199309L')
env.Append(LIBPATH='.')

env.Object(glob.glob('*.c'))

server_libs = ['enet', 'z', 'm']
client_libs = ['SDL', 'SDL_ttf', 'enet', 'z', 'm']

env.Program('client', ['client.o', 'enet_aux.o', 'sdl_aux.o', 'common.o'], LIBS=client_libs)
env.Program('server', ['server.o', 'enet_aux.o', 'common.o'], LIBS=server_libs)

