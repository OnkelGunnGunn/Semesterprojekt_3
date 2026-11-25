
#include <stdio.h>

#include "pico/stdlib.h"
#include "DTMFReceiver.h"

int main()
{
    stdio_init_all();

    sleep_ms(2000);

    // Opret DTMF-modtager
    DTMFReceiver receiver;

    // Initialiser MCP3002 og SPI
    receiver.begin();

    printf("\n");
    printf("============================\n");
    printf("     DTMF RECEIVER PICO 2   \n");
    printf("============================\n");
    printf("Sampling: 8000 Hz\n");
    printf("Listening...\n\n");

    while (true)
    {
        char key = receiver.readKey();

        if (key != '\0')
        {
            printf("DTMF registreret: %c\n", key);
        }
    }

    return 0;
}
