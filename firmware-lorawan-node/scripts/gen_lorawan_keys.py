
import os
from pathlib import Path

# Tenta importar load_dotenv, mas continua mesmo se não estiver disponível
try:
    from dotenv import load_dotenv
except ImportError:
    load_dotenv = None

################################## Função para carregar variáveis do .env ou do ambiente ##############################
def load_env_file() -> None:
    env_path = ".." / Path(os.getcwd()) / ".env"
    if env_path.exists() and load_dotenv is not None:
        load_dotenv(dotenv_path=env_path)
        print(f"✅ Variáveis carregadas de {env_path}")
    elif env_path.exists():
        print(f"ℹ️  .env encontrado (python-dotenv não disponível; usando variáveis de ambiente do sistema)")
    else:
        print(f"⚠️  Arquivo .env não encontrado em {env_path}")

################################## Função para converter string hexadecimal em array C ##############################
def hexstr_to_c_array(hexstr:str, size:int) -> str:
    hexstr = hexstr.strip().replace(' ', '').upper()
    hexstr = hexstr.zfill(size * 2)
    return ', '.join(f'0x{hexstr[i:i+2]}' for i in range(0, size * 2, 2))

###################################### Carrega variáveis do .env ou do ambiente ###############################
load_env_file()
APPEUI = os.getenv('LORAWAN_APPEUI')
DEVEUI = os.getenv('LORAWAN_DEVEUI')
APPKEY = os.getenv('LORAWAN_APPKEY')

######################################### Define tamanhos e valores para as chaves #########################################
sizes = {
    'APPEUI': 8, 
    'DEVEUI': 8, 
    'APPKEY': 16
}
values = {
    'APPEUI': APPEUI, 
    'DEVEUI': DEVEUI, 
    'APPKEY': APPKEY
}

######################################### Gera conteúdo do header #########################################
header = '''
#pragma once
#include <stdint.h>
'''
for key in ['APPEUI', 'DEVEUI', 'APPKEY']:
    if values[key]:
        arr = hexstr_to_c_array(values[key], sizes[key])
        header += f'const uint8_t PROGMEM {key}[{sizes[key]}] = {{ {arr} }};\n'

########################################### Escreve o header no arquivo #########################################
header_path = Path(os.getcwd()) / 'src' / 'tasks' / 'protocol' / 'lorawan_keys.hpp'
with open(header_path, 'w') as f:
    f.write(header)
print(f"Header generated at {header_path}")
