#include "mfrc522.h"

MFRC522::MFRC522()
{
}
void MFRC522::InitDevice(uint32_t devIndex,uint32_t spiIndex,PVSI_INIT_CONFIG pInitConfig)
{
    MFRC522::devIndex = devIndex;
    MFRC522::spiIndex = spiIndex;
    VSI_InitSPI(VSI_USBSPI,MFRC522::devIndex,pInitConfig);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdRequest(uint8_t req_code,uint8_t *pTagType)
{
   uint8_t status;
   uint32_t  unLen;
   uint8_t ucComMF522Buf[MAXRLEN];

   ClearBitMask(Status2Reg,0x08);
   WriteRawRC(BitFramingReg,0x07);
   SetBitMask(TxControlReg,0x03);

   ucComMF522Buf[0] = req_code;

   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

   if ((status == MI_OK) && (unLen == 0x10))
   {
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {
       status = MI_ERR;
   }

   return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdAnticoll(uint8_t *pSnr)
{
    uint8_t status;
    uint8_t i,snr_check=0;
    uint32_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
         for (i=0; i<4; i++)
         {
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {
             status = MI_ERR;
         }
    }

    SetBitMask(CollReg,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdSelect(uint8_t *pSnr)
{
    uint8_t status;
    uint8_t i;
    uint32_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr+i);
        ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);

    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdAuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {
        ucComMF522Buf[i+2] = *(pKey+i);
    }
    for (i=0; i<6; i++)
    {
        ucComMF522Buf[i+8] = *(pSnr+i);
    }

    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdRead(uint8_t addr,uint8_t *pData)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {
            *(pData+i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdWrite(uint8_t addr,uint8_t *pData)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {
            ucComMF522Buf[i] = *(pData+i);
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ��ۿ�ͳ�ֵ
//����˵��: dd_mode[IN]��������
//               0xC0 = �ۿ�
//               0xC1 = ��ֵ
//          addr[IN]��Ǯ����ַ
//          pValue[IN]��4�ֽ���(��)ֵ����λ��ǰ
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdValue(uint8_t dd_mode,uint8_t addr,uint8_t *pValue)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
       // memcpy(ucComMF522Buf, pValue, 4);
        for (i=0; i<16; i++)
        {
            ucComMF522Buf[i] = *(pValue+i);
        }
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�����Ǯ��
//����˵��: sourceaddr[IN]��Դ��ַ
//          goaladdr[IN]��Ŀ���ַ
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdBakValue(uint8_t sourceaddr, uint8_t goaladdr)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf,4,&ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,6,ucComMF522Buf,&unLen);
        if (status != MI_ERR)
        {    status = MI_OK;    }
    }

    if (status != MI_OK)
    {    return MI_ERR;   }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }

    return status;
}


/////////////////////////////////////////////////////////////////////
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdHalt(void)
{
    uint8_t status;
    uint32_t  unLen;
    uint8_t ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//��MF522����CRC16����
/////////////////////////////////////////////////////////////////////
void MFRC522::CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData)
{
    uint8_t i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata+i));   }
        WriteRawRC(CommandReg, PCD_CALCCRC);
        i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdReset(void)
{
    VGI_SetPins(VSI_USBSPI,MFRC522::devIndex,1<<rstIndex);//Set RST
    Sleep(1);
    VGI_ResetPins(VSI_USBSPI,MFRC522::devIndex,1<<rstIndex);//Reset RST
    Sleep(10);
    VGI_SetPins(VSI_USBSPI,MFRC522::devIndex,1<<rstIndex);//Set RST
    Sleep(1);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    Sleep(1);
    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
    WriteRawRC(TxAutoReg,0x40);

    ClearBitMask(TestPinEnReg, 0x80);//off MX and DTRQ out
    WriteRawRC(TxAutoReg,0x40);
    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::ReadRawRC(uint8_t Address)
{
     uint8_t ucAddr = ((Address<<1)&0x7E)|0x80;
     uint8_t ucResult=0;
     VSI_WriteReadBytes(VSI_USBSPI,MFRC522::devIndex,MFRC522::spiIndex,&ucAddr,1,&ucResult,1);
     return ucResult;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void MFRC522::WriteRawRC(uint8_t Address, uint8_t value)
{
    uint8_t ucAddr = ((Address<<1)&0x7E);
    uint8_t sendBuff[2]={ucAddr,value};
    VSI_WriteBytes(VSI_USBSPI,MFRC522::devIndex,MFRC522::spiIndex,sendBuff,2);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void MFRC522::SetBitMask(uint8_t reg,uint8_t mask)
{
    uint8_t tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void MFRC522::ClearBitMask(uint8_t reg,uint8_t mask)
{
    uint8_t tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
/////////////////////////////////////////////////////////////////////
uint8_t MFRC522::PcdComMF522(uint8_t Command,
                 uint8_t *pInData,
                 uint8_t InLenByte,
                 uint8_t *pOutData,
                 uint32_t  *pOutLenBit)
{
    uint8_t status = MI_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }

    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);

    for (i=0; i<InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);


    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg,0x80);
    }

    i = 500;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    do
    {
        //Sleep(5);
        n = ReadRawRC(ComIrqReg);
        i--;
    }while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {
         if(!(ReadRawRC(ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {
                 status = MI_NOTAGERR;
             }
             if (Command == PCD_TRANSCEIVE)
             {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n-1)*8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n*8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i=0; i<n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
         }
         else
         {
             status = MI_ERR;
         }
   }
   SetBitMask(ControlReg,0x80);           // stop timer now
   WriteRawRC(CommandReg,PCD_IDLE);
   return status;
}

/************************* ����RC522�Ĺ�����ʽ  ���� ****************************/
uint8_t MFRC522::M500PcdConfigISOType(uint8_t type)
{
    if (type == 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg,0x08);
        WriteRawRC(ModeReg,0x3D);//3F
        WriteRawRC(RxSelReg,0x86);//84
        WriteRawRC(RFCfgReg,0x7F);   //4F
        WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH,0);
        WriteRawRC(TModeReg,0x8D);
        WriteRawRC(TPrescalerReg,0x3E);
        PcdAntennaOn();
    }else{
        return MI_ERR;
    }

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//��������
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
/////////////////////////////////////////////////////////////////////
void MFRC522::PcdAntennaOn(void)
{
    uint8_t i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//�ر�����
/////////////////////////////////////////////////////////////////////
void MFRC522::PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}
