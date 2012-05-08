import os, glob

env = Environment(ENV = {'PATH' : os.environ['PATH']})

env = Environment(CPPPATH='src')
env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa']

flags = '-Wall -pedantic -g -std=c99'

client_libs = ['SDL', 'SDL_ttf', 'enet']
client_src = glob.glob('*.c')
client_src.remove('server.c')
client_src.remove('test.c')

server_libs = ['enet']
server_src = glob.glob('*.c')
server_src.remove('common.c')
server_src.remove('client.c')
server_src.remove('test.c')

env.Append(CPPPATH = ['/opt/local/include/'])

Program('client', client_src, LIBS=client_libs, FRAMEWORKS=env['FRAMEWORKS'],
        LIBPATH='.', CPPPATH=env['CPPPATH'], CPPFLAGS=flags)

Program('server', server_src, LIBS=server_libs, FRAMEWORKS=env['FRAMEWORKS'],
        LIBPATH='.', CPPPATH=env['CPPPATH'], CPPFLAGS=flags)

Program('test', ['test.c'], FRAMEWORKS=env['FRAMEWORKS'],
        CPPPATH=env['CPPPATH'], CPPFLAGS=flags)

