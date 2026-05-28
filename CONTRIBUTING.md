# Contributing

Merci pour ta contribution.

## Workflow de branches

- `main`: branche stable, toujours fonctionnelle.
- `feat/<sujet-court>`: nouvelle fonctionnalite.
- `fix/<sujet-court>`: correction de bug.
- `chore/<sujet-court>`: maintenance (docs, refacto legere, tooling).

Exemples:

- `feat/gyro-calibration`
- `fix/serial-reconnect`
- `chore/readme-update`

## Commits

Messages de commit courts, explicites, en anglais:

- `feat: add startup gyro offset calibration`
- `fix: handle serial reconnect on timeout`
- `docs: clarify firmware flashing steps`

## Pull requests

Avant d'ouvrir une PR:

1. Verifier que le firmware compile dans l'IDE Arduino.
2. Verifier que l'app Python se lance (`python viewer.py --port COMx --baud 115200`).
3. Mettre a jour la documentation si le comportement change.

Template recommande pour une PR:

- **What**: ce qui change.
- **Why**: raison du changement.
- **How tested**: comment c'est teste.

## Definition of done

Un changement est considere termine si:

- il est fonctionnel localement,
- il n'introduit pas de regression evidente,
- il est documente si necessaire.
