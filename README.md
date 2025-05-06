# Semáforo Interativo 🚦

Este é um projeto desenvolvido para a placa de desenvolvimento BitDogLab, baseada no microcontrolador Raspberry Pi Pico W.
O objetivo é simular o funcionamento de um semáforo veicular tradicional, utilizando LEDs, matriz e display para indicar o estado atual do semáforo, e buzzers para acessibilidade.

---

## 📌 Sobre o Projeto

O Semáforo Interativo foi desenvolvido como parte das atividades da 2ª fase da residência tecnológica EmbarcaTech.
O projeto tem como foco principal o uso do FreeRTOS, implementando múltiplas tarefas a serem executadas simultâneamente pelo sistema, visando simular um semáforo através dos periféricos da BitDogLab.

---

## 🧪 Como funciona

O sistema simula um semáforo clássico com três estados:

-   Vermelho: LED RGB acende vermelho por 5 segundos.

-   Verde: LED RGB acende verde por 5 segundos.

-   Amarelo: LED RGB acende amarelo por 2.5 segundos.

Além disso, em cada um dos estados, são mostradas informações de estado do sistema no display, bem como ícones na matriz de LEDs e sinais em intervalos específicos emitidos por um buzzer.

Esses estados se repetem continuamente de forma automática.

A lógica é controlada via tarefas, sendo cada uma responsável por diferentes periféricos.

---

## 📁 Utilização

Atendendo aos requisitos de organização da 2º fase da residência, o arquivo CMakeLists.txt está configurado para facilitar a importação do projeto no Visual Studio Code.  
Segue as instruções:

1. Na barra lateral, clique em **Raspberry Pi Pico Project** e depois em **Import Project**.

![image](https://github.com/user-attachments/assets/4b1ed8c7-6730-4bfe-ae1f-8a26017d1140)

2. Selecione o diretório do projeto e clique em **Import** (utilizando a versão **2.1.1** do Pico SDK).

![image](https://github.com/user-attachments/assets/be706372-b918-4ade-847e-12706af0cc99)

3. Além disso, dada a necessidade da utilização do **FreeRTOS**, se faz necessária a instalação do mesmo e alteração do **CMakeLists.txt** para indicar o seu caminho.

![image](https://github.com/user-attachments/assets/7271e3aa-405c-4e24-9022-8a4e1ea98f97)

4. Agora, basta **compilar** e **rodar** o projeto, com a placa **BitDogLab** conectada.

---
