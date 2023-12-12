void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
}

uint16_t odTyU(uint16_t in) { // zamienia bity uinta na od tyłu
	uint16_t ret = 0;
	for(uint8_t i=0; i<16; i++) {
		// wytnij i-ty bit, przesuń go na początek i wstaw w (15-i)ty bit wyjścia
		ret |= ((in&(1<<i)) != 0) << (15-i);
	}
	return ret;
}

float getRandomNumber() { // generuje losowego floata (0.0 - 1.0)
	static uint16_t prevBuf; // poprzednio wygenerowana wartość
	uint16_t buf = 0; // aktualnie generowana wartość
	uint8_t i = 0;

	for(i=0; i<16; i+=4) { // połowa bitów pochodzi z szumów ADC
		buf |= (ADC_INS[i>>2] && 0b11) << i; // z każdego ADC'ka po 2 najmłodsze bity
	}

	uint16_t krwawaMasakra = (~odTyU(prevBuf))*2137; // zamień i odwróć bity poprzedniej liczby i pomnóż przez gupotę
	buf |= krwawaMasakra & 0b1100110011001100; // połowa bitów pochodzi od zrujnowanej wartości poprzedniej
	prevBuf = buf; // zapamiętaj bufor na przyszłość
	return ((float)buf) / 65536.0;
}
