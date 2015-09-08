Files
---------------------

smafestore.dump.sql - Struktur
smafestore.dump.roles.sql - Gruppen/User
smafestore.bootstrap.data.sql - Bootstrap Daten

Datenbank sichern
---------------------

1. Struktur sichern
pg_dump -f smafestore.dump.sql -o -s -U postgres smafestore

2. User sichern (evtl. user postgres löschen, damit am zielsystem dieser nicht verändert wird.)
pg_dumpall -r -f smafestore.dump.roles.sql -U postgres -W

Datenbank anlegen 
---------------------

1. bestehende Datenbank löschen (ignorieren wenn nicht gewünscht)
dropdb -U postgres -W smafestore

2. User einspielen 
psql -W postgres postgres < smafestore.dump.roles.sql

3. Datenbank anlegen (name kann angepasst werden)
createdb -W --username=smafeadmin smafestore

4. Struktur einspielen, Permissions vergeben
psql -W smafestore postgres < smafestore.dump.sql

5. Bootstrap Daten einspielen
psql -W smafestore postgres < smafestore.bootstrap.data.sql


