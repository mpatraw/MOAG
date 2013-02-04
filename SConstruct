import os, glob, sys

AddOption('--platform',
          default='linux',
          dest='platform',
          type='string',
          nargs=1,
          action='store',
          metavar='PLATFORM',
          help='target platform (linux, mingw32, mingw32-linux)')

AddOption('--with-logging',
          default=False,
          dest='with-logging',
          action='store_true',
          help='enable logging (adds -DVERBOSE)')

client_objects = ['client.o', 'common.o', 'sdl_aux.o']
server_objects = ['server.o', 'common.o']

# NOTE: compiler flag -mno-ms-bitfields allows __attribute__((packed)) to work properly for gcc versions >= 4.7.0

if GetOption('platform') == 'linux':
    env = Environment(ENV={'PATH' : os.environ['PATH']})
    env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa']
    env.Append(CPPPATH = ['/opt/local/include/'])
    env.Append(CCFLAGS='-Wall -pedantic -g -std=c99 -mno-ms-bitfields -D_POSIX_C_SOURCE=199309L')
    if GetOption('with-logging'):
        env.Append(CCFLAGS='-DVERBOSE')
    env.Append(LIBPATH='.')

    env.Object(glob.glob('*.c'))

    server_libs = ['enet', 'z', 'm']
    client_libs = ['SDL', 'SDL_ttf', 'enet', 'z', 'm']
    # fix for Mac
    if ( sys.platform == 'darwin' ) : client_libs.append('SDLMain')

    env.Program('client', client_objects, LIBS=client_libs)
    env.Program('server', server_objects, LIBS=server_libs)
else:
    env = Environment(ENV={'PATH' : os.environ['PATH']})
    env['FRAMEWORKS'] = ['OpenGL', 'Foundation', 'Cocoa']
    env.Append(CPPPATH = ['/opt/local/include/'])
    env.Append(CCFLAGS='-Wall -pedantic -g -std=c99 -mno-ms-bitfields -D_POSIX_C_SOURCE=199309L -DWIN32')
    if GetOption('with-logging'):
        env.Append(CCFLAGS='-DVERBOSE')
    env.Append(LIBPATH='.')
    if GetOption('platform') == 'mingw32-linux':
        env.Replace(CC='i486-mingw32-gcc')
    else:
        env.Replace(CC='mingw32-gcc')

    env.Object(glob.glob('*.c'))

    server_libs = ['mingw32', 'SDL', 'enet', 'z', 'm', 'ws2_32', 'winmm']
    client_libs = ['mingw32', 'SDLmain', 'SDL', 'SDL_ttf', 'enet', 'z', 'm', 'ws2_32', 'winmm']

    env.Program('client.exe', client_objects, LIBS=client_libs)
    env.Program('server.exe', server_objects, LIBS=server_libs)
