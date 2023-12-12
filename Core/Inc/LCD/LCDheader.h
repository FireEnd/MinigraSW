#define LCD_DATA_PORT GPIOB // piny 0-7
#define LCD_CTRL_PORT GPIOA
#define LCD_CS_PIN GPIO_PIN_8
#define LCD_RS_PIN GPIO_PIN_9 //czy to CD? Tak. I X- też
#define LCD_WR_PIN GPIO_PIN_10
#define LCD_RD_PIN GPIO_PIN_11
#define LCD_MISC_PORT GPIOC // port pinu RST i LED wyświetlacza
#define LCD_RST_PIN GPIO_PIN_15
#define LCD_LED_PIN GPIO_PIN_14

#define write_8(data) {	LCD_DATA_PORT->ODR = ((LCD_DATA_PORT->ODR & 0xFF00) | (data)); }
#define write8(data)  { write_8(data); WR_STROBE; }
#define write16(data) { write8((data)>>8); write8((data)&0xFF); }

#define CS_HIGH { LCD_CTRL_PORT->ODR |=  (LCD_CS_PIN); }
#define CS_LOW  { LCD_CTRL_PORT->ODR &= ~(LCD_CS_PIN); }
#define CD_DATA    { LCD_CTRL_PORT->ODR |=  (LCD_RS_PIN); }
#define CD_COMMAND { LCD_CTRL_PORT->ODR &= ~(LCD_RS_PIN); }
#define WR_ACTIVE { LCD_CTRL_PORT->ODR &= ~(LCD_WR_PIN); }
#define WR_IDLE   { LCD_CTRL_PORT->ODR |=  (LCD_WR_PIN); }
#define RD_IDLE   { LCD_CTRL_PORT->ODR |=  (LCD_RD_PIN); }
#define RD_ACTIVE { LCD_CTRL_PORT->ODR &= ~(LCD_RD_PIN); }

//#define CS_LOWiCD_COMMAND { LCD_CTRL_PORT->ODR &= 0b1111111111111100; }
//#define CS_LOWiCD_COMMAND { LCD_CTRL_PORT->ODR &= 0b1111111111001111; }
#define CS_LOWiCD_COMMAND { LCD_CTRL_PORT->ODR &= ~(LCD_CS_PIN | LCD_RS_PIN); }

#define WR_STROBE { WR_ACTIVE; WR_IDLE;}
#define WriteCmd(x) { CD_COMMAND; write16(x); }
#define WriteData(x) { CD_DATA; write16(x); }
#define backlight(st) { HAL_GPIO_WritePin(LCD_MISC_PORT, LCD_LED_PIN, (st)?GPIO_PIN_SET:GPIO_PIN_RESET); } // włącza i wyłącza podświetlenie

uint16_t _SC = 0x210;
uint16_t _EC = 0x211;
uint16_t _SP = 0x212;
uint16_t _EP = 0x213;
#define MIPI_DCS_REV1   (1<<0)
#define REV_SCREEN      (1<<12)
#define INVERT_GS       (1<<8)
#define INVERT_SS       (1<<9)
#define MV_AXIS         (1<<10)
#define INVERT_RGB      (1<<11)
uint16_t _lcd_capable;
uint8_t rotation = 0;
uint16_t _lcd_rev, _lcd_madctl;
#ifdef ILI9327
#define OFFSET_9327 32
#endif

uint16_t _MW = 0x202;
uint16_t _MP = 0x201;
uint16_t _MC = 0x200;
//uint16_t _SC = 0x210, _EC = 0x211, _SP = 0x212, _EP = 0x213;

void WriteCmdParamN(uint16_t cmd, uint8_t n, uint8_t *data) {
	CS_LOW;
	WriteCmd(cmd);
	for(uint8_t i=0; i<4; i++) {
		CD_DATA;
		write8(data[i]);
	}
	CS_HIGH;
}

