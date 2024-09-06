# Sistemas Embarcados I (Projeto 1)
Este trabalho foi realizado para a cadeira de Sistemas Embarcados I do curso de Engenharia Elétrica da PUCRS. O projeto consiste em um servidor web para monitorar a temperatura e
umidade. O sensor utilizado é o DHT22.

## Ferramentas necessárias
O projeto usa como base o núcleo do ESP32 para Arduino. As seguintes dependências são necessárias:
- arduino-cli
- python 3
- esptool (instalar com o pip)

## Instalando dependências
É necessário instalar as ferramentas de compilação do ESP32 para o Arduino e as bibliotecas.
```shell
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli update
arduino-cli core install esp32:esp32
arduino-cli lib install "DHT Sensor Library"
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/me-no-dev/AsyncTCP.git
arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncWebServer.git
```

## Compilando o projeto
O projeto utiliza um layout de memória específico. As etapas abaixo devem ser seguidas para configurar o projeto corretamente.

### Gerando os arquivos web
O diretório web contém as páginas do servidor. É necessário gerar um arquivo de cabeçalho para gravar as páginas no ESP32.
```shell
python gravar.py T1 gen
```

### Arquivo de configuração
Dentro do diretório T1 há um modelo de arquivo de configuração. Nele devem constar informações como as credenciais do Wi-Fi, senha e login do administrador, etc. Crie uma cópia deste arquivo e renomeie para T1/data/config.txt. Então grave o novo sistema de arquivos como segue:
```shell
python gravar.py T1 config <Porta COM>
```

### Compilação
Digite:
```shell
python gravar.py T1 build
```

### Gravação
Digite:
```shell
pyhton gravar.py T1 upload <Porta COM>
```

## Funcionamento

### Estação
- Tenta conectar à rede Wi-Fi configurada e habilita o servidor web. A partir daí, é possível interagir com o ESP32 acessando as páginas no endereço exibido.

### Ponto de acesso
- Caso o botão conectado ao pino GPIO5 esteja pressionado durante sua inicialização, o ESP32 funcionará como um ponto de acesso, que pode aceitar estações em sua própria rede local. O servidor web também está disponível neste modo.

## Ferramentas alternativas
Também é possível utilizar o projeto com a IDE do Arduino (ou PlatformIO). Algumas observações devem ser feitas:

- O esquema de partição utilizado foi o no_ota (2MB app/2MB spiffs)
- Outros esquemas de partição podem ser utilizados desde que haja espaço para os arquivos estáticos web (aproximadamente 703kB comprimidos)
- O script gravar.py não grava os arquivos web no sistema de arquivos SPIFFS. Ao invés disso, gera um arquivo de cabeçalho site_data.h, acessível ao firmware como uma estrutura indexada
- Caso prefira utilizar SPIFFS, é possível remover a referência a sita_data.h e substituir as chamadas a sendGzipProgmem por sendGzipFile