#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "lib/font.h"
#include "lib/ssd1306.h"
#include "lib/icons.h"
#include "traffic_light.pio.h"

#define led_pin_red 13
#define led_pin_blue 12
#define led_pin_green 11

#define botaoA 5
#define buzzer_a_pin 10   // Buzzer A

#define num_pixels 25 // Número de pixels da matriz de LEDs
#define matrix_pin 7 // Pino da matriz de LEDs

#define I2C_SDA 14 // SSD1306: SDA
#define I2C_SCL 15 // SSD1306: SCL
#define I2C_PORT i2c1 // Porta I2C
#define endereco 0x3C // Endereço I2C do display SSD1306
ssd1306_t ssd; // Estrutura para o display SSD1306

volatile bool modo_noturno = 0; // 0 = normal, 1 = noturno
volatile uint8_t estado_semaforo = 0; // 0 = vermelho, 1 = verde, 2 = amarelo

// Variáveis para controle de cor e ícone exibido na matriz de LEDs
double red = 255.0, green = 255.0 , blue = 255.0; // Variáveis para controle de cor
int icon = 0; //Armazena o número atualmente exibido
double* icons[4] = {icon_zero, icon_one, icon_two, icon_three}; //Ponteiros para os desenhos dos números

// Função para configurar o PWM
void pwm_setup_gpio(uint gpio, uint freq) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);  // Define o pino como saída PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio);  // Obtém o slice do PWM

    if (freq == 0) {
        pwm_set_enabled(slice_num, false);  // Desabilita o PWM
        gpio_put(gpio, 0);  // Desliga o pino
    } else {
        uint32_t clock_div = 4; // Define o divisor do clock
        uint32_t wrap = (clock_get_hz(clk_sys) / (clock_div * freq)) - 1; // Calcula o valor de wrap

        // Configurações do PWM (clock_div, wrap e duty cycle) e habilita o PWM
        pwm_set_clkdiv(slice_num, clock_div); 
        pwm_set_wrap(slice_num, wrap);  
        pwm_set_gpio_level(gpio, wrap / 5);  
        pwm_set_enabled(slice_num, true);  
    }
}

// Rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R, G, B;
    R = r * red;
    G = g * green;
    B = b * blue;
    return (G << 24) | (R << 16) | (B << 8);
}

