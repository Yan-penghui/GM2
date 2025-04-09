#include "crc.h"

uint16_t ModbusCRCCalc(unsigned char *pbuf,unsigned char nlen)
{
  unsigned int index;
  unsigned char crch = 0xFF;  //高CRC字节
  unsigned char crcl = 0xFF;  //低CRC字节

  while (nlen--)  //计算指定长度的CRC
    {
      index = crch ^ *pbuf++;
      crch = crcl ^ TabH[ index];
      crcl = TabL[ index];
    }

  return ((crch<<8) | crcl);
}

