# Zadanie rekrutacyjne - Senior Embedded C++ Developer

**Szacowany czas: 2–4 h.** Nie oczekujemy produktu, tylko próbki tego, jak pracujesz.
Jeśli czegoś nie zdążysz - opisz to w `DECISIONS.md` zamiast robić na siłę.

---

## Kontekst

Automat vendingowy stoi w galerii handlowej. Klient przykłada kartę RFID, wybiera produkt,
automat go wydaje. Urządzenie **regularnie traci połączenie z siecią**,
a mimo to musi działać dalej i nigdy nie zgubić transakcji - bo za każdą z nich stoi realny towar,
który wyjechał z maszyny.

Twoim zadaniem jest napisanie **rdzenia tej logiki + minimalnego UI**.

---

## Zakres obowiązkowy

### 1. Rdzeń logiki (C++17/20, bez zależności od Qt)

Maszyna stanów obsługująca cykl wydania:

```
Idle → CardRead → ProductSelected → Dispensing → Completed
                                              ↘ Failed
```

- przejścia wyzwalane zdarzeniami (`onCardTapped`, `onProductSelected`, `onDispenseResult`),
- timeout na etapie wyboru produktu (np. 15s → powrót do `Idle`),
- ponowne przyłożenie karty w trakcie wydawania **nie** może rozpocząć drugiej transakcji.

Sprzęt (podajnik, czytnik RFID) mockujesz - interfejs + fake w testach.

### 2. Trwały dziennik transakcji (SQLite)

- każda transakcja ma **UUID nadany po stronie urządzenia**, znacznik czasu, id produktu, id karty, status,
- zapis do bazy **przed** fizycznym wydaniem, aktualizacja statusu po,
- po restarcie aplikacji stan bazy musi być spójny - udokumentuj, co się dzieje z transakcją
  przerwaną w połowie (zabicie procesu w trakcie `Dispensing`).

### 3. Synchronizacja z backendem

- worker wysyła niezsynchronizowane transakcje jako JSON na endpoint REST (`POST /transactions`),
- backend jest **niedostępny przez większość czasu** - potrzebny retry z backoffem,
- ponowna wysyłka tej samej transakcji nie może jej zdublować po stronie serwera,
- warstwa sieciowa za interfejsem, żeby dało się ją podmienić na fake w testach.

> Nie musisz stawiać prawdziwego serwera. Wystarczy prosty mock (np. skrypt w Pythonie,
> `nc`, kontener) albo wyłącznie implementacja `FakeTransport` w testach - Twój wybór, opisz go w README.

### 4. UI w QML

Absolutne minimum, **nie oceniamy estetyki**:

- siatka 3–4 produktów,
- przycisk „Symuluj przyłożenie karty",
- wskaźnik postępu wydawania,
- status: `online / offline` + licznik transakcji czekających na sync.

**Wymóg twardy:** UI nie może się zacinać. Wydawanie i synchronizacja nie blokują wątku GUI.

### 5. Testy

Minimum kilka testów jednostkowych na maszynę stanów i na kolejkę synchronizacji
(GTest / Catch2 / QTest - obojętne). Nie oczekujemy pokrycia 100%, oczekujemy testów,
które łapią coś istotnego.

---

## Czego **nie** robić

Żeby zmieścić się w czasie - te rzeczy świadomie pomijamy i nie będą oceniane:

- prawdziwy sprzęt, Yocto, cross-compilacja,
- autentykacja, TLS, obsługa płatności,
- ładne UI, animacje, motywy,
- ORM, warstwy abstrakcji „na przyszłość", DI container,
- obsługa wielu automatów / wielu użytkowników.

---

## `DECISIONS.md` - to czytamy najuważniej

Krótki plik (pół strony do strony), w którym opisujesz:

1. jak podzieliłeś kod i dlaczego,
2. jak rozwiązałeś idempotencję synchronizacji,
3. co dzieje się przy zaniku zasilania w trakcie wydawania,
4. **czego nie zdążyłeś i jak byś to zrobił, mając tydzień**,
5. ile realnie zajęło Ci to zadanie.

Punkt 4. nie jest karany. Świadome cięcie zakresu jest sygnałem doświadczenia.

---

## Oddanie pracy

Publiczne repozytorium na GitHub'ie + link mailem. Historia commitów mile widziana
(chcemy zobaczyć, jak pracujesz, nie jeden commit „init").