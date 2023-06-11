# ASB_arduino
Kod wygrywany do ESP32. 
Po podłączeniu się do WiFi pobiera z serwisu informacje na temat temperatury załączania wentylatora (REST), informację o tym czy wentylator jest włączony oraz czy wentylator ma być włączany automatycznie.
Pozostała komunikacja odbywa się poprzez MQTT.
Program odczytuje co 5 sekund dane dotyczące wilgotności oraz temperatury (wykorzystując DHT11 lub DHT22) oraz wysyła je poprzez MQTT.
Do programu można odezwać się po MQTT w celu włączenia/wyłączenia wentylatora, włączenia/wyłączenia automatycznej wentylacji, zmiany temperatury załączania automatycznej wentylacji oraz w celu uruchomienia procesu dodawania nowej karty RFID.
Program umożliwia ręczne ustawienie temepratury załączania automatycznej wentylacji poprzez odczyt autoryzowanej karty przyłożonej do RFID-RC522 oraz oporu na potencjometrze.