// Rotina para acionar a matrix de leds - ws2812b
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < num_pixels; i++) {
        valor_led = matrix_rgb(desenho[24-i], desenho[24-i], desenho[24-i]);
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Variáveis para controle do buzzer
void vBuzzerTask() {
    pwm_setup_gpio(buzzer_a_pin, 0); // Desabilita o buzzer
    while (true) {
        if (!modo_noturno) {
            pwm_setup_gpio(buzzer_a_pin, 1000); // Habilita o buzzer com frequência de 1kHz
            for (int i = 0; i < 100; i++) {
                if (modo_noturno) break;
                if (estado_semaforo == 0) { vTaskDelay(pdMS_TO_TICKS(5.0)); } else if (estado_semaforo == 1) { vTaskDelay(pdMS_TO_TICKS(1.0)); } else {  vTaskDelay(pdMS_TO_TICKS(1.0)); }
            }
            pwm_setup_gpio(buzzer_a_pin, 0); // Desabilita o buzzer
            for (int i = 0; i < 100; i++) {
                if (modo_noturno) break;
                if (estado_semaforo == 0) { vTaskDelay(pdMS_TO_TICKS(15)); } else if (estado_semaforo == 1) { vTaskDelay(pdMS_TO_TICKS(9.0)); } else {  vTaskDelay(pdMS_TO_TICKS(1.0)); }
            }

        } else {
            pwm_setup_gpio(buzzer_a_pin, 1000); //Habilita o buzzer com frequência de 1kHz
            for (int i = 0; i < 100; i++) {
                if (!modo_noturno) break;
                vTaskDelay(pdMS_TO_TICKS(7.5));
            }
            pwm_setup_gpio(buzzer_a_pin, 0); // Desabilita o buzzer
            for (int i = 0; i < 100; i++) {
                if (!modo_noturno) break;
                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }
    }
}

// Rotina para exibir informações no display
void vDisplayTask() {
    while (true) {
        bool cor = true;

        // Desenha o semáforo
        ssd1306_fill(&ssd, !cor);  
        ssd1306_vline(&ssd, 28, 0, 7, cor);
        ssd1306_rect(&ssd, 7, 10, 35, 50, cor, !cor);
        if (estado_semaforo == 0 && !modo_noturno) { ssd1306_rect(&ssd, 12, 15, 10, 10, cor, cor); } else {  ssd1306_rect(&ssd, 12, 15, 10, 10, cor, !cor); }
        if (estado_semaforo == 1 && !modo_noturno) { ssd1306_rect(&ssd, 27, 15, 10, 10, cor, cor); } else { ssd1306_rect(&ssd, 27, 15, 10, 10, cor, !cor); }
        if (estado_semaforo == 2 && !modo_noturno) { ssd1306_rect(&ssd, 42, 15, 10, 10, cor, cor); } else { ssd1306_rect(&ssd, 42, 15, 10, 10, cor, !cor); }
        ssd1306_draw_string(&ssd, "R", 30, 15);
        ssd1306_draw_string(&ssd, "G", 30, 30);
        ssd1306_draw_string(&ssd, "Y", 30, 45);
        ssd1306_vline(&ssd, 28, 56, 63, cor); 

        // Desenha as informações do semáforo
        ssd1306_rect(&ssd, 0, 50, 78, 63, cor, !cor);
        ssd1306_draw_string(&ssd, "Modo: ", 55, 3); 
        ssd1306_draw_string(&ssd, modo_noturno ? "Noturno" : "Normal", 55, 13); 

        ssd1306_draw_string(&ssd, "Cor: ", 55, 23); 
        if (estado_semaforo == 0 && !modo_noturno) { ssd1306_draw_string(&ssd, "Vermelho", 55, 33); } else if (estado_semaforo == 1 && !modo_noturno) { ssd1306_draw_string(&ssd, "Verde", 55, 33); } else { ssd1306_draw_string(&ssd, "Amarelo", 55, 33); }

        ssd1306_draw_string(&ssd, "Estado:", 55, 43);
        if (estado_semaforo == 0 && !modo_noturno ) { ssd1306_draw_string(&ssd, "Pare", 55, 53); } else if (estado_semaforo == 1 && !modo_noturno) { ssd1306_draw_string(&ssd, "Atravesse", 55, 53); } else { ssd1306_draw_string(&ssd, "Atencao", 55, 53); }

        ssd1306_send_data(&ssd); // Envia os dados para o display
        vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda 100ms
    }
}

// Rotina para exibir informações na matriz de LEDs
void vMatrixTask() {
    while (true) {
        if (!modo_noturno) {
            if (estado_semaforo == 0) {
                red = 255.0; green = 0.0; blue = 0.0; // Vermelho
                desenho_pio(icon_zero, 0, pio0, 0, red, green, blue);
            } else if (estado_semaforo == 1) {
                red = 0.0; green = 255.0; blue = 0.0; // Verde
                desenho_pio(icon_one, 0, pio0, 0, red, green, blue);
            } else if (estado_semaforo == 2) {
                red = 255.0; green = 255.0; blue = 0.0; // Amarelo
                desenho_pio(icon_two, 0, pio0, 0, red, green, blue);
            }
        } else {
            red = 255.0; green = 150.0; blue = 0.0; // Dourado
            desenho_pio(icon_three, 0, pio0, 0, red, green, blue);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda 100ms
    }
}

// Desenha o número correspondente ao estado do semáforo
void vLedsTask()
{
    // Configuração dos pinos dos LEDs
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_init(led_pin_blue);
    gpio_set_dir(led_pin_blue, GPIO_OUT);
    gpio_init(led_pin_green);
    gpio_set_dir(led_pin_green, GPIO_OUT);

    while (true)
    {
        if (!modo_noturno) { // Altera os LEDs se não estiver no modo noturno
            if (estado_semaforo == 0) {
                gpio_put(led_pin_red, 1);
                gpio_put(led_pin_green, 0);
            } else if (estado_semaforo == 1) {
                gpio_put(led_pin_red, 0);
                gpio_put(led_pin_green, 1);
            } else if (estado_semaforo == 2) {
                gpio_put(led_pin_red, 1);
                gpio_put(led_pin_green, 1);
            }

            if (estado_semaforo != 2) {
                for (int i = 0; i < 100; i++) {
                    if (modo_noturno) break;
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
            } else {
                for (int i = 0; i < 100; i++) {
                    if (modo_noturno) break;
                    vTaskDelay(pdMS_TO_TICKS(25));
                }
            }
            gpio_put(led_pin_red, 0);
            gpio_put(led_pin_green, 0);
            estado_semaforo = (estado_semaforo + 1) % 3;
        } else { 
            // Altera os LEDs se estiver no modo noturno
            gpio_put(led_pin_red, 1);
            gpio_put(led_pin_green, 1);
            for (int i = 0; i < 100; i++) {
                if (!modo_noturno) break;
                vTaskDelay(pdMS_TO_TICKS(7.5));
            }
            gpio_put(led_pin_red, 0);
            gpio_put(led_pin_green, 0);
            for (int i = 0; i < 100; i++) {
                if (!modo_noturno) break;
                vTaskDelay(pdMS_TO_TICKS(20));
            }
            
        }
    }
}

// Rotina para alternar entre os modos noturno e normal conforme o botão A
void vButtonTask() {
    while (true) {
        if (!gpio_get(botaoA)) {
            modo_noturno = !modo_noturno;
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);
    gpio_init(buzzer_a_pin);  
    gpio_set_dir(buzzer_a_pin, GPIO_OUT);

    //Configurações da PIO
    PIO pio = pio0; 
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, matrix_pin);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line                          
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configuração do botão B para modo BOOTSEL
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    xTaskCreate(vDisplayTask, "Display Task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL); // Cria a task do display
    xTaskCreate(vLedsTask, "Leds Task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL); // Cria a task dos LEDs
    xTaskCreate(vButtonTask, "Button Task",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL); // Cria a task do botão
    xTaskCreate(vBuzzerTask, "Buzzer Task",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL); // Cria a task do buzzer
    xTaskCreate(vMatrixTask, "Matrix Task",
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL); // Cria a task da matriz de LEDs
    
    vTaskStartScheduler();
    panic_unsupported();
}