static inline void WriteCmdParam4(uint8_t cmd, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
	uint8_t data[4] = {d1, d2, d3, d4};
	WriteCmdParamN(cmd, 4, data);
}

void WriteCmdData(uint16_t cmd, uint16_t data) {
	//CS_LOW;
	//WriteCmd(cmd);
	CS_LOWiCD_COMMAND;
	write16(cmd);
	WriteData(data);
	CS_HIGH;
}

void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	/*WriteCmdParam4(_MC, x>>8, x, x>>8, x);
	WriteCmdParam4(_MP, y>>8, y, y>>8, y);*/
	WriteCmdData(_MC, x);
	WriteCmdData(_MP, y);
	WriteCmdData(_MW, color);
}

void setAddrWindow(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1) { // nie działa chyba
#if defined(ILI9327)
	/*if (rotation == 2) y += OFFSET_9327, y1 += OFFSET_9327;
	if (rotation == 3) x += OFFSET_9327, x1 += OFFSET_9327;*/

	//uint8_t data[4] = {x >> 8, x, x1 >> 8, x1};
	//WriteCmdParamN(_SC, 4, data);
	// Przyspieszone WriteCmdParamN(_SC...)
	CS_LOW;
	WriteCmd(_SC);
	CD_DATA;
	write8(x >> 8);
	write8(x);
	write8(x1 >> 8);
	write8(x1);
	CS_HIGH;

	//uint8_t data2[4] = {x >> 8, x, x1 >> 8, x1};
	//WriteCmdParamN(_SP, 4, data2);
	// Przyspieszone WriteCmdParamN(_SC...)
	CS_LOW;
	WriteCmd(_SP);
	CD_DATA;
	write8(y >> 8);
	write8(y);
	write8(y1 >> 8);
	write8(y1);
	CS_HIGH;

#else
	WriteCmdData(_MC, x);
    WriteCmdData(_MP, y);

    WriteCmdData(_SC, x);
    WriteCmdData(_SP, y);
    WriteCmdData(_EC, x1);
    WriteCmdData(_EP, y1);
#endif
}

/*void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) { // to ten zwykły
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	uint8_t hi = color>>8;
	uint8_t lo = color&0xFF;
	uint16_t end = w;
	CD_DATA;
	while(h-- > 0) {
		end = w;
		while(end-- > 0) {
			write8(hi);
			write8(lo);
		}
	}
	CS_HIGH;
	//setAddrWindow(0, 0, 240 - 1, 400 - 1);
}*/

void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) { // to ten szybszy
	uint8_t hi = color>>8;
	uint8_t lo = color&0xFF;
	if(hi==lo) { // tryb zapiepszania
		setAddrWindow(x, y, x + w - 1, y + h - 1);
		CS_LOW;
		WriteCmd(_MW);
		//uint8_t hi = color>>8;
		//uint8_t lo = color&0xFF;
		uint16_t end = w;
		CD_DATA;
		write_8(lo);
		while(h-- > 0) {
			end = w;
			while(end-- > 0) {
				/*write8(hi);
				write8(lo);*/
				WR_STROBE;
				WR_STROBE;

			}
		}
		CS_HIGH;
	}
	else { // normalny tryb
		setAddrWindow(x, y, x + w - 1, y + h - 1);
		CS_LOW;
		WriteCmd(_MW);
		//uint8_t hi = color>>8;
		//uint8_t lo = color&0xFF;
		uint16_t end = w;
		CD_DATA;
		while(h-- > 0) {
			end = w;
			while(end-- > 0) {
				write8(hi);
				write8(lo);
			}
		}
		CS_HIGH;
	}
}

void drawHLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
	setAddrWindow(x, y, x+len-1, y);
	CS_LOWiCD_COMMAND;
	write16(_MW);
	uint8_t hi = color>>8;
	uint8_t lo = color&0xFF;
	CD_DATA;
	while(len-- > 0) {
		write8(hi);
		write8(lo);
	}
	CS_HIGH;
}

void drawVLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
	setAddrWindow(x, y, x, y+len-1);
	CS_LOWiCD_COMMAND;
	write16(_MW);
	uint8_t hi = color>>8;
	uint8_t lo = color&0xFF;
	CD_DATA;
	while(len-- > 0) {
		write8(hi);
		write8(lo);
	}
	CS_HIGH;
}

uint16_t col8to16(uint8_t in) {
	//return ((in & 0b11000000)<<8) | ((in & 0b0011100)<<6) | ((in & 0b0000011)<<3);
	return ((in & 0b11100000)<<8) | ((in & 0b00011100) <<6) | ((in & 0b00000011)<<3);
}

void drawBitmapSlowLo(uint8_t *mapin, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	uint32_t pxn = 0;
	uint16_t endx = x+w;
	uint16_t endy = y+h;
	for(uint16_t i=y; i<endy; i++) {
		for(uint16_t j=x; j<endx; j++) {
			drawPixel(j, i, col8to16(mapin[pxn]));
			pxn++;
		}
	}
}

void drawBitmapLo(const uint8_t *mapin, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	uint32_t pxn = 0;
	uint16_t endy = y+h;
	uint16_t endx = x+w;
	for(int16_t j=y; j<endy; j++) {
		for(uint16_t i=x; i<endx; i++) {
			write16(col8to16(mapin[pxn]));
			pxn++;
		}
	}
	CS_HIGH;
}

void drawBitmapSlowHi(uint16_t *mapin, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	uint32_t pxn = 0;
	uint16_t endy = x+w;
	uint16_t endx = y+h;
	for(int16_t j=y; j<endy; j++) {
		for(uint16_t i=x; i<endx; i++) {
			drawPixel(i, j, mapin[pxn]);
			pxn++;
		}
	}
}

void drawBitmapHi(const uint16_t *mapin, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	uint32_t pxn = 0;
	uint16_t endy = y+h;
	uint16_t endx = x+w;
	for(int16_t j=y; j<endy; j++) {
		for(uint16_t i=x; i<endx; i++) {
			write16(mapin[pxn]);
			pxn++;
		}
	}
	CS_HIGH;
}

void drawMonoBitmap(uint16_t x, uint16_t y, const uint8_t *bitmap, uint16_t color, uint16_t bgc) {
	// Monochromatyczna mapa bitowa o zdefiniowanej kolorystyce. Mapa to jednowymiarowa tablica bajtów niosąca informacje o kolejnych bitach
	// Pierwsze dwa bajty to szerokość i wysokość

	uint16_t w = bitmap[0];
	uint16_t h = bitmap[1];
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	uint16_t i, j;
	uint16_t num = ceil(w*h/8)+2;
	uint8_t coltab[4];
	coltab[0] = color>>8; coltab[1] = color & 0xFF;
	coltab[2] = bgc>>8;   coltab[3] = bgc&0xFF;
	//uint8_t bitnum = 0;
	for(j=2; j<num; j++) {
		for(i=0; i<8; i++) {
			if(bitmap[j] & (1<<i)) { // jeśli 1 - kolor
				write8(coltab[0]);
				write8(coltab[1]);
			}
			else { // jeśli 0 - tło
				write8(coltab[2]);
				write8(coltab[3]);
			}
		}
	}
	CS_HIGH;
}
/*
void drawMonoBitmap(uint16_t x, uint16_t y, uint8_t *bitmap, uint16_t color, uint16_t bgc) {
	// Monochromatyczna mapa bitowa o zdefiniowanej kolorystyce. Mapa to jednowymiarowa tablica bajtów niosąca informacje o kolejnych bitach
	// Pierwsze dwa bajty to szerokość i wysokość

	uint16_t w = bitmap[0];
	uint16_t h = bitmap[1];
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	uint16_t i, j;
	uint16_t num = ceil(w*h/8)+2;
	uint8_t coltab[4];
	coltab[0] = color>>8; coltab[1] = color & 0xFF;
	coltab[2] = bgc>>8;   coltab[3] = bgc&0xFF;
	uint16_t bajt=2;
	uint8_t bit = 0;
	//uint8_t bitnum = 0;
	for(j=0; j<h; j++) {
		for(i=0; i<w; i++) {
			if(bitmap[bajt] & (1<<bit)) { // jeśli 1 - kolor
				write8(coltab[0]);
				write8(coltab[1]);
			}
			else { // jeśli 0 - tło
				write8(coltab[2]);
				write8(coltab[3]);
			}
			bit++;
			if(bit > 7) {
				bit=0;
				bajt++;
			}
		}
		bajt++;
	}
	CS_HIGH;
}*/

