import gdb
import re
import os

from collections import OrderedDict

FN_REGEX = r'[0-9]+:\s+(\w+)\s(\w+)\(((?:\([^()]*\)|.)*)\)'
DIRECTORY = 'hal/stm32f0xx/Src'
OUTFILE = 'hal/hal_shim.c'

functions = list()


def parse_params(param_string):
    '''
    Convert a string representation of all parameters to a OrderedDict
    with name as key and type as value.
    '''
    params = param_string.split(',')
    if params[0] == 'void':
        return {}

    ret = OrderedDict()
    for i, name in enumerate(range(97, 97 + len(params))):
        if params[i].strip().startswith('void'):
            print(params[i])
        ret[chr(name)] = params[i]

    return ret


class ShimFunction:
    '''Represent a shim function, important bit is the string representation'''
    def __init__(self, ret_type, name, params):
        self._ret = ret_type
        self._name = name
        self._params = parse_params(params)

    def __str__(self):
        fn_call = '%s(%s);' % (self._name, ', '.join(self._params.keys()))
        ret = 'return ' if self._ret != 'void' else ''

        body = ('\tif (l4)\n'
                '\t\t{0}l4_{1}\n'
                '\telse\n'
                '\t\t{0}f0_{1}\n').format(ret, fn_call)

        params = str()
        first = True
        for key in self._params.keys():
            if not first:
                params += ', '
            params += '%s %s' % (self._params[key], key)
            first = False

        return '__weak %s %s(%s) {\n%s}\n\n' % (self._ret,
                                                self._name,
                                                params,
                                                body)


def write_shim():
    '''
    Write a c file with shim functions.

    Example:
        HAL_Tick_Init() { ... } turns into:
        HAL_Tick_Init() {
            if (l4)
                l4_Hal_Tick_Init();
            else
                f0_Hal_Tick_Init();
        }
    '''
    with open(OUTFILE, 'w') as f:
        f.write('#include <stm32f0xx_hal.h>\n')
        f.write('static int l4 = 0;\n')

        for fn in functions:
            f.write(str(fn))


def process_file(f):
    ''' Parse the info functions command from GDB and create
        ShimFunction objects for each function found.
    '''
    gdb.execute('file %s' % f, False, False)
    info = gdb.execute('info functions', False, True)

    for line in info.splitlines():
        m = re.search(FN_REGEX, line)
        if m:
            fn = ShimFunction(m.group(1), m.group(2), m.group(3))
            functions.append(fn)


gdb.execute('set pagination off')
for f in os.listdir(DIRECTORY):
    if f.endswith('.o'):
        process_file(os.path.join(DIRECTORY, f))

write_shim()

gdb.execute('quit')
