
#include <stdio.h>

#include "pico/stdlib.h"
#include "DTMFReceiver.h"

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    DTMFReceiver receiver;

    receiver.begin();

    printf("DTMF Receiver started\n\n");

    while (true)
    {
        char key = receiver.readKey();

        if (key != '\0')
        {
            printf("Ny tone: %c\n", key);

            printf("Samlet sekvens: %s\n\n",
                   receiver.getReceivedSequence());
        }

        sleep_ms(1);
    }

    return 0;
}
