# GitHub veröffentlichen

## Einfach über GitHub Desktop

1. Dieses ZIP **vollständig entpacken** (keine ZIP-Datei selbst hochladen).
2. In GitHub ein neues **öffentliches**, leeres Repository `Keller-Sensorzentrale` anlegen. Kein zusätzliches README, .gitignore oder LICENSE erzeugen, da alles vorhanden ist.
3. In GitHub Desktop: **File → Add local repository** → entpackten Projektordner wählen. Falls nötig **Create a repository here**.
4. Änderungen committen, dann über **Publish repository** auf GitHub veröffentlichen. Dort die Sichtbarkeit **Public** bestätigen.
5. Die Webansicht öffnen und überprüfen, dass die `firmware/`, `home-assistant/`, `docs/` und `enclosure/`-Ordner samt README und LICENSE sichtbar sind.

## Alternativ per Git

Im entpackten Ordner in PowerShell oder Terminal:

```bash
git init
git add .
git commit -m "Initial public prototype v0.1.1"
git branch -M main
git remote add origin https://github.com/DEIN-BENUTZERNAME/Keller-Sensorzentrale.git
git push -u origin main
```

Vorher das leere Repository bei GitHub angelegt haben. `DEIN-BENUTZERNAME` ersetzen. Falls bereits `.git` existiert, `git init` überspringen. **Nicht** direkt Dateien aus dem eigenen Arduino-Sketch verwenden, in dem reale Zugangsdaten eingetragen sind; nur diese bereinigte Repositoryversion publizieren.

## Vor dem Klick auf Publish

- [ ] Eigene Bild- und Quellcode-Rechte prüfen
- [ ] Alle Lizenzbedingungen (auch möglicher Dritter) prüfen
- [ ] Sanitisierten Sketch kompilieren; drei `#BITTE_AENDERN`-Werte nur in der lokalen Kopie ersetzen
- [ ] Keine echte `secrets.yaml`, MQTT-Daten, private IP-Adresse oder WLAN-Daten enthalten
- [ ] Schaltbild und Pins mit realer Platinenrevision vergleichen
- [ ] Keine Netzspannungs-Verdrahtung oder unzulässige Sensorinstallation zeigen

Das MakerWorld-Gehäuse bleibt getrennt. Dessen STL/3MF-Freigabe erfolgt erst später mit eigener Lizenzentscheidung.
