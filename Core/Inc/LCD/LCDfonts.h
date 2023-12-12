typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint16_t first;   ///< ASCII extents (first char)
  uint16_t last;    ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

GFXfont *gfxFont = NULL;

void setFont(const GFXfont *f) {
	gfxFont = (GFXfont *)f;
}

void drawCusChar(uint16_t x, uint16_t y, unsigned char cin, uint16_t color, uint16_t bg) {
	unsigned char c = cin - gfxFont->first;
	GFXglyph *glyph = gfxFont->glyph + c;
	uint8_t *bitmap = gfxFont->bitmap;
	uint8_t w = glyph->width;
	uint8_t h = glyph->height;
	uint16_t bo = glyph->bitmapOffset;

	uint16_t i, bj=0;
	uint8_t bajt = bitmap[bo]; // odczytaj pierwszy bajt bitmapy
	uint16_t pixn = w*h; // liczba pikseli bitmapy

	setAddrWindow(x, y, x + w-1, y + h-1); // przekaż wyświetlaczowi okno bitmapy
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	for(i=0; i<pixn; i++) { // każdy piksel bitmapy
		write16((bajt & (0x80>>bj)) ? color : bg); // wyślij piksel
		bj++; // następny piksel bajtu
		if(bj > 7) { // jeśli bajt się skończy
			bj=0; // zresetuj odczytywany bit
			bo++; // przejdź do następnego bajtu
			bajt = bitmap[bo]; // odczytaj następny bajt
		}
	}
	CS_HIGH; // zakończ transmisję
}

void writeCusChar(unsigned char cin) {
	unsigned char c = cin - gfxFont->first;
	GFXglyph *glyph = gfxFont->glyph + c;
	uint8_t *bitmap = gfxFont->bitmap;
	uint8_t w = glyph->width;
	uint8_t h = glyph->height;
	uint16_t bo = glyph->bitmapOffset;
	int8_t xo = glyph->xOffset;
	int8_t yo = glyph->yOffset;
	uint8_t xa = glyph->xAdvance;
	uint8_t ya = gfxFont->yAdvance;

	uint16_t i, bj=0;
	uint8_t bajt = bitmap[bo]; // odczytaj pierwszy bajt bitmapy
	uint16_t pixn = w*h; // liczba pikseli bitmapy

	if(cin == '\n') {
		cur_x = txt_margin_x;
		cur_y += ya;
		if(cur_y > 239) cur_y = ya;
		return;
	}

	if(cur_x+xa > 399) {
		cur_x = txt_margin_x;
		cur_y += ya;
		if(cur_y > 239) cur_y = ya;
	}

	setAddrWindow(cur_x+xo, cur_y+yo, cur_x+xo + w-1, cur_y+yo + h -1); // przekaż wyświetlaczowi okno bitmapy
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	for(i=0; i<pixn; i++) { // każdy piksel bitmapy
		write16((bajt & (0x80>>bj)) ? txt_color : txt_bg); // wyślij piksel
		bj++; // następny piksel bajtu
		if(bj > 7) { // jeśli bajt się skończy
			bj=0; // zresetuj odczytywany bit
			bo++; // przejdź do następnego bajtu
			bajt = bitmap[bo]; // odczytaj następny bajt
		}
	}
	CS_HIGH; // zakończ transmisję


	cur_x += xa;
	if(cur_x > 399) {
		cur_x = txt_margin_x;
		cur_y += ya;
		if(cur_y > 239) cur_y = ya;
	}
}

void naiveCusPrint(const char textin[]) {
	uint16_t textlen = strlen(textin);
	for(uint16_t i=0; i<textlen; i++) {
		writeCusChar(textin[i]);
	}
}

