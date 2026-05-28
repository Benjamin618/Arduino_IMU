# Versioning and Releases

Ce projet utilise **Semantic Versioning**: `MAJOR.MINOR.PATCH`.

- `MAJOR`: changement incompatible.
- `MINOR`: ajout compatible (nouvelle feature).
- `PATCH`: correction compatible (bugfix).

## Regles pratiques

- Tant que le projet est en phase de POC: commencer a `0.1.0`.
- Evolution type POC:
  - `0.1.0` premiere version partageable
  - `0.2.0` nouvelles features
  - `0.2.1` correctifs rapides

## Quand creer un tag

Creer un tag quand une version est:

- testee localement (firmware + viewer),
- suffisamment stable pour etre reutilisee.

## Commandes Git utiles

Creer et pousser un tag:

```powershell
git tag -a v0.1.0 -m "First stable IMU POC release"
git push origin v0.1.0
```

Lister les tags:

```powershell
git tag -l
```

## Notes de release (format court)

Pour chaque version, noter:

- points importants (feature/fix),
- impacts utilisateur,
- instructions de migration s'il y en a.
