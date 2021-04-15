import gdb
import re
import os

from collections import OrderedDict

FN_REGEX = r'[0-9]+:\s+(\w+)\s(\w+)\(((?:\([^()]*\)|.)*)\)'
DIRECTORY = 'hal/stm32f0xx/Src'
OUTFILE = 'hal/hal_shim.c'

functions = list()

def specialHAL_DMA_RegisterCallback():
    return ('HAL_StatusTypeDef HAL_DMA_RegisterCallback(DMA_HandleTypeDef *hdma, HAL_DMA_CallbackIDTypeDef CallbackID, void (* pCallback)( DMA_HandleTypeDef * _hdma))\n'
            '{\n'
            '   if (isL4) {\n'
            '       return HAL_DMA_RegisterCallback_l4(hdma, CallbackID, pCallback);\n'
            '   } else {\n'
            '       return HAL_DMA_RegisterCallback_f0(hdma, CallbackID, pCallback);\n'
            '   }\n'
            '}\n\n')

blacklist = [
    'HAL_MspInit',
    'HAL_GPIO_EXTI_Callback',
    'HAL_I2C_MspInit',
    'HAL_I2C_MspDeInit',
    'HAL_UART_MspInit',
    'HAL_UART_MspDeInit',
    'HAL_SPI_MspInit',
    'HAL_SPI_MspDeInit',
    'HAL_PCD_ConnectCallback',
    'HAL_PCD_DataInStageCallback',
    'HAL_PCD_DisconnectCallback',
    'HAL_PCD_ISOINIncompleteCallback',
    'HAL_PCD_ISOOUTIncompleteCallback',
    'HAL_PCD_MspDeInit',
    'HAL_PCD_ResetCallback',
    'HAL_PCD_ResumeCallback',
    'HAL_PCD_SOFCallback',
    'HAL_PCD_SetupStageCallback',
    'HAL_PCD_SuspendCallback',
    'HAL_PCD_DataOutStageCallback',
    'HAL_PCD_MspInit',
    'HAL_PCDEx_LPM_Callback'
]

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
        self.name = name
        self._params = parse_params(params)

    def __str__(self):
        fn_call = '%s(%s);' % (self.name, ', '.join(self._params.keys()))
        ret = 'return ' if self._ret != 'void' else ''

        body = ('\tif (isL4)\n'
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

        return '%s %s(%s) {\n%s}\n\n' % (self._ret,
                                                self.name,
                                                params,
                                                body)


def write_shim():
    '''
    Write a c file with shim functions.

    Example:
        HAL_Tick_Init() { ... } turns into:
        HAL_Tick_Init() {
            if (isL4)
                l4_Hal_Tick_Init();
            else
                f0_Hal_Tick_Init();
        }
    '''
    with open(OUTFILE, 'w') as f:
        f.write('#include <stm32f0xx_hal.h>\n')
        f.write('extern int isL4;\n')

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
            if fn.name not in blacklist:
                if fn.name == 'HAL_DMA_RegisterCallback':
                    fn.__str__ = specialHAL_DMA_RegisterCallback
                functions.append(fn)


gdb.execute('set pagination off')
for f in os.listdir(DIRECTORY):
    if f.endswith('.o'):
        process_file(os.path.join(DIRECTORY, f))

write_shim()

gdb.execute('quit')
