
uint8_t butUnbl(uint8_t i) { // odczyt przycisku (bez blokady)
	// Numery przycisków: (0 - Generator | 1 - Rezystancja | 2 - Blokada | 3 - Podsłuch)
	static uint32_t lockTimer[BUT_N]; // timer debouncingu przycisków
	static uint8_t bufState[BUT_N]; // bufor statusu przycisków
	uint8_t ret; // zwracana wartość
	if(lockTimer[i] && HAL_GetTick() < lockTimer[i]) return bufState[i]; // jeśli trwa debouncing zwróć zapamiętany stan
	else { // jeśli nie debouncing
		ret = (BUT_PORT->IDR & (1<<(BUT_PIN+i))) ? 0:1; // zwracana wartość to odczyt przycisku
		if(ret != bufState[i] && (lockTimer[i] == 0 || HAL_GetTick() >= lockTimer[i])) { // jeśli zbocze i nie debouncing
			bufState[i] = ret; // zapamiętaj nowy stan przycisku
			lockTimer[i] = HAL_GetTick() + DEBOUNCE_TIME; // ustaw timer debouncingu
		}
		return bufState[i]; // zwróć stan przycisku
	}
}

uint8_t but(uint8_t i) { // odczyt przycisku
	if(butLockFlag[i]) return 0; // jeśli zablokowany zwróć 0
	else return butUnbl(i); // jeśli odblokowany zwróć przycisk
}

void butLock(uint8_t i) { // ustawia blokadę przycisku, by zawsze zwracał 0
	butLockFlag[i] = 1;
}

void releaseButLock() { // sprawdza które przyciski można odblokować
	for(uint8_t i=0; i<BUT_N; i++) { // przeleć przez wszystkie przyciski
		if(butLockFlag[i] && butUnbl(i)==0) butLockFlag[i] = 0; // jeśli blokada aktywna i przycisk puszczony to odblokuj
	}
}

void led(uint8_t n, uint8_t st) { // steruje uniwersalnymi diodami diagnostycznymi. Znaczy tutaj jest jedna, więc tylko jedną
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, st);
}
