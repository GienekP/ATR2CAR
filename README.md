# ATR2CAR
Convert ATARI ATR files to CAR (SWITCHABLE XEGS CARTRIDGE)

Konwerter uruchamiamy z wiersza poleceń:

atr2car File.atr File.car [-c] [-128|-256|-512]

Przykładowa konwersja:

atr2car Game.atr Game.car

Gdy wyskoczy nam informacja o potencjalnie niebezpiecznych skokach do procedur odczytu sektora np:

Possible calls:
 JSR JDSKINT ; 0x0000C6 20 53 E4 -> 20 00 01
 JSR JDSKINT ; 0x000412 20 53 E4 -> 20 00 01
 JSR JDSKINT ; 0x003A0D 20 53 E4 -> 20 00 01
 JSR JDSKINT ; 0x003DA3 20 53 E4 -> 20 00 01
 JSR JDSKINT ; 0x0080B5 20 53 E4 -> 20 00 01

to zalecane jest urcuhomienie z parametrem "-c"

atr2car Game.atr Game.car -c

Zostaną automatycznie podmienione wszystkie skoki. Bez przełącznika "-c" należy ręcznie samemu podmienić wymagane skoki.

Gdy chcemy bezwględnie wymusić wielkość pamięci flash np. 512kB należy użyć przełącznika "-512" np.:

atr2car Game.atr Game.car -512


Program buduje obraz dla "SWITCHABLE XEGS CARTRIDGE". Ma to trzy zalety. Pierwsze, to jest to standard dość dobrze opisany i wiadomo o co chodzi. Po drugie, dostępne są w sprzedaży elementy do tego. Po trzecie, ten standard dość dobrze nadaje się do emulowania dyskietki.

Podstawowy cart S-XEGS ma 128kB. Bank ostatni siedzi zawsze na $A000-$BFFF i dla niego napisano starter w wersji 128 bajtów na sektor i 256. Rejestr $D500 przełącza bank na adresie $8000-$9FFF. I tam jest podkładany obraz ATR.

Konwerter działa dla dyskietek SD i DD. Ich pojemność może być dowolna. Wielkość pamięci flash cartridgea 128kB, 256kB, 512kB zostanie dobrana automatycznie. Czasami dyskietka ma 130kB ale ostatnie sektory są nie wykorzystane, można wtedy wymusić wielkość pamięci flash parametrem "-128".
