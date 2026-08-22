# DECISIONS

## 0. Generalne idee / wizja / fundamenty procesowe i projektowe
Punktem wyjścia było stworzenie kontenera z niezbędnymi zależnościami. To początkowo zajęło najwięcej
czasu ze zwględu na to, że obraz był przygotowany pod QT Widgets, a źródła nie zawierały
niezbędnych zależności do QML i SQLite ```qtdeclarative,qtshadertools```. Sukcesywnie dodawałem do obrazu kolejne 
zależności i sprawdzałem czy aplikacja - początkowo stub się uruchamia.

Dla mnie osobiście i w projektach, które prowadzę konteneryzacja jest jednym z priorytetów. Dzięki temu po kilku latach można łatwo
wrócić do wcześniejszych projektów, zbudować obraz i uruchomić projekt. Z takiego też założenia wyszedłem w tym przypadku.

Przygotowanie środowiska jest istotne ponieważ każdy członek zespołu powinien pracować na identycznych wersjach po to aby 
łatwo wychwycić różnicę pomiędzy zależnościami, które mogą wprowadzić regresję trudną do wychwycenia jeśli zależności nie są wersjonowane.

Rozpocząłem od standardowego setupu z istniejących projetków i dodałem zależności w conanie do pobrania Catch2. 

Ze zwględu jednak na problem z uruchamianiem dodatkowych kroków budowania jeszcze przed konfiguracją CMake zarzuciłem ten
krok i doinstalowałem z repozytorium ubuntu, choć normalnie każda biblioteka powinna pochodzić z conan albo vcpackage dla
lepszej kontroli 3rd party.

Ze zwględu na indywidualną pracę i ograniczony czas nie tworzyłem dodatkowych branchy a puszowałem bezpośrednio do głównego brancha.

Normalnie tworzę dedykowane feature branche i w MR oceniamy w zespole jakość kodu.

Ja korzystam z pełnego pakietu od JetBrains, a załadowanie projektu CMake do edytora CLion + pluginy do SQLlite, Docker przyspieszają pracę i
potwierdzają, że konfiguracja jest wczytywalna przez edytor co potwierdza poprawność zastosowanych rozwiązań.

Potem inny deweloper może skorzystać z innego IDE i podpiąć zarówno projekt jak i obrazy pod swoje preferencje.

To co warto podkreślić, fakt użycia dedykowanego obrazu pod to zadanie nie zaburza zainstalowanych na hoscie innych wersji qt, kompilatorów, narzędzi.
To szczególnie istotne jeśli na konkretnej maszynie rozwija się oprogramowanie dla różnych produktów lub nawet tego samego produktu w różnych wersjach
zależnych od innych wersji 3rd party czy konfiguracji.

Inną kwestią jest budowa modułowa oparta o modern CMake. Ważne aby konkretne biblioteki były rozdzielone, a testy były 
dostarczane razem z nimi. Dzięki temu można łatwo przenosić moduły wewnątrz projektu oraz między projektami oraz delegować pracę
pomiędzy różnych inżynierów.

Chciałem aby na każdym etapie rozwoju tego zadania możliwe było wyciągnięcie czegoś dla osoby, która to analizowałaby w przyszłości.
Dotyczy to także w szczególności procesu realnego produktu.

Przykładowo:

Pierwszym krokiem jest posiadanie kontenera z zależnościami. I to ma wartość, bo każdy inżynier może już pracować 
na tym samym izolowanym środowisku deweloperskim lub releasowym. Raz zbudowany obraz wysłany do artifactory może być wykorzytywany
przez innych inżynierów albo przekazany do devopsa aby wpiąć to w proces CI/CD. 

Dlaczego to takie istotne? Choćby ze względu na testy, które będą uruchamiane w osobnym kontenerze dając gwarancję,
że nie zaburzą one hosta. Np. w przypadku testowania aplikacji z realną bazą danych, a nie tylko fakeami.

Faki i mocki pomagają testować i prowadzić funkcjonalność w konkretnych kierunku, ale niezbędne są testy integracyjne i możliwie 
wysoko poziomowe testy, które zasymulują realne warunki. I wtedy konteneryzacja jest bardzo istotna bo w różnych kontenerach 
można przetestować realne użycie aplikacji w **izolacji**. Np. kontenery mogą komunikować się pomiedzy sobą.

Sam serwer http może być oddzielnym mikroserwisem uruchomionym w kontenerze. Aplikacja może pracować w innym.

## 1. Podzial kodu