void drawPropCusChar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, unsigned char cin) {
	unsigned char c = cin - gfxFont->first;
	GFXglyph *glyph = gfxFont->glyph + c; // aktualny znak
	uint8_t *bitmap = gfxFont->bitmap;
	uint8_t bw = glyph->width; // szerokość bitmapy
	uint8_t bh = glyph->height; // wysokość bitmapy
	uint16_t bo = glyph->bitmapOffset;
	//int8_t xo = glyph->xOffset;
	int8_t yo = glyph->yOffset;
	yo = h+yo-1;

	uint16_t i, j, bj=0;
	uint8_t bajt = bitmap[bo]; // odczytaj pierwszy bajt bitmapy
	setAddrWindow(x, y, x+w-1, y+h-1); // przekaż wyświetlaczowi okno bitmapy
	CS_LOW;
	WriteCmd(_MW);
	CD_DATA;
	for(i=0; i<h; i++) { // każdy piksel bitmapy
		for(j=0; j<w; j++) {
			if(j<bw && i<bh+yo && i>=yo) {
				write16((bajt & (0x80>>bj)) ? txt_color : txt_bg); // wyślij piksel
				bj++; // następny piksel bajtu
				if(bj > 7) { // jeśli bajt się skończy
					bj=0; // zresetuj odczytywany bit
					bo++; // przejdź do następnego bajtu
					bajt = bitmap[bo]; // odczytaj następny bajt
				}
			}
			else write16(txt_bg);
		}
	}
	CS_HIGH; // zakończ transmisję
}

void naivePrintFloat(float numin, float p_numin, uint8_t cyfry, uint8_t przed, uint8_t fresh) { // wartość do wyświetlenia, ile cyfr w sumie, liczba cyfr przed przecinkiem
	if(numin == p_numin) return; // jeśli liczba się nie zmieniła, nic nie rób
	przed++; // zwiększ liczbę cyfr o 1 (bo przecinek też się liczy chyba)
	char komenda[7];
	sprintf(komenda, "%%%d.%df", cyfry+1, cyfry-przed); // String stanowiący format pisania liczb

	char str[cyfry*2+przed]; // string nowej liczby
	char p_str[cyfry*2+przed]; // string starej liczby
	sprintf(str, komenda, numin); // wypisz nową liczbę
	sprintf(p_str, komenda, p_numin); // wypisz starą liczbę

	uint8_t skip, clear, i=0;
	while(str[i] == ' ' && p_str[i] == ' ') { // znajdź puste znaki do pominięcia
		skip = i;
		i++;
	}
	clear = 0;
	while(str[i] == ' ') { // znaki do wymazania
		clear++;
		i++;
	}

	GFXglyph *glyph = gfxFont->glyph + '3'; // bo najszersza cyfra
	uint8_t digitw = glyph->width + 1; // szerokość najszerszej cyfry
	uint8_t digith = glyph->height + 1; // wysokość najwyższej cyfry

	cur_y -= digith; // cofnij kursor o wysokość znaku
	cur_x += skip * (digitw + 1); // przesuń kursor o pominięte znaki
	fillRect(cur_x, cur_y, clear * (digitw + 1), digith, txt_bg); // wymaż odpowiednie cyfry prostokątem
	cur_x += clear * (digitw + 1); // przesuń kursor o wymazane cyfry

	for(; i<cyfry+1; i++) { // wszystkie znaki
		if(str[i] != p_str[i] || fresh) drawPropCusChar(cur_x, cur_y, digitw, digith, str[i]); // jeśli ten znak się zmienił, narysuj go
		cur_x += digitw + 1; // przesuń kursor o jeden znak dalej
	}

	cur_y += digith; // wróć kursorem na dół znaku
}

void getGlyphSize(char litera, uint16_t *wit, uint16_t *hei, int16_t *down) {
	// Sprawdza wymiary danego char'u
	litera -= gfxFont->first;
	GFXglyph *glyph = gfxFont->glyph + litera;
	*wit = glyph->width;
	*hei = glyph->height;
	*down = glyph->height -1 + glyph->yOffset;
}

