# -*- coding: utf-8 -*-
import os
import sys
import subprocess
from sysconfig import get_paths, get_config_vars

FILES = [('', 'nextflap.cpp'), ('parser', 'parser.cpp'), ('parser', 'syntaxAnalyzer.cpp'),
         ('parser', 'parsedTask.cpp'), ('preprocess', 'preprocess.cpp'),
         ('preprocess', 'preprocessedTask.cpp'), ('grounder', 'grounder.cpp'),
         ('grounder', 'groundedTask.cpp'), ('heuristics', 'evaluator.cpp'),
         ('heuristics', 'hFF.cpp'), ('heuristics', 'hLand.cpp'), ('heuristics', 'landmarks.cpp'),
         ('heuristics', 'numericRPG.cpp'), ('heuristics', 'rpg.cpp'), ('heuristics', 'temporalRPG.cpp'),
         ('planner', 'intervalCalculations.cpp'), ('planner', 'linearizer.cpp'), ('planner', 'plan.cpp'),
         ('planner', 'planBuilder.cpp'), ('planner', 'planComponents.cpp'), ('planner', 'planEffects.cpp'),
         ('planner', 'planner.cpp'), ('planner', 'plannerSetting.cpp'), ('planner', 'printPlan.cpp'),
         ('planner', 'selector.cpp'), ('planner', 'state.cpp'), ('planner', 'successors.cpp'),
         ('planner', 'z3Checker.cpp'), ('sas', 'mutexGraph.cpp'), ('sas', 'sasTask.cpp'),
         ('sas', 'sasTranslator.cpp'), ('utils', 'utils.cpp'), ('', 'up_nextflap.py')]

def error(msg):
    raise Exception(msg)

def checkInputFiles():
    for folder, file in FILES:
        if folder == '':
            path = file
        else:
            path = os.path.join(folder, file)
        if not os.path.exists(path):
            error('file "' + path + '" not found.')

def getPybindFolder():
    try:
        import pybind11
    except:
        raise Exception('pybind11 module not found.\nTry installing it using the following command: pip install pybind11')
    path = pybind11.__file__.split(os.sep)[:-1]
    path.append('include')
    path.append('pybind11')
    folder = path[0] + os.sep
    for i in range(1, len(path)):
        folder = os.path.join(folder, path[i])
    header = os.path.join(folder, 'pybind11.h')
    if not os.path.exists(header):
        error(f'check the pybind11 installation. File {header} not found.')
    print('* pybind11 module found in', folder)
    return folder    
        
def getZ3Folder():
    print('If you do not have Z3 installed, download the appropriate asset from https://github.com/Z3Prover/z3/releases and unzip it.')
    folder = input('Write the path of Z3: ')
    folder = folder.replace('\\', os.sep)
    folder = folder.replace('/', os.sep)
    z3IncludeFolder = os.path.join(folder, 'include')
    if not os.path.exists(os.path.join(z3IncludeFolder, 'z3++.h')):
        error('z3++.h file not found in ' + z3IncludeFolder)
    z3LibFolder = os.path.join(folder, 'lib')
    if not os.path.exists(z3LibFolder):
        z3LibFolder = os.path.join(folder, 'bin')
    if not os.path.exists(z3LibFolder):
        error('lib or bin folder not found in ' + folder)
    return z3IncludeFolder, z3LibFolder

def getPythonFolder():
    info = get_paths()
    pvars = get_config_vars()
    folder = ''
    if 'include' in info and os.path.exists(info['include']):
        folder = info['include']
    elif 'platinclude' in info and os.path.exists(info['platinclude']):
        folder = info['platinclude']
    else:
        folder = input('Python include folder not found. Enter the path: ')
        folder = folder.replace('\\', os.sep)
        folder = folder.replace('/', os.sep)
    if not os.path.exists(folder):
        error('invalid Python include path')
    libsPath = os.path.join(os.sep.join(folder.split(os.sep)[:-1]), 'libs') + os.sep
    pythonLib = 'python' + pvars['py_version_nodot']
    return folder, libsPath, pythonLib
        
def getPlatform():
    platform = sys.platform.lower()
    if platform.startswith('win'):
        return "windows"
    if platform.startswith('darw'):
        return "mac"
    return "linux"
    
def getZ3Library(z3LibFolder, platform):
    libFile = {"windows": "libz3.dll", "mac": "libz3.dylib", "linux": "libz3.so"}
    path = os.path.join(z3LibFolder, libFile[platform])
    if not os.path.exists(path):
        error(f'library {path} not found')
    return path

def compilePlanner():
    pybindFolder = getPybindFolder()
    pythonFolder, pythonLibPath, pythonLib = getPythonFolder()
    z3IncludeFolder, z3LibFolder = getZ3Folder()
    platform = getPlatform()
    z3Lib = getZ3Library(z3LibFolder, platform)
    CFLAGS = f'-c -Wall -std=c++20 -O3 -Wextra -pedantic -fPIC -I{z3IncludeFolder} -I{pythonFolder} -I{pybindFolder}'
    print('Compiling...')
    for folder, file in FILES:
        if file.endswith('.cpp'):
            print(f'* {file}')
            params = ['g++']
            params.extend(CFLAGS.split())
            if folder == '':
                params.append(file)
            else:
                params.append(folder + os.sep + file)
            try:
                process = subprocess.run(params, capture_output=True)
                compiledFile = file[:-3] + 'o'
            except:
                error(params[0] + ' not found')
            if process.returncode != 0 or not os.path.exists(compiledFile):
                print(process.stdout.decode('UTF-8'))
                print(process.stderr.decode('UTF-8'))
                error(f'could not compile {params[-1]}')
    print('Linking...')
    params = ['g++', '-shared', '-lm']
    if platform == 'linux':
        pvars = get_config_vars()
        params.append("-Wl,-rpath,$$ORIGIN")
        params.append(f"-L{pvars['LIBPL']}")
        params.append(f"-L{pvars['LIBDIR']}")
    if platform == 'mac':
        pvars = get_config_vars()
        params.extend(["-Wl,-rpath,@loader_path", "-ldl", "-framework", "CoreFoundation", "-undefined", "dynamic_lookup"])
        params.append(f"-L{pvars['srcdir']}")
    for _, file in FILES:
        if file.endswith('.cpp'):
            params.append(file[:-3] + 'o')
    params.append(z3Lib)
    params.append('-o')
    if platform == 'windows':
        outputFile = 'nextflap.pyd'
    else:
        outputFile = 'nextflap.so'
    params.append(outputFile)
    if platform == 'windows':
        params.append(f'-L{pythonLibPath}')
        params.append(f'-l{pythonLib}')
    try:
        process = subprocess.run(params, capture_output=True)
    except:
        error(params[0] + ' not found')
    if process.returncode != 0 or not os.path.exists(outputFile):
        print(process.stdout.decode('UTF-8'))
        print(process.stderr.decode('UTF-8'))
        error(f'could not link {outputFile}') 

try:
    print('NextFLAP installation')
    checkInputFiles()
    compilePlanner()
except Exception as err:
    print('Error:', err)
print('Done')