Kod podzieliłem na osobne katalogi ze względu na ich odpowiedzialności. 
Maszyna stanów jest niezwiązana z Qt, dlatego biblioteka nie powinna zawierać żadnych zależności do frameworka QT ani SQLlite.
Testy są dostarczone razem z biblioteką. Każdy katalog jest osobną biblioteką, która jest linkowana do innego modułu według
potrzeb.

Jesli mamy same interfejsy tworzę bilbioteki header only. 

Unikam jednego dużego pliku konfiguracyjnego. Każdy moduł powinien implementować i dostarczać tylko to co jest jemu niezbędne.

Kod rozwijałem małymi fragmentami aby łatwo kontrolować co zostało dodane i ewentualnie wycofać jeśli okaże się błędne.
Nawet sam stab pustej aplikacji QTWidgets, a potem QML uruchamiajacy się w konkretnym comicie ma wartość, bo potwierdza poprawność
kontenera z którego zależności korzysta. 

Został dodany dedykowany katalog tests, który zawiera target AllTests. Jest to target stworzony po to, aby CLion mógł łatwo uruchomić wszystkie testy razem z code-coveragem.
Dzięki temu edytor po uruchomieniu takiego targetu z odpowiednimi parametrami(co nie wyklucza uruchomienia tego poprzez Gitlaba lub innych skryptach) pokazuje elegancko gdzie kod 
jest pokryty. Pomaga to śledzić dead code nie pokryty testami. Oczywiście samo pokrycie testem nie gwarantuje, że kod na pewno działa. Ale potwierdza tylko wykonanie ścieżki.

Zdecydowałem się na Catch2 ze względu na jego prostotę i czytelność, ale korzystam także z bardzo dojrzałego GTesta i Gmocka. Można w projekcie korzystać z różnych frameweroków 
testowych do sprawdzania różnych modułów według uznania i standardów kodowania istniejących w danej firmie.

To co istontne to fakt, że targety są Cmakowe. Nie ma żadnych skryptów bashowych, etc. Każdy devops czy inny developer może owrapować sobie targety
lub stworzyć własne konfiguracje wedłgu potrzeb lokalnie. Dzięki zastosowaniu takiego podejścia każdy edytor może uruchomić dowolny target jednym kliknięciem. Kod musi być IDE friendly i 
agnostyczny wzgledem powłok i edytorów, które go uruchamiają. Akceptowalne są targety CMake i Pythonowe skrypty, ale skrypoty nie mogą one wymuszać potem 
na innym developerze czegoś w stylu ./build.sh ./tests.sh. Takie skrypty mógłby sobie każdy indywidualnie stworzyć bądz devops w konkretnym pipeline na gitlabie.

Operacje na bazie danych są oddzielną biblioteką. Rozpocząłem od prostej bazy z inicjalizacją i 3 kolumnami aby typy byly inne oraz zapisem do memory.
Na pierwszą iterację to wystarczyło aby potwierdzić, że baza się zainicjalizowała oraz tabela została utworzona. Proste operacje insert i update zostały dodane na początku.

Gui w QML rozpocząłem od hello world, aby potwierdzić, że zależności są zainstalowane. Projekt tworzy dedykowany target **package**, który paczkuje gotową aplikację. 
Jest to istotne, bo pozwala użytkowinkowi korzystać z gotowego paczkowanego produktu na hoście, a kod jest zbudowany w kontenerze i przygotowany do deploya na innej maszynie razem
z potrzebnymi bibliotekami qt. Dzięki temu mamy zalążek produktu działający już na innej maszynie.

**Jest to istotne bo taki setup projektu pozwala zbudować inny produkt oparty o QML i CMake i Docker**  

Bibioteka sieciowa znajduje się za prostym interfejsem. Na samym początku skorzystałem z fake odpowiedzi serwera aby zaimplementować prostego SyncWorkera.
Dla wszystkich nie zsynchronizowanych rekordów w bazie syncworker wyśle request do backendu na /transactions i zwróci ilość zsynchronizowanych.

Potem został dodany realny mechanizm korzystający z network frameworka Qt wysyłający na localhosta requesty przez kontroler.
Kontroler tworzy oddzielny wątek, który w uruchamia synchworkera, który cyklicznie wysyła zapytania w przypadku braku odpowiedzi.

## 2. Idempotencja synchronizacji

UUID tranzakcji jest nadawany lokalnie przez `Transaction::create()` i jest uzywany
jako naglowek:

```text
Idempotency-Key: <transaction UUID>
```

Retry tej samej transakcji uzywa tego samego UUID. Po odpowiedzi HTTP 200-300 rekord jest
oznaczany jako zsynchronizowany w SQLite. Przy zmianie statusu, np. z `Dispensing`
na `Completed`, rekord ponownie otrzymuje `synchronized = 0` i jest wysylany z tym
samym UUID. Backend musi traktowac ten klucz jako identyfikator idempotencji i nie
tworzyc drugiego rekordu.

