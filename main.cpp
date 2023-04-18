#include "mbed.h"

Thread thread_master;
Thread thread_slave;

//master

SPI spi(D11, D12, D13); // mosi, miso, sclk
DigitalOut cs(D9);

SPISlave device(PD_4, PD_3, PD_1, PD_0); //mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int slave()
{
   device.format(16, 3);
   device.frequency(1000000);
   uint16_t container[256];
   uint16_t address;
   int sel = 0;
   int mode = 0;
   //device.reply(0x00); // Prime SPI with first reply
   while (1)
   {
      if (device.receive())
      {     
            if (!sel) {
                uint16_t v = device.read();
                printf("readdata = %x\n", v);
                mode = v>>8;
                printf("Mode: %d\n", mode);
                if ((v >> 8) == 1) // Write mode
                {                      //Verify the command
                    // Read address
                    address = (v & 0x00FF);
                    printf("Address from master: address = %0x\n", address);
                }
                else // Read mode
                {
                    printf("Reading!\n");
                    // Read address
                    address = v & 0x00FF;
                    printf("Address from master: address = %0x\n", address);
                    device.reply(container[address]); //Reply default value
                }
                sel = !sel;
            } else {
                if (mode == 1) // Write mode
                {                      //Verify the command
                    // Write data
                    container[address] = device.read();
                    printf("Container: %0x\n", container[address]);
                    led = !led;      // led turn blue/orange if device receive
                }
                else // Read mode
                {
                    address = device.read();
                }
                sel = !sel;
            }
      }
   }
}

void master()
{
   // Setup the spi for 8 bit data, high steady state clock,
   // second edge capture, with a 1MHz clock rate
   spi.format(16, 3);
   spi.frequency(1000000);
      // Chip must be deselected
      cs = 1;
      // Select the device by seting chip select low
      cs = 0;

      printf("Send Write.\n");
      spi.write(0x0177); // Send Address
      ThisThread::sleep_for(100ms);
      spi.write(0xa0FF); // Send Data
      cs = 1;                       // Deselect the device
      ThisThread::sleep_for(100ms); //Wait for debug print

      // Select the device by seting chip select low
      cs = 0;
      printf("Read mode\n");
      spi.write(0x0077); //Send Address
      ThisThread::sleep_for(100ms); //Wait for debug print
      uint16_t response = spi.write(0x0000); // Send Data
      ThisThread::sleep_for(100ms); //Wait for debug print
      printf("response from slave = %x\n", response);
      cs = 1; // Deselect the device
}

int main()
{
   thread_slave.start(slave);
   thread_master.start(master);
}
