unsigned char Usb_Check_Status(void);
void Usb_Initial(void);
void Usb_List_Files(void);
void Usb_Read_Files(unsigned char index);
unsigned char Usb_Get_FileNum(void);
char* Usb_Get_FileName(unsigned char index);
char Usb_Data_decode(void);
void Usb_Write_Data_Flash(char table,unsigned char line,char pos,unsigned int dat,char length);
void Usb_Write_String_Flash(char table,unsigned char line,char pos,char* string,char length);
unsigned int Usb_Read_Flash_Dat(char table,unsigned char line,char pos,char length);
char* Usb_Read_Flash_String(char table,unsigned char line,char pos,char length);
void Usb_Flash_To_Ram(void);
void Usb_Check(void);




