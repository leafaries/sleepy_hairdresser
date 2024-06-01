# Problem śpiącego fryzjera

Projekt ma na celu zaimplementowanie symulacji pracy salonu fryzjerskiego z jednym fotelem i poczekalnią z `n` krzesłami. Symulacja ta ilustruje klasyczny problem synchronizacji wątków.

## Opis Problemu

Salon fryzjerski składa się z:
- **Gabinetu z jednym fotelem**: Tylko jeden klient może być strzyżony w danym momencie.
- **Poczekalni z n krzesłami**: Klienci, którzy nie mogą być natychmiast obsłużeni, czekają na wolnych krzesłach. Jeśli poczekalnia jest pełna, nowy klient rezygnuje z wizyty.

Fryzjer:
- Strzyże klienta przez stały czas.
- Po skończeniu strzyżenia, zaprasza kolejnego klienta z poczekalni lub idzie spać, jeśli poczekalnia jest pusta.
- Jest budzony przez nowego klienta, jeśli śpi.

## Zasady Działania Programu

1. **Obsługa klienta**: Klient, którego numer jest wywołany, przechodzi do gabinetu.
2. **Czekanie**: Pozostali klienci, którzy nie mogą być obsłużeni, z powodu braku wolnych miejsc w poczekalni, rezygnują.
3. **Raportowanie stanu**: Program wypisuje bieżący stan, który obejmuje:
    - Liczbę klientów, którzy zrezygnowali. (klient to pojedynczy wątek)
    - Liczbę zajętych miejsc w poczekalni.
    - Numer klienta w fotelu. (zasób wymagający synchronizacji)

## Wygląd raportu:
    Rezygnacja:2 Poczekalnia: 5/10 [Fotel: 4]

## Implementacja

Projekt zawiera dwie implementacje:
1. **Bez zmiennych warunkowych**: Implementacja wykorzystująca tylko mutexy i semafory do synchronizacji wątków. Semafor jest używany do kontroli liczby osób w poczekalni, a mutex do zarządzania dostępem do fotelu fryzjerskiego.
2. **Z użyciem zmiennych warunkowych**: W tej wersji wykorzystywane są zmienne warunkowe w połączeniu z mutexami, co pozwala na bardziej efektywną synchronizację poprzez uśpienie wątkó, gdy nie ma pracy i ich budzenie w odpowiednim momencie.

## Uruchomienie programu
```bash
cd ~/sleepy_hairdresser/
make
cd ./bin/
./hair_salon
```
#### Pozostało wybrać którą implementacje chcemy uruchomić.
##### Implementacja 1
```bash
./hair_salon1
```
##### Implementacja 2
```bash
./hair_salon2
```