Do lokalnej demonstracji uzywany jest `src/PythonFakeServer/backed_simulation.py`.
Pierwsze zadanie danego klucza zwraca `201`, a kolejne `200`. W testach worker jest
oddzielony od sieci przez `ITransactionTransport` i uzywa `FakeTransactionTransport`.

## 3. Zanik zasilania podczas wydawania

Transakcja jest zapisywana przed wywolaniem podajnika ze statusem `Dispensing`.
Jezeli proces zostanie zakonczony przed otrzymaniem wyniku podajnika, po restarcie
rekord pozostaje w SQLite jako `Dispensing` i `synchronized = 0`. `SyncWorker` moze
go wyslac do backendu, ale nie zmienia sam statusu wydawania na `Completed` ani
`Failed`, poniewaz nie ma dowodu, czy towar fizycznie zostal wydany.

Można to zasymulować poprzez manualne przeklikanie w aplikacji i wyłaczanie jej w odpowiednim momencie.
Celowo ustawiłem 5 sekund aby można było to ręcznie zasymulować i sprawdzić w bazie danych np.poprzez połącznie się pluginem
z Cliona bezpośrednio do pliku sqlite.

Dodałem też test tego przypadku.

Tutaj należy zastanowić się jak obsłużyć taki przypadek. TBD w innym komicie. 

## 4. Zakres odlozony

Powinnien być watchdog sprawdzający czy jest połaczenie z Backendem.

Należy przeprowadzić recenzję kodu z innymi osobami specjalizującymi się w konkretnych obszarach technicznych związanych z
softwarem, ale nie tylko. Np. informacja od osób z galerii handlowej kiedy mogą być zaniki zasilania, problemy z dostępej do
sieci internetowej także mogą pomóc projektować rozwiązania i antycypować potencjalne problemy ze sprzętem.

Sprzęt bhp wydawany z machyny vendingowej w galerii może pracować w tzw. normalnych warunkach. Może się jednak zdarzyć, że
sprzęt będzie wymagał odpowiedniej izolacji elektrostatycznej - a mam doświadczenia, że maszyna wbudowana przy której pracowałem,
w galerii handlowej nie została zamontowana z uwzględnienim jej wymagań i maszyna "kopała prądem". Warto się upewnić,
że wymagania techniczne w pomieszczeniu spełniają wszystkie wymagania i nie będą wpływać na losowe/negatywne działanie oprogramowania.

Urządzenie ze względu na charakter wydawanego produktu może być ustawione w kopalniach, hutach, gdzie mogą panować niższe/wyższe temperatury,
albo np. zaburzenia elektromagnetyczne.

Powinno się przeprowadzić testy aplikacji z różnymi użytkownikami oraz testerami, którzy mogą szybko dostarczyć
przypadki testowe na podstawie doświadczenia. Warto pytać osoby z branży jakie najczęściej występują problemy
i czy one są przewidziane w obecnym oprogramowaniu.

Zarówno pozytywne scenariusze jak i oczekiwane błędy powinny mieć testy na różnym poziomie, także
możliwie wysokiego na poziomie aplikacji oraz smoke testy. 

## 5. Czas pracy

Dokladnie czasu nie mierzyłem. Myślę, że poświęciłem około 20h, ale traktowałem zadanie jako możliwość wypracowania setupów,
które moga mi się po prostu przydać przy rozwoju dowolnego produktu niezależnie od wyniku procesu rekrutacyjnego.
Sporo nowych elementów odkryłem w etydorze i CMakeu oraz budowaniu obrazu pod QML, których wcześniej nie znałem.
Prace obejmowaly przygotowanie srodowiska Qt/SQLite, maszynę stanow, testy, trwale transakcje, UUID, synchronizacje, integracje QML
oraz lokalny backend HTTP. 

## Najwazniejsze etapy w historii commitow

- przygotowanie Docker/Qt/SQLite i minimalnego UI,
- implementacja interfejsow sprzetowych oraz testow maszyny stanow,
- dodanie repozytorium SQLite i testow trwalosci,
- rozszerzenie transakcji do pieciu wymaganych pol i dodanie UUID,
- implementacja `SyncWorker`, fake transportu, JSON i exponential backoff,
- polaczenie QML z maszyna stanow i baza,
- dodanie timeoutu wyboru produktu,
- dodanie lokalnego backendu Python, HTTP transportu i synchronizacji w osobnym
  watku,

