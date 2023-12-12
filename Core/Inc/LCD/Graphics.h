void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	drawHLine(x, y, w, color);
	drawVLine(x, y, h, color);
	drawHLine(x, y+h-1, w, color);
	drawVLine(x+w-1, y, h, color);
}

void drawCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
	uint16_t n = r/sqrtf(2)+1;
	uint16_t rkw = r*r;
	uint16_t ytab[n];
	for(uint16_t i=0; i<n; i++) {
		ytab[i] = sqrtf(rkw - i*i);
		// Czwarta ćwiara
		drawPixel(x+i, y+ytab[i], color);
		drawPixel(x+ytab[i], y+i, color);
		// Trzecia ćwiara
		drawPixel(x-i, y+ytab[i], color);
		drawPixel(x-ytab[i], y+i, color);
		// Pierwsza ćwiara
		drawPixel(x+i, y-ytab[i], color);
		drawPixel(x+ytab[i], y-i, color);
		// Druga ćwiara
		drawPixel(x-i, y-ytab[i], color);
		drawPixel(x-ytab[i], y-i, color);
	}
}

void fillCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color) {
	// ta metoda trwa 30.02ms koło R=110
	uint16_t wpos, wpos2;
	uint16_t rkw = r*r;
	for(uint16_t i=0; i<r; i++) {
		wpos = sqrtf(rkw - i*i);
		wpos2 = wpos<<1;
		drawHLine(x-wpos, y+i, wpos2, color);
		drawHLine(x-wpos, y-i, wpos2, color);
	}
}

void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color) {
	uint16_t n = r/sqrtf(2)+1;
	uint16_t rkw = r*r;
	uint16_t ytab[n];
	uint16_t x1 = x+r;
	uint16_t x2 = x+w-1-r;
	uint16_t y1 = y+r;
	uint16_t y2 = y+h-1-r;

	for(uint16_t i=0; i<n; i++) {
		ytab[i] = sqrtf(rkw - i*i);
		// Czwarta ćwiara
		drawPixel(x2+i, y2+ytab[i], color);
		drawPixel(x2+ytab[i], y2+i, color);
		// Trzecia ćwiara
		drawPixel(x1-i, y2+ytab[i], color);
		drawPixel(x1-ytab[i], y2+i, color);
		// Pierwsza ćwiara
		drawPixel(x2+i, y1-ytab[i], color);
		drawPixel(x2+ytab[i], y1-i, color);
		// Druga ćwiara
		drawPixel(x1-i, y1-ytab[i], color);
		drawPixel(x1-ytab[i], y1-i, color);
	}
	drawHLine(x+r, y, w-(r<<1), color);
	drawHLine(x+r, y+h-1, w-(r<<1), color);
	drawVLine(x, y+r, h-(r<<1), color);
	drawVLine(x+w-1, y+r, h-(r<<1), color);
}

void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color) {
	uint16_t wpos, wpos2;
	uint16_t rkw = r*r;
	uint16_t x1 = x+r;
	uint16_t y1 = y+r;
	uint16_t y2 = y+h-1-r;
	for(uint16_t i=0; i<r; i++) {
		wpos = sqrtf(rkw - i*i);
		wpos2 = (wpos<<1) + (w-(r<<1));
		drawHLine(x1-wpos, y2+i, wpos2, color);
		drawHLine(x1-wpos, y1-i, wpos2, color);
	}
	fillRect(x, y1+1, w, h-(r<<1), color);
}

#define fillScreen(color) { fillRect(0,0,400,240,(color)); }

void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	int16_t i;

	if(x1 > x2) {
		int16_t buf = x1;
		x1 = x2;
		x2 = buf;
		buf = y1;
		y1 = y2;
		y2 = buf;
	}
	int16_t w = x2-x1;
	int16_t h = y2-y1;

	if(w > ((y2>y1) ? y2-y1 : y1-y2)) { // bardziej pozioma
		for(i=x1; i<=x2; i++) drawPixel(i, y1+(h*(i-x1)/w), color);
	}
	else { // bardziej pionowa
		if(h>=0) for(i=y1; i<=y2; i++) drawPixel(x1+(w*(i-y1)/(y2-y1)), i, color);
		else     for(i=y2; i<=y1; i++) drawPixel(x1+(w*(i-y1)/(y2-y1)), i, color);
	}
}

#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t tymczasowazmiennaktorejsieboje = a;                                \
    a = b;                                                                     \
    b = tymczasowazmiennaktorejsieboje;                                        \
  }

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	// Treść tej funkcji nie jest moja bo nie umiem

	int16_t a, b, y, last;
	if (y0 > y1) {
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}
	if (y1 > y2) {
		_swap_int16_t(y2, y1);
		_swap_int16_t(x2, x1);
	}
	if (y0 > y1) {
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
			dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2)
		last = y1; // Include y1 scanline
	else
		last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
	    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
	    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			_swap_int16_t(a, b);
		drawHLine(a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = (int32_t)dx12 * (y - y1);
	sb = (int32_t)dx02 * (y - y0);
	for (; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
	    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
	    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			_swap_int16_t(a, b);
		drawHLine(a, y, b - a + 1, color);
	}
}


