import argparse
import json
import os
import shlex
import subprocess
from io import StringIO
from os import path

# 0x200000 -> 2MB para firmware e 2MB para SPIFFS
ADDRESS_SPIFFS = "0x210000"
TMP_FILE_NAME = "fs.out"
ARDUINO_CLI_COMMAND_CONFIG = "arduino-cli config dump --format json"
ARDUINO_CLI_COMMAND_COMPILE = "arduino-cli compile -b esp32:esp32:esp32:PartitionScheme=no_ota {Sketch} --verbose"
ARDUINO_CLI_COMMAND_UPLOAD = "arduino-cli upload -p {Port} -b esp32:esp32:esp32:PartitionScheme=no_ota {Sketch} --verbose"
MKSPIFFS_ARGS = " -c {Sketch}/data -p 256 -b 4096 -s 1966080 " + TMP_FILE_NAME
ESPTOOL_COMMAND_WRITE = "python -m esptool -p {Port} -b 115200 write_flash {Address} {File}"
COMMAND_SITE_DATA = "python compress_data.py web -c {Sketch}/site_data.h"

def run_command(command_string, show_output=False):
    proc = subprocess.Popen(
        command_string.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if show_output:
        for line in proc.stdout:
            print(line.decode('utf-8'), end='')
        proc.wait()
    else:
        status = ['\\', '|', '/', '_']
        count = 0
        while True:
            try:
                proc.communicate(timeout=0.1)
                print('\r', end='')
                break
            except subprocess.TimeoutExpired:
                print('\r', end='')
                print(status[count], end='')
                count = count + 1
                count = count % len(status)
    
    if proc.returncode != 0:
        print('Processo falhou com código', proc.returncode)
    else:
        return True

def get_arduino_config():
    """
    Obtém as configurações de arduino-cli e as retorna um dicionário.
    A função tenta executar o programa arduino-cli que deve estar no
    caminho do sistema.

    Retorna:
    dict: Um dicionário contendo a configuração reportada por arduino-cli config dump --format json
    """
    try:
        proc = subprocess.run(shlex.split(ARDUINO_CLI_COMMAND_CONFIG), capture_output=True)
        io = StringIO(proc.stdout.decode('utf-8'))
        return json.load(io)
    except FileNotFoundError:
        raise FileNotFoundError('arduino-cli não encontrado')

def find_mkspiffs_bin(root_dir, recursive=False):
    """
    Retorna o caminho absoluto do executável mkspiffs.

    Parâmetros:
    root_dir (string): O diretório base por onde começar a procurar.
    recursive (bool): Uso interno para permitir a recursão da função.

    Retorna:
    string: O caminho absoluto do executável mkspiffs.
    """
    if not recursive:
        mkspiffs_dir = path.join(root_dir, 'packages/esp32/tools/mkspiffs')
        mkspiffs_dir = path.abspath(path.normpath(mkspiffs_dir))

        if not path.isdir(mkspiffs_dir):
            raise FileNotFoundError('mkspiffs não encontrado')

        return find_mkspiffs_bin(mkspiffs_dir, True)
    else:
        for sub in os.scandir(root_dir):
            if sub.is_dir():
                return find_mkspiffs_bin(sub.path, True)
            elif sub.is_file() and sub.name.find('mkspiffs') >= 0:
                return sub.path
            else:
                raise FileNotFoundError('mkspiffs não encontrado')

def run_mkspiffs(mkspiffs_path, sketch):
    """
    Executa o programa mkspiffs como: 
    
    mkspiffs -c esp8266_firebase/data -p 256 -b 8192 -s 1048576 fs.out
    """
    try:
        run_command(mkspiffs_path + MKSPIFFS_ARGS.format(Sketch=sketch), True)
    except subprocess.CalledProcessError:
        raise Exception('Erro ao criar sistema de arquivos com mkspiffs.')

def clean_up_temp_files():
    if path.isfile(TMP_FILE_NAME):
        os.remove(TMP_FILE_NAME)

def build_commands():
    parser = argparse.ArgumentParser(description='Manage build, data and upload')
    parser.add_argument('sketch', help='sketch name')
    subparsers = parser.add_subparsers(dest='cmd')

    build_parser = subparsers.add_parser('build', help='build program')
    build_parser.add_argument('-v', '--verbose', help='show verbose output', action='store_true')

    upload_parser = subparsers.add_parser('upload', help='upload the program')
    upload_parser.add_argument('port', help='port name')

    config_parser = subparsers.add_parser('config', help='generate and upload config data to board')
    config_parser.add_argument('port', help='port name name')

    gen_parser = subparsers.add_parser('gen', help='generate site_data.h from web folder')

    return parser

def main():
    parser = build_commands()
    args = parser.parse_args()
    if args.cmd == 'build':
        print('Compilando sketch...')
        if run_command(ARDUINO_CLI_COMMAND_COMPILE.format(Sketch=args.sketch), args.verbose):
            print('Sucesso')
    elif args.cmd == 'upload':
        print('Gravando sketch...')
        run_command(ARDUINO_CLI_COMMAND_UPLOAD.format(Port=args.port, Sketch=args.sketch), True)
    elif args.cmd == 'config':
        # Obtém diretório data do arduino-cli
        print('Obtendo configurações de arduino-cli...')
        config = get_arduino_config()['config']

        # Encontra o executável de mkspiffs a partir do diretório data
        # e cria sistema de arquivos
        mkspiffs_path = find_mkspiffs_bin(config['directories']['data'])
        print('Econtrado mkspiffs em', mkspiffs_path)
        print('Criando sistema de arquivos spiffs...')
        run_mkspiffs(mkspiffs_path, args.sketch)

        # Usa esptool para gravar o arquivo gerado por mkspiffs no microcontrolador
        print('Gravando sistema de arquivos spiffs...')
        run_command(ESPTOOL_COMMAND_WRITE.format(Port=args.port, Address=ADDRESS_SPIFFS, File=TMP_FILE_NAME), True)
    elif args.cmd == 'gen':
        print('Gerando site_data.h...')
        run_command(COMMAND_SITE_DATA.format(Sketch=args.sketch), True)
    else:
        parser.print_help()

if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as e:
        print('Erro ao tentar executar arduino-cli. Código', e.returncode)
        print(e)
    except FileNotFoundError as e:
        print(e)
    except Exception as e:
        print(e)
    finally:
        clean_up_temp_files()
