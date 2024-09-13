# Synergy

> [!TIP]
> We're on Libera Chat IRC network (`#synergy` or `#synergy-dev`).
> 
> Alternatively, there's [Discord](https://discord.com/invite/xBFv6j7) or [Slack](https://synergy-app.slack.com/join/shared_invite/zt-d8if26fr-6x~TSTz4skGmTnFP5IPaww#/shared-invite/email) (`#open-source` channel).
>
> Or if you want to stay on GitHub, you can [start a new discussion](https://github.com/symless/synergy/discussions).

> [!IMPORTANT]
> 🚀 Big changes are happening in the Synergy development community!
> 
> Been away for a while? Curious about what's changed?
> [Join the discussion](https://github.com/symless/synergy/discussions/7476)

[![GitHub Release](https://img.shields.io/github/v/release/symless/synergy?display_name=release&label=latest%20version)](https://github.com/symless/synergy/releases)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/symless/synergy)](https://github.com/symless/synergy/commits/master/)
[![GitHub License](https://img.shields.io/github/license/symless/synergy)](LICENSE)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=symless_synergy-core&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=symless_synergy-core)

[![CI](https://github.com/symless/synergy/actions/workflows/ci.yml/badge.svg)](https://github.com/symless/synergy/actions/workflows/ci.yml)
[![CodeQL Analysis](https://github.com/symless/synergy/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/symless/synergy/actions/workflows/codeql-analysis.yml)
[![SonarCloud Analysis](https://github.com/symless/synergy/actions/workflows/sonarcloud-analysis.yml/badge.svg)](https://github.com/symless/synergy/actions/workflows/sonarcloud-analysis.yml)
[![Build containers](https://github.com/symless/synergy/actions/workflows/build-containers.yml/badge.svg)](https://github.com/symless/synergy/actions/workflows/build-containers.yml)

[Synergy](https://symless.com/synergy) is a keyboard and mouse sharing app. 
Use the keyboard, mouse, or trackpad of one computer to control nearby computers, 
and work seamlessly between them.

This project contains the source code for _Synergy 1 Community Edition_ which is actively 
maintained.
Synergy 1 Community Edition is free and open source software, and anyone is welcome to build it,
run it, tinker with it, redistribute it as part of their own app, etc.

**Wayland support:** Experimental support in 
[Synergy v1.16](https://github.com/symless/synergy/releases/tag/1.16.0-beta%2Br2)
(required >= GNOME 46 or KDE Plasma 6.1).

![Synergy 1 Community Edition](https://github.com/user-attachments/assets/faf5bd69-336c-4bd0-ace3-e911f199d961)

To use the community edition, we encourage you to build it yourself using the [Build Quick Start](#build-quick-start)
instructions below or you can install the `synergy` package with your [favorite package manager](#how-to-install-packages)
(please note that some package managers have older versions and need to be updated).

## Roadmap

| Version | Main goal | Date | Status |
| --- | --- | --- | --- |
| [v1.16](https://github.com/symless/synergy/releases/tag/1.16.0-beta%2Br2) | Experimental Wayland support (`libei` and `libportal`) | 9th Sep 2024 | Beta released |
| v1.17 | Cross-platform daemon to replace legacy daemon | 18th Oct 2024 | Planning |

Dates are likely to change. If you have any questions, 
please [start a discussion](https://github.com/symless/synergy/discussions).

## Goals and Philosophy

Version 1.15 brings a [new philosophy](https://github.com/symless/synergy/discussions/7476)
of being more approachable to the open source community instead of wholly focusing on commercial interests.
We still have customers to finance the development of the code, but we are committed to
maintaining and improving Synergy 1 Community Edition for years to come and we're excited to work
with our community of open source users to improve the code for the benefit of everyone.

## Ways to get involved

Here are a few ways to join in with the project and get involved:
* Build the latest `master` version (see below) and [report a bug](https://github.com/symless/synergy/issues)
* [Submit a PR](https://github.com/symless/synergy/wiki/Contributing) (pull request) with a bug fix or improvement
* [Let us know](https://github.com/symless/synergy/issues) if you have an idea for an improvement

## Build Quick Start

Synergy 1 Community Edition is free and open source software, and anyone is welcome to build it,
run it, tinker with it, redistribute it as part of their own app, etc.

> [!TIP]
> These instructions will build Synergy 1 Community Edition, which doesn't require a license
> or serial key. Check the [Build Guide](https://github.com/symless/synergy/wiki/Build-Guide)
> wiki page if you have problems.

**1. Dependencies:**

*Linux, macOS, or BSD-derived:*
```
./scripts/install_deps.sh
```

*Windows:*
```
python scripts/install_deps.py
```

**2. Configure:**

*Linux, macOS, or BSD-derived:*
```
cmake -B build
```

*Windows:*
```
cmake -B build --preset=windows-release
```

**3. Build:**
```
cmake --build build -j8
```

**4. Test:**
```
./build/bin/unittests
./build/bin/integtests
```

**5. Run**
```
./build/bin/synergy
```

## How to install (packages)

Synergy 1 Community Edition is packaged by the community (status shown below).

Synergy 2 is no longer in development and we recommend that package maintainers do not provide it.

Synergy 3 and Synergy 1 (licensed) are available to download from the 
[official packages](https://symless.com/synergy/download).

**Community edition:**

*macOS:*
```
brew install synergy
```

*Fedora, Red Hat, etc:*
```
sudo dnf install synergy
```

*Debian, Ubuntu, etc:*
*(temporarily broken)*
```
sudo apt install synergy
```

*Arch, Manjaro, etc:*
*(temporarily broken)*
```
sudo pacman -S synergy
```

*Windows:*
*(very outdated)*
```
choco install synergy
```

**Note:** Some packages are temporarily broken. We have reached out to the package maintainers to 
resolve this. If you're a package maintainer and have a question, please let us know.

## Operating Systems

We support all major operating systems, including Windows, macOS, Linux, and Unix-like BSD-derived.

All Linux distributions are supported, primarily focusing on: 
Debian, Ubuntu, Linux Mint, Fedora, RHEL, AlmaLinux, Rocky Linux, Arch Linux, openSUSE, Gentoo.

We officially support FreeBSD, and would also like to support: OpenBSD, NetBSD, DragonFly, Solaris.

## Package Maintainers

Synergy is already available through most package managers as `synergy`, and we would love to see
the latest version of Synergy 1 Community Edition on every package manager for every OS we support
(Windows, macOS, Linux, Unix-like BSD-derived, etc).

Package maintainers can use `scripts/package.py` to see how we build packages.
Feel free to contact us if you have any questions about packaging Synergy.

Current packages:
- [Fedora](https://packages.fedoraproject.org/pkgs/synergy/synergy/)
- [Arch Linux](https://aur.archlinux.org/packages/synergy)
- [FreeBSD](https://www.freshports.org/sysutils/synergy)
- [Homebrew](https://formulae.brew.sh/formula/synergy-core#default)
- [Gentoo](https://packages.gentoo.org/packages/x11-misc/synergy)

Very outdated packages:
- [Chocolatey](https://community.chocolatey.org/packages/synergy)
- [openSUSE](https://build.opensuse.org/package/show/openSUSE:Factory/synergy)

Broken packages:
- [Ubuntu](https://launchpad.net/ubuntu/+source/synergy)
- [Debian](https://tracker.debian.org/pkg/synergy)

It appears that the `synergy` package has been removed or discontinued from some package repositories.
That's a bit sad. If you're a package maintainer, please let us know if you have any questions.

## Similar Projects

* [Lan Mouse](https://github.com/feschber/lan-mouse) - A Rust implementation with the goal of
  having native front-ends and interoperability with Synergy.
* [Input Leap](https://github.com/input-leap/input-leap) - A Synergy-derivative with the goal
  of continuing what Barrier started (now a dead Synergy fork).

## FAQ

### Is Synergy compatible with Lan Mouse or Input Leap?

We would love to see compatibility between our fellow open source projects, Lan Mouse and 
Input Leap. This idea is discussed from time in the communities for all of our projects,
so it may happen in the not-too-distant future.

### Has Synergy moved beyond its goals from the 1.x era?

Our goal for Synergy 1 (including the community edition) has always been and will always be to 
make a simple, reliable, and feature-rich mouse and keyboard-sharing tool. We do maintain another
product called Synergy 3, but as this uses Synergy 1 Core (the server and client part of Synergy),
we depend on Synergy 1 to remain stable and modern which is why we continue to develop and improve
the product.

### If I want to solve issues in Synergy do I need to contribute to a fork?

We welcome PRs (pull requests) from the community. If you'd like to make a change, please feel
free to [start a discussion](https://github.com/symless/synergy/discussions) or 
[open a PR](https://github.com/symless/synergy/wiki/Contributing).
It's great that people spin up re-branded forks of Synergy, power to them. However, it's not 
necessary if you want to make changes.
If you're thinking of starting your own re-branded fork of Synergy, it might be because we're
doing something wrong so please 
[let us know](https://github.com/symless/synergy/wiki/Contact-the-team) what we can do to let
you feel welcome in our community.

### Is clipboard sharing supported?

Absolutely. The clipboard-sharing feature is a cornerstone feature of the product and we are 
committed to maintaining and improving that feature.

### Is Wayland for Linux supported?

Yes! Wayland (the Linux display server protocol aimed to become the successor of the X Window 
System) is an important platform for us.
The [`libei`](https://gitlab.freedesktop.org/libinput/libei) and 
[`libportal`](https://github.com/flatpak/libportal) libraries enable 
Wayland support for Synergy. We would like to give special thanks to Peter Hutterer (@whot),
who is the author of `libei`, a major contributor to `libportal`, and the author of the Wayland
implementation in Synergy. Others such as Olivier Fourdan helped with the Wayland implementation,
and we rely on the work of our community of developers to continue the development of Wayland 
support.

### Where did it all start?

Synergy was first created in 2001 by Chris Schoeneman.
Read about the [history of the project](https://github.com/symless/synergy/wiki/History) on our
wiki.

## Repology

Repology monitors a huge number of package repositories and other sources comparing package
versions across them and gathering other information.

[![Repology](https://repology.org/badge/vertical-allrepos/synergy.svg?exclude_unsupported=1)](https://repology.org/project/synergy/versions)

## License

This project is licensed under [GPL-2.0](LICENSE) with an [OpenSSL exception](LICENSE_EXCEPTION).
