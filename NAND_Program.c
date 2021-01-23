#include "msp.h"




#define DEV_ID 0x9F

#define WINBOND_MAN_ID 0xEF

#define READ_STATUS_REG 0x0F

#define WRITE_STATUS_REG 0x1F

#define WRITE_EN 0x06

#define BLOCK_MAN 0xA1

#define READ_BBM 0xA5

#define LAST_ECC_FAIL 0xA9

#define BLOCK_ERASE 0xD8

#define PROG_DATA_LOAD 0x02

#define PROG_EXECUTE 0x10

#define PAGE_READ_DATA 0x13

#define READ_DATA 0x03

#define PROT_REG 0xA0

#define CONFIG_REG 0xB0

#define STAT_REG 0xC0

#define MAX_PAGE 65535

#define MAX_COL 2112


void send_data(char *buf, int length);
void write_enable();
void setstatus(char reg);
void get_devid();
void block_erase(uint16_t page_add);
void load_data(uint16_t col_add);
void prog_exe(uint16_t page_add);
void pagedataread(uint16_t page_add);
void read(uint16_t col_add);
int fill_buffer();
void  delay_ms(uint8_t d);



char bfr[];

int i;
int recieve_data=0;
int data=0;
char buffer[2048];

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    CS->KEY = 0x695A;
    CS->CTL1 |= CS_CTL1_DIVA_1 + CS_CTL1_SELA_0;
    CS->CTL2 = CS_CTL2_LFXT_EN;
    CS->KEY = 0x0000;
    P3->SEL0 = BIT1|BIT2|BIT3;
        P3->DIR |= BIT0;
        P3->OUT |= BIT0;
        EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_SWRST;     //set ucswrst
                    EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_CKPL + EUSCI_A_CTLW0_MSB + EUSCI_A_CTLW0_MST + EUSCI_A_CTLW0_SYNC;// set clock phase,msb or lsb, master mode, synchronous
                    EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_SSEL__ACLK;
                    EUSCI_A2->BRW = 0x01;

                    EUSCI_A2->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;   //Clear
        while(1){
          get_devid();
          while(!(EUSCI_A2->IFG & EUSCI_A_IFG_RXIFG));
                                   data = EUSCI_A2->RXBUF;
                                   P3->OUT = BIT0;
          setstatus(PROT_REG);

          while(!(EUSCI_A2->IFG & EUSCI_A_IFG_RXIFG));
                         data = EUSCI_A2->RXBUF;
                         P3->OUT = BIT0;
                          bfr[]=fill_buffer();

                         //block_erase(0x0000);
                         load_data(0x0000);
                         prog_exe(0x0000);
                         pagedataread(0x0000);
                         read(0x0000);
                         while(!(EUSCI_A2->IFG & EUSCI_A_IFG_RXIFG));
                                           data = EUSCI_A2->RXBUF;
                                            P3->OUT = BIT0;
        }

}


void send_data(char *buf, int length)
{
            P3->OUT &= ~BIT0;                           //cs low
            for(i=0;i<length;i++){
            while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG));

                          EUSCI_A2->TXBUF = buf[i];
            }
            //P3->OUT = BIT0;                           //cs high

}

void write_enable(){
    char buf[]={WRITE_EN};
    send_data(buf, sizeof(buf));
}


void setstatus(char reg){
    char buf[3]= {READ_STATUS_REG, reg,0x00};
    send_data(buf, sizeof(buf));
    };

  void get_devid(){
      char jedec[5]={DEV_ID,0x00,0x00,0x00,0x00};
      send_data(jedec, sizeof(jedec));

  }

void block_erase(uint16_t page_add)
{
    if(page_add < MAX_PAGE){
    char page_high = page_add>>8;
    char page_low = page_add;
    char buf[7] = {BLOCK_ERASE,0x00,0x00,0x00,0x00,page_high,page_low};
    write_enable();
    send_data(buf, sizeof(buf));
    }

}

void load_data(uint16_t col_add){
    if(col_add < MAX_COL){
        //if(datalen < MAX_COL-col_add){
        //char buf=1;
            char columnhigh = col_add>>8;
            char columnlow = col_add;
            char cmdbuf[3] = {PROG_DATA_LOAD,columnhigh,columnlow};
            write_enable();
            send_data(cmdbuf, sizeof(cmdbuf));
            delay_ms(250);
            for(i=0;i<2048;i++){
                while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG));

                                          EUSCI_A2->TXBUF = bfr[i];
            }

                        //return 0;
        //}
    }
}

void prog_exe(uint16_t page_add){
    if(page_add < MAX_PAGE){
        char page_high = page_add>>8;
        char page_low = page_add;
        write_enable();
        char buf[7] ={PROG_EXECUTE,0x00,page_high,page_low};
        send_data(buf,sizeof(buf));
       // return 0;

    }
}

void pagedataread(uint16_t page_add){
    if(page_add < MAX_PAGE){
            char page_high = page_add>>8;
            char page_low = page_add;
            char buf[7] = {PAGE_READ_DATA, 0x00, page_high, page_low};
            send_data(buf, sizeof(buf));

    }
}

void read(uint16_t col_add)
{
    if(col_add < MAX_COL){
           // if(datalen < MAX_COL-col_add){
                char columnhigh = col_add>>8;
                char columnlow = col_add;
                char cmdbuf[7] = {PROG_DATA_LOAD,columnhigh,columnlow,0x00,0x00,0x00,0x00};
                write_enable();
                send_data(cmdbuf,sizeof(cmdbuf));
                delay_ms(250);
            //}
    }
}

int fill_buffer()
{
   for(i=0;i<2048;i++)
   {
       buffer[i]=i;
   }
   return buffer[i];
}
void  delay_ms(uint8_t d)
{
    uint8_t i;
    for(i = 0; i < d; i++);
}









