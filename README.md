# IMU Nano 33 BLE Sense Rev2 - POC Brut + Fusion

Ce projet contient:

- Un firmware Arduino qui lit l'IMU et calcule une fusion capteurs simple (filtre complementaire).
- Une app Python qui affiche en temps reel:
  - donnees brutes accelerometre / gyroscope,
  - angles fusionnes (pitch, roll, yaw).

## 1) Firmware

Fichier: `firmware/imu_poc.ino`

### Bibliotheques Arduino

Dans l'IDE Arduino, installer:

- `Arduino_BMI270_BMM150`

### Flash

1. Ouvrir `firmware/imu_poc.ino`
2. Selectionner la carte `Arduino Nano 33 BLE Sense`
3. Televerser
4. Ouvrir le moniteur serie a `115200` pour verifier les trames CSV

Exemple trame:

`ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,pitch_deg,roll_deg,yaw_deg`

## 2) App affichage Python

Fichiers:

- `app/viewer.py`
- `app/requirements.txt`

### Installation

```powershell
cd app
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt
```

### Lancement

```powershell
python viewer.py --port COM5 --baud 115200
```

Remplace `COM5` par ton port reel.

## 3) Notes

- `pitch`/`roll` sont stabilises avec accelerometre + gyroscope.
- `yaw` est une integration gyro simple (derive dans le temps, normal pour ce POC).
- Pour ameliorer: calibration au demarrage, Mahony/Madgwick, correction magnetometre.
