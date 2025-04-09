#include "crc.h"

uint16_t ModbusCRCCalc(unsigned char *pbuf,unsigned char nlen)
{
  unsigned int index;
  unsigned char crch = 0xFF;  //��CRC�ֽ�
  unsigned char crcl = 0xFF;  //��CRC�ֽ�

  while (nlen--)  //����ָ�����ȵ�CRC
    {
      index = crch ^ *pbuf++;
      crch = crcl ^ TabH[ index];
      crcl = TabL[ index];
    }

  return ((crch<<8) | crcl);
}