void getTextSize(const char textin[], uint16_t *wit, uint16_t *hei, int16_t *down) {
	// Znajdź wymiary [px] jakie zajmie tekst. Uwaga, działa tylko na jedną linijkę!
	*wit=0; *hei=0;
	uint16_t textlen = strlen(textin);
	char c;

	uint8_t /**bitmap, w, */h, xa;
	int16_t d;
	//uint16_t bo;

	for(uint16_t i=0; i<textlen; i++) {
		c = textin[i] - gfxFont->first;
		GFXglyph *glyph = gfxFont->glyph + c;
		//*bitmap = gfxFont->bitmap;
		//w = glyph->width;
		//h = glyph->height;
		h = glyph->height;
		d = h - 1 + glyph->yOffset;
		//bo = glyph->bitmapOffset;
		xa = glyph->xAdvance;

		//d = h+d;

		if(h > *hei) *hei = h;
		if(d > *down) *down = d;
		*wit += xa;
	}
}

void naiveFloat(char komenda[], float in) {
	char message[strlen(komenda) + 20];
	sprintf(message, komenda, in);
	naiveCusPrint(message);
}

void constWfloat(char komenda[], float in) {
	char textin[strlen(komenda) + 20];
	sprintf(textin, komenda, in);

	uint16_t textlen = strlen(textin);
	uint16_t i;
	int16_t maxw = 0;
	char c;
	for(i=0; i<textlen; i++) {
		c = textin[i] - gfxFont->first;
		GFXglyph *glyph = gfxFont->glyph + c;
		if(glyph->xAdvance > maxw) maxw = glyph->xAdvance;
	}
	for(uint16_t i=0; i<textlen; i++) {
		unsigned char c = textin[i] - gfxFont->first;
		GFXglyph *glyph = gfxFont->glyph + c;
		//uint8_t *bitmap = gfxFont->bitmap;
		//uint8_t w = glyph->width;
		//uint8_t h = glyph->height;
		//uint16_t bo = glyph->bitmapOffset;
		int8_t xo = glyph->xOffset;
		int8_t yo = glyph->yOffset;
		//uint8_t xa = glyph->xAdvance;
		uint8_t ya = gfxFont->yAdvance;
		//uint8_t bajt = bitmap[bo]; // odczytaj pierwszy bajt bitmapy
		//uint16_t pixn = w*h; // liczba pikseli bitmapy
		//uint16_t bj=0;


		drawCusChar(cur_x+xo, cur_y+yo, textin[i], txt_color, txt_bg);

		cur_x += maxw;
		if(cur_x > 399) {
			cur_x = txt_margin_x;
			cur_y += ya;
			if(cur_y > 239) cur_y = ya;
		}
	}
}


void nowyGetTextSize(const char textin[], uint16_t *wit, uint16_t *hei, int16_t *down) {
	// Znajdź wymiary [px] jakie zajmie tekst. Uwaga, działa tylko na jedną linijkę!
	*wit=0; *hei=0;
	uint16_t textlen = strlen(textin);
	char c;

	//uint8_t /**bitmap, w, */h;
	int16_t ya, yb, ybuf;
	//uint16_t bo;


	c = textin[0] - gfxFont->first;
	GFXglyph *glyph = gfxFont->glyph + c;
	ya = glyph->yOffset;
	yb = ya + glyph->height;

	for(uint16_t i=0; i<textlen; i++) {
		c = textin[i] - gfxFont->first;
		GFXglyph *glyph = gfxFont->glyph + c;
		//*bitmap = gfxFont->bitmap;
		//w = glyph->width;
		//h = glyph->height;
		//d = h - 1 + glyph->yOffset;
		//bo = glyph->bitmapOffset;
		//xa = glyph->xAdvance;

		//d = h+d;
		//if(h > *hei) *hei = h;
		//if(d > *down) *down = d;
		*wit += glyph->xAdvance;

		if(glyph->yOffset < ya) ya = glyph->yOffset; // najwyższa współrzędna
		ybuf = glyph->yOffset + glyph->height;
		if(ybuf > yb) yb = ybuf; // najniższa współrzędna
	}

	*down = yb; *hei = yb-ya;
}

void zamazText(const char textin[]) { // zamazuje custom font tłem (pozycja, cionka i kolor wzięty z fontu)
	uint16_t wit, hei;
	int16_t down;
	nowyGetTextSize(textin, &wit, &hei, &down);
	fillRect(cur_x, cur_y-hei+down, wit+1, hei, txt_bg);
}
