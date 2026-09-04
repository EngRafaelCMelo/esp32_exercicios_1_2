# Exercícios 1 e 2 - ESP32 com ESP-IDF, sem API do FreeRTOS

Projetos preparados para um **ESP32 DevKit V1**, usando somente APIs do ESP-IDF.
Cada exercício é um projeto independente e deve ser aberto/compilado separadamente.

O código da aplicação não inclui cabeçalhos do FreeRTOS nem cria tarefas, filas,
semáforos ou temporizadores do sistema operacional. Cada exercício possui somente um
laço principal em `app_main`, usando `usleep` para a pausa entre as iterações.

> O ESP-IDF usa o FreeRTOS internamente para inicializar o sistema e chamar `app_main`.
> Portanto, esta versão não usa a **API do FreeRTOS no código da aplicação**, mas não é
> um firmware bare-metal. Para eliminar o kernel também da infraestrutura seria
> necessário trocar o ESP-IDF por um ambiente bare-metal específico.

## Materiais

- 1 ESP32 DevKit V1
- 3 LEDs para o Exercício 1 (o Exercício 2 pode reutilizar um deles)
- 3 resistores de 220 a 330 ohms para limitar a corrente dos LEDs
- 2 botões normalmente abertos
- 1 potenciômetro de aproximadamente 10 kohms
- Protoboard e jumpers

## Exercício 1 - GPIO

### O que o programa faz

- Cada pressão no botão 1 alterna o LED 1 entre aceso e apagado.
- Cada pressão no botão 2 alterna o LED 2 entre aceso e apagado.
- O LED 3 pisca com período de 1 segundo (1 Hz: 500 ms aceso e 500 ms apagado).
- O monitor serial informa pressões, liberações e o estado atualizado dos LEDs.
- Os pinos são configurados com gpio_config, atendendo também à adaptação solicitada.
- Há debounce por software de 50 ms, evitando várias alternâncias em uma única pressão.

### Ligações

| Componente | Ligação ao ESP32 | Outra ligação |
| --- | --- | --- |
| LED 1 | GPIO18, passando por resistor de 220-330 ohms | Cátodo no GND |
| LED 2 | GPIO22, passando por resistor de 220-330 ohms | Cátodo no GND |
| LED 3 | GPIO23, passando por resistor de 220-330 ohms | Cátodo no GND |
| Botão 1 | GPIO2 | Outro terminal no GND |
| Botão 2 | GPIO4 | Outro terminal no GND |

Nos LEDs, ligue o terminal longo (ânodo) ao resistor que vem do GPIO e o terminal curto
(cátodo) ao GND. Os botões usam os resistores internos de pull-up; por isso, soltos são
lidos como nível 1 e pressionados como nível 0.

> Atenção: o GPIO2 é um pino de inicialização do ESP32 e foi mantido porque aparece no
> esqueleto fornecido. Não mantenha o botão 1 pressionado ao reiniciar ou gravar a placa.
> Se o professor permitir a troca, o GPIO19 é uma alternativa mais segura.

> Nesta montagem, o LED 1 foi transferido do GPIO21 para o GPIO18 porque o GPIO21 da
> placa utilizada apresentou falha elétrica durante os testes.

## Exercício 2 - ADC e PWM

### O que o programa faz

- Lê o potenciômetro pelo ADC2, canal 0 (GPIO4/D4), com resolução de 12 bits.
- Faz média de 32 amostras para reduzir o ruído.
- Converte a leitura para tensão usando a calibração de linha do ESP32.
- Converte a leitura de 0-4095 em duty cycle PWM de 0-100%.
- Controla o brilho do LED no GPIO21 pelo periférico LEDC.
- Exibe leitura bruta, tensão e duty cycle no monitor serial.

### Ligações

| Componente | Ligação ao ESP32 | Outra ligação |
| --- | --- | --- |
| Potenciômetro - terminal lateral 1 | 3V3 | - |
| Potenciômetro - terminal central | GPIO4 (D4) | - |
| Potenciômetro - terminal lateral 2 | GND | - |
| LED PWM | GPIO21, passando por resistor de 220-330 ohms | Cátodo no GND |

Se o sentido do potenciômetro ficar invertido, troque entre si os dois terminais laterais.
Use apenas 3,3 V: nunca aplique 5 V ao GPIO4. Como esse pino pertence ao ADC2, não use
Wi-Fi durante este exercício. Com a atenuação de 12 dB, a extremidade
superior do potenciômetro pode entrar na região de saturação do ADC antes dos 3,3 V; isso
não impede o controle de chegar a 100%, mas reduz a faixa útil próxima do final do giro.

## Compilar e gravar

Abra um terminal ESP-IDF e entre na pasta do exercício desejado. Para o ESP32 original:

    idf.py set-target esp32
    idf.py build
    idf.py -p PORTA flash monitor

No Windows, PORTA costuma ser algo como COM3. No Linux, costuma ser
/dev/ttyUSB0 ou /dev/ttyACM0. Para sair do monitor, pressione Ctrl+].

No VS Code com a extensão ESP-IDF, abra a pasta individual do exercício, selecione a
porta e use os comandos **Build**, **Flash** e **Monitor** da extensão.

## Alterar pinos

Os pinos ficam definidos no início de cada arquivo .c. Caso a sua montagem use outros
GPIOs, altere apenas os respectivos #define, respeitando as limitações do ESP32.
