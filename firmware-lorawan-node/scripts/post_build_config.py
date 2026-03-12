import os

# Configura o caminho para o arquivo de configuração do LMIC
CONFIG_PATH = os.path.join(
    os.getcwd(),
    ".pio",
    "libdeps",
    "heltec_wifi_lora_32_V2",
    "MCCI LoRaWAN LMIC library",
    "project_config",
    "lmic_project_config.h"
)

# Verifica se o arquivo de configuração existe
if os.path.exists(CONFIG_PATH):
    # Lê o conteúdo do arquivo de configuração
    with open(CONFIG_PATH, "r") as f:
        content = f.read()

    # Substitui CFG_us915 por CFG_au915
    content = content.replace("#define CFG_us915", "#define CFG_au915")
    with open(CONFIG_PATH, "w") as f:
        f.write(content)

    print("Configuração LMIC atualizada para AU915.")
else:
    print("Arquivo de configuração LMIC não encontrado.")
