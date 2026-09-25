# Arch / yay packaging

`PKGBUILD-git` is the development package intended for testers. Once it is published to the AUR, testers can use:

```sh
yay -S sony-headphones-linux-git
```

The `PKGBUILD` package tracks signed version tags (`v0.1.0`, etc.) and is the stable release channel:

```sh
yay -S sony-headphones-linux
```

## Publishing

The AUR uses a separate Git repository and SSH key. Do not put an AUR private key in this project. After an AUR account has an SSH key registered, create the two AUR repositories and push the corresponding packaging files plus generated `.SRCINFO` metadata there. GitHub Actions can later automate that push with an `AUR_SSH_PRIVATE_KEY` repository secret.