uint16_t RGB2col(uint8_t R, uint8_t G, uint8_t B) {
	return ((R&0b11111000)<<8) | ((G&0b11111100)<<3) | ((B&0b11111000)>>3);
}

void reset() {
	/*LCD_CTRL_PORT->MODER &= ~(1<<(LCD_RS_PIN*2+1));
	LCD_CTRL_PORT->MODER |=  (1<<(LCD_RS_PIN*2));
	LCD_CTRL_PORT->MODER &= ~(1<<(LCD_WR_PIN*2+1));
	LCD_CTRL_PORT->MODER |=  (1<<(LCD_WR_PIN*2));*/

	LCD_CTRL_PORT->MODER &= ~((LCD_RS_PIN*LCD_RS_PIN)<<1);
	LCD_CTRL_PORT->MODER |=  (LCD_RS_PIN*LCD_RS_PIN);
	LCD_CTRL_PORT->MODER &= ~((LCD_WR_PIN*LCD_WR_PIN)<<1);
	LCD_CTRL_PORT->MODER |=  (LCD_WR_PIN*LCD_WR_PIN);
	CS_HIGH;
	RD_IDLE;
	WR_IDLE;
	HAL_GPIO_WritePin(LCD_MISC_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_MISC_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

#define TFTLCD_DELAY 0xFFFF
#define TFTLCD_DELAY8 0xFF

#ifndef ILI9327
static void init_table16(const uint16_t *table, int16_t size) {
    uint16_t p=0;
    uint16_t cmd, d;
    while (size > 0) {
        cmd = table[p];
        p++;
        d = table[p];
        p++;
        if (cmd == TFTLCD_DELAY)
            HAL_Delay(d);
        else {
            CS_LOW;
            WriteCmd(cmd);
            WriteData(d);
            CS_HIGH;
        }
        size -= 2 * sizeof(int16_t);
    }
}
#else
//#define TFTLCD_DELAY 0xFFFF
//#define TFTLCD_DELAY8 0x7F
static void init_table(const uint8_t *table, int16_t size) {
    //copes with any uint8_t table.  Even HX8347 style
    uint8_t p=0;
    while (size > 0) {
        uint8_t cmd = table[p++];
        uint8_t len = table[p++];
        if (cmd == TFTLCD_DELAY8) {
            HAL_Delay(len);
            len = 0;
        } else {
            CS_LOW;
            CD_COMMAND;
            write8(cmd);
            for (uint8_t d = 0; d++ < len; ) {
                uint8_t x = table[p++];
                CD_DATA;
                write8(x);
                /*if (is8347 && d < len) {
                    CD_COMMAND;
                    cmd++;
                    write8(cmd);
                }*/
            }
            CS_HIGH;
        }
        size -= len + 2;
    }
}
#endif

int16_t HEIGHT = 400;
int16_t WIDTH = 240;
#ifndef ILI9327
void initDisplay() {
	int16_t *p16;
	reset();
	//_lcd_xor=0;
	_lcd_capable = REV_SCREEN;
	static const uint16_t R61509V_regValues[] = {
	            0x0000, 0x0000,
	            0x0000, 0x0000,
	            0x0000, 0x0000,
	            0x0000, 0x0000,
	            TFTLCD_DELAY, 15,
	            0x0400, 0x6200,     //NL=0x31 (49) i.e. 400 rows
	            0x0008, 0x0808,
	            //gamma
	            0x0300, 0x0C00,
	            0x0301, 0x5A0B,
	            0x0302, 0x0906,
	            0x0303, 0x1017,
	            0x0304, 0x2300,
	            0x0305, 0x1700,
	            0x0306, 0x6309,
	            0x0307, 0x0C09,
	            0x0308, 0x100C,
	            0x0309, 0x2232,

	            0x0010, 0x0016,     //69.5Hz         0016
	            0x0011, 0x0101,
	            0x0012, 0x0000,
	            0x0013, 0x0001,

	            0x0100, 0x0330,     //BT,AP
	            0x0101, 0x0237,     //DC0,DC1,VC
	            0x0103, 0x0D00,     //VDV
	            0x0280, 0x6100,     //VCM
	            0x0102, 0xC1B0,     //VRH,VCMR,PSON,PON
	            TFTLCD_DELAY, 50,

	            0x0001, 0x0100,
	            0x0002, 0x0100,
	            0x0003, 0x1030,     //1030
	            0x0009, 0x0001,
	            0x000C, 0x0000,
	            0x0090, 0x8000,
	            0x000F, 0x0000,

	            0x0210, 0x0000,
	            0x0211, 0x00EF,
	            0x0212, 0x0000,
	            0x0213, 0x018F,     //432=01AF,400=018F
	            0x0500, 0x0000,
	            0x0501, 0x0000,
	            0x0502, 0x005F,     //???
	            0x0401, 0x0001,     //REV=1
	            0x0404, 0x0000,
	            TFTLCD_DELAY, 50,

	            0x0007, 0x0100,     //BASEE
	            TFTLCD_DELAY, 50,

	            0x0200, 0x0000,
	            0x0201, 0x0000,
	        };
	        init_table16(R61509V_regValues, sizeof(R61509V_regValues));
	        p16 = (int16_t *) & HEIGHT; // tutaj było to HEIGHT
	        *p16 = 400;

	        _lcd_rev = ((_lcd_capable & REV_SCREEN) != 0);
}
#else
void initDisplay() {
	int16_t *p16;
	reset();

	const uint8_t *table8_ads = NULL;
	int16_t table_size;
	//_lcd_xor=0;
	_lcd_capable = (1<<1) | (1<<0) | (1<<10);
	static const uint8_t ILI9327_regValues[] = {
			0xB0, 1, 0x00,      //Disable Protect for cmds B1-DF, E0-EF, F0-FF
			//            0xE0, 1, 0x20,      //NV Memory Write [00]
			//            0xD1, 3, 0x00, 0x71, 0x19,  //VCOM control [00 40 0F]
			//            0xD0, 3, 0x07, 0x01, 0x08,  //Power Setting [07 04 8C]
			0xC1, 4, 0x10, 0x10, 0x02, 0x02,    //Display Timing [10 10 02 02]
			0xC0, 6, 0x00, 0x35, 0x00, 0x00, 0x01, 0x02,        //Panel Drive [00 35 00 00 01 02 REV=0,GS=0,SS=0
			0xC5, 1, 0x04,      //Frame Rate [04]
			0xD2, 2, 0x01, 0x04,        //Power Setting [01 44]
			//            0xC8, 15, 0x04, 0x67, 0x35, 0x04, 0x08, 0x06, 0x24, 0x01, 0x37, 0x40, 0x03, 0x10, 0x08, 0x80, 0x00,
			//            0xC8, 15, 0x00, 0x77, 0x77, 0x04, 0x04, 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
			0xCA, 1, 0x00,      //DGC LUT ???
			0xEA, 1, 0x80,      //3-Gamma Function Enable
			//                     0xB0, 1, 0x03,      //Enable Protect
	};
	table8_ads = ILI9327_regValues, table_size = sizeof(ILI9327_regValues);
	p16 = (int16_t *) & HEIGHT;
	*p16 = 400;
	p16 = (int16_t *) & WIDTH;
	*p16 = 240;

    if (table8_ads != NULL) {
        static const uint8_t reset_off[] = {
            0x01, 0,            //Soft Reset
            TFTLCD_DELAY8, 150,  // .kbv will power up with ONLY reset, sleep out, display on
            0x28, 0,            //Display Off
            0x3A, 1, 0x55,      //Pixel read=565, write=565.
        };
        static const uint8_t wake_on[] = {
			0x11, 0,            //Sleep Out
            TFTLCD_DELAY8, 150,
            0x29, 0,            //Display On
        };
		init_table(reset_off, sizeof(reset_off));
	    init_table(table8_ads, table_size);   //can change PIXFMT
		init_table(wake_on, sizeof(wake_on));
    }
}
#endif

void setRotation(uint8_t r) {
	uint16_t GS, SS, ORG; //, REV = _lcd_rev;
	uint8_t val; // , d[3];
	rotation = r & 3;           // just perform the operation ourselves on the protected variables
	//_width = (rotation & 1) ? HEIGHT : WIDTH;
	//_height = (rotation & 1) ? WIDTH : HEIGHT;
	switch (rotation) {
	case 0:                    //PORTRAIT:
		val = 0x48;             //MY=0, MX=1, MV=0, ML=0, BGR=1
		break;
	case 1:                    //LANDSCAPE: 90 degrees
		val = 0x28;             //MY=0, MX=0, MV=1, ML=0, BGR=1
		break;
	case 2:                    //PORTRAIT_REV: 180 degrees
		val = 0x88;             //MY=1, MX=0, MV=0, ML=1, BGR=1
		break;
	case 3:                    //LANDSCAPE_REV: 270 degrees
		val = 0xF8;             //MY=1, MX=1, MV=1, ML=1, BGR=1
		break;
	}
	if (_lcd_capable & INVERT_GS)
		val ^= 0x80;
	if (_lcd_capable & INVERT_SS)
		val ^= 0x40;
	if (_lcd_capable & INVERT_RGB)
		val ^= 0x08;
	if (_lcd_capable & MIPI_DCS_REV1) {
		//common_MC:
		_MC = 0x2A, _MP = 0x2B, _MW = 0x2C, _SC = 0x2A, _EC = 0x2A, _SP = 0x2B, _EP = 0x2B;
		//common_BGR:
		WriteCmdParamN(0x36, 1, &val);
		//_lcd_madctl = val;
	}
	// cope with 9320 variants
	else {
		_MC = 0x200, _MP = 0x201, _MW = 0x202, _SC = 0x210, _EC = 0x211, _SP = 0x212, _EP = 0x213;
		GS = (val & 0x80) ? (1 << 15) : 0;
		uint16_t NL;
		NL = ((HEIGHT / 8) - 1) << 9;
		//  WriteCmdData(0x400, GS | NL);
		uint16_t scan;
		if (GS == 0) scan = 0;
		else scan = 0x04;
		WriteCmdData(0x400, GS | NL | scan);

		SS = (val & 0x40) ? (1 << 8) : 0;
		//SS = (val&0x05)<<8;
		WriteCmdData(0x01, SS);     // set Driver Output Control
		//common_ORG:
		ORG = (val & 0x20) ? (1 << 3) : 0;
		if (val & 0x08)
			ORG |= 0x1000;  //BGR
		_lcd_madctl = ORG | 0x0030;
		WriteCmdData(0x03, _lcd_madctl);    // set GRAM write direction and BGR=1.
	}
	if ((rotation & 1) && ((_lcd_capable & MV_AXIS) == 0)) {
		uint16_t x;
		x = _MC, _MC = _MP, _MP = x;
		x = _SC, _SC = _SP, _SP = x;    //.kbv check 0139
		x = _EC, _EC = _EP, _EP = x;    //.kbv check 0139
	}

}

#define BLACK   0
#define WHITE   0xFFFF
#define RED     0b1111100000000000
#define GREEN   0b0000011111100000
#define BLUE    0b0000000000011111
#define MAGENTA 0b1111100000011111
#define YELLOW  0b1111111111100000
#define ORANGE  0b1111101111100000
#define CYAN    0b0000011111111111
#define GREY    0b0111101111101111
//#define D_GREY  0b0011100111100111
#define D_GREY  0x4A4A // szybki ciemny szary
#define D_RED   0b1001100000000000
#define D_GREEN 0b0000010010100000
#define D_BLUE  0b0000000000010000
#define L_RED   0b1111101111101111
#define L_GREEN 0b0111111111101111
//#define L_BLUE  0b0111101111111111
#define L_BLUE  0x5C5C // szybki

// A TERAZ LITERKI, CYFERKI I INNE PISADŁA

#include "defaultFont.h"
uint16_t cur_x = 0; // pozycja X kursora
uint16_t cur_y = 0; // pozycja Y kursora
uint8_t font_size = 0; // rozmiar cionki
uint16_t litera_w = 5; // szerokość litery
uint16_t litera_h = 8; // wysokość litry
uint16_t litera_o = 1; // przerwa między literami
uint16_t txt_color = WHITE; // kolor tekstu
uint16_t txt_bg = BLACK; // kolor tła
uint16_t txt_margin_x = 0; // lewy margines tekstu

void drawDefChar(uint16_t x, uint16_t y, unsigned char c, uint16_t color, uint16_t bg) {
	// 1750 znaków w 110ms
	uint16_t j, i;
	setAddrWindow(x, y, x + 4, y + 7);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	for(i=0; i<8; i++) {
		for(j=0; j<5; j++) {
			write16( (default_font[c*5+j] & (1<<i)) ? color : bg);
		}
	}
	CS_HIGH;
}

void writeChar(unsigned char c) {
	drawDefChar(cur_x, cur_y, c, txt_color, txt_bg);
	if(400-cur_x >= 2*(litera_w+litera_o)) cur_x += litera_w + litera_o;
	else {
		cur_x = txt_margin_x;
		if(240-cur_y >= 2*(litera_h+litera_o)) cur_y += litera_h + litera_o;
		else cur_y = 0;
	}
}

void naivePrint(char textin[]) {
	uint16_t textlen = strlen(textin);
	for(uint16_t i=0; i<textlen; i++) {
		if(textin[i] == '\n') {
			cur_x = txt_margin_x;
			if(240-cur_y >= 2*(litera_h+litera_o)) cur_y += litera_h + litera_o;
			else cur_y = 0;
		}
		else writeChar(textin[i]);
	}
}

void gradientVRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colorTab[]) {
	// Prostokąt wypełniony pionowym gradientem. Gradient przekazywany jest jako jednowymiarowa tablica kolorów o długości równej wysokości prostokąta
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	uint8_t hi;
	uint8_t lo;
	uint16_t end = w;
	CD_DATA;
	while(h-- > 0) {
		end = w;
		hi = colorTab[h]>>8;
		lo = colorTab[h]&0xFF;
		while(end-- > 0) {
			write8(hi);
			write8(lo);
		}
	}
	CS_HIGH;
}

void gradientHRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colorTab[]) {
	// Prostokąt wypełniony poziomym gradientem. Gradient przekazywany jest jako jednowymiarowa tablica kolorów o długości równej szerokości prostokąta
	setAddrWindow(x, y, x + w - 1, y + h - 1);
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	uint16_t i, j;
	for(j=0; j<h; j++) {
		for(i=0; i<w; i++) {
			write16(colorTab[i]);
		}
	}
	CS_HIGH;
}

